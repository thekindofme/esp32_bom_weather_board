#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <string>

#include "BomParser.h"
#include "Config.h"
#include "FtpUtils.h"

struct WeatherData {
  String stationName;
  String observedTimeLocal;
  String windDir;
  String airTempC;
  String apparentTempC;
  String relHumidityPct;
  String windKmh;
  String rainfallMm;
  bool valid = false;
};

TFT_eSPI tft = TFT_eSPI();

uint32_t lastRefreshMs = 0;
String lastError;
WeatherData latestData;

static bool waitForFtpCode(WiFiClient &control, const char *expectedPrefix, uint32_t timeoutMs, String &response) {
  uint32_t start = millis();
  response = "";
  while (millis() - start < timeoutMs) {
    while (control.available()) {
      String line = control.readStringUntil('\n');
      line.trim();
      if (!line.isEmpty()) {
        response = line;
      }
    }
    if (!response.isEmpty() && response.startsWith(expectedPrefix)) {
      return true;
    }
    delay(10);
  }
  return false;
}

static bool sendFtpCommand(WiFiClient &control, const String &cmd, const char *expectCode, String &resp) {
  control.print(cmd);
  control.print("\r\n");
  return waitForFtpCode(control, expectCode, FTP_TIMEOUT_MS, resp);
}

static bool fetchFromBomFtp(WeatherData &outData, String &error) {
  WiFiClient control;
  if (!control.connect(BOM_HOST, 21)) {
    error = "FTP control connect failed";
    return false;
  }
  control.setTimeout(FTP_TIMEOUT_MS / 1000);

  String resp;
  if (!waitForFtpCode(control, "220", FTP_TIMEOUT_MS, resp)) {
    error = "No FTP welcome";
    control.stop();
    return false;
  }
  if (!sendFtpCommand(control, "USER anonymous", "331", resp)) {
    error = "USER failed: " + resp;
    control.stop();
    return false;
  }
  if (!sendFtpCommand(control, "PASS esp32@example.com", "230", resp)) {
    error = "PASS failed: " + resp;
    control.stop();
    return false;
  }
  if (!sendFtpCommand(control, "TYPE I", "200", resp)) {
    error = "TYPE failed: " + resp;
    control.stop();
    return false;
  }
  if (!sendFtpCommand(control, "PASV", "227", resp)) {
    error = "PASV failed: " + resp;
    control.stop();
    return false;
  }

  std::string dataIpStd;
  uint16_t dataPort = 0;
  if (!ParsePasvResponse(std::string(resp.c_str()), dataIpStd, dataPort)) {
    error = "PASV parse failed";
    control.stop();
    return false;
  }
  String dataIp = String(dataIpStd.c_str());

  WiFiClient data;
  if (!data.connect(dataIp.c_str(), dataPort)) {
    error = "FTP data connect failed";
    control.stop();
    return false;
  }
  data.setTimeout(FTP_TIMEOUT_MS / 1000);

  if (!sendFtpCommand(control, String("RETR ") + BOM_FILE_PATH, "150", resp)) {
    error = "RETR failed: " + resp;
    data.stop();
    control.stop();
    return false;
  }

  BomStationParser parser(BOM_STATION_ID);

  outData = {};

  while (data.connected() || data.available()) {
    String line = data.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) {
      delay(1);
      continue;
    }

    parser.FeedLine(std::string(line.c_str()));
    if (parser.IsDone()) {
      break;
    }
  }

  data.stop();
  waitForFtpCode(control, "226", FTP_TIMEOUT_MS, resp);
  sendFtpCommand(control, "QUIT", "221", resp);
  control.stop();

  std::string parserError;
  if (!parser.Finalize(parserError)) {
    error = String(parserError.c_str());
    return false;
  }

  const ParsedWeatherData &parsed = parser.Data();
  outData.stationName = String(parsed.stationName.c_str());
  outData.observedTimeLocal = String(parsed.observedTimeLocal.c_str());
  outData.windDir = String(parsed.windDir.c_str());
  outData.airTempC = String(parsed.airTempC.c_str());
  outData.apparentTempC = String(parsed.apparentTempC.c_str());
  outData.relHumidityPct = String(parsed.relHumidityPct.c_str());
  outData.windKmh = String(parsed.windKmh.c_str());
  outData.rainfallMm = String(parsed.rainfallMm.c_str());
  outData.valid = true;
  return true;
}

static bool ensureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t start = millis();
  while (millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(250);
  }
  return false;
}

static void drawHeader(const char *title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, 8, 6, 2);
  tft.drawFastHLine(0, 26, tft.width(), TFT_DARKCYAN);
}

static void drawWeather(const WeatherData &w) {
  drawHeader("ESP32 BOM Weather");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(w.stationName, 8, 32, 2);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Temp", 8, 60, 2);
  tft.drawString(w.airTempC + " C", 150, 60, 4);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Feels", 8, 102, 2);
  tft.drawString(w.apparentTempC + " C", 150, 102, 2);

  tft.drawString("Humidity", 8, 128, 2);
  tft.drawString(w.relHumidityPct + " %", 150, 128, 2);

  tft.drawString("Wind", 8, 154, 2);
  tft.drawString(w.windDir + " " + w.windKmh + " km/h", 150, 154, 2);

  tft.drawString("Rain", 8, 180, 2);
  tft.drawString(w.rainfallMm + " mm", 150, 180, 2);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Obs: " + w.observedTimeLocal, 8, 210, 2);
  tft.drawString("IP: " + WiFi.localIP().toString(), 8, 236, 2);
}

static void drawStatus(const String &status, const String &detail) {
  drawHeader("ESP32 BOM Weather");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(detail, 8, 84, 2);
}

static void refreshWeatherNow() {
  if (!ensureWifiConnected()) {
    lastError = "Wi-Fi connect timeout";
    if (latestData.valid) {
      drawWeather(latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 290, 2);
    } else {
      drawStatus("Wi-Fi failed", lastError);
    }
    return;
  }

  drawStatus("Updating...", "Fetching BOM FTP XML");

  WeatherData fresh;
  String err;
  if (fetchFromBomFtp(fresh, err)) {
    latestData = fresh;
    lastError = "";
    drawWeather(latestData);
  } else {
    lastError = err;
    if (latestData.valid) {
      drawWeather(latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 290, 2);
    } else {
      drawStatus("BOM fetch failed", lastError);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(1); // 320x240 landscape

#if defined(TFT_BL) && (TFT_BL >= 0)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

  drawStatus("Booting", "Starting Wi-Fi and display");
  refreshWeatherNow();
  lastRefreshMs = millis();
}

void loop() {
  const uint32_t now = millis();
  if (now - lastRefreshMs >= WEATHER_REFRESH_MS) {
    refreshWeatherNow();
    lastRefreshMs = now;
  }
  delay(100);
}
