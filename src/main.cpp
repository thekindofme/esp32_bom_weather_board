#include <Arduino.h>
#include <time.h>
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
  String dayMinTempC;
  String dayMaxTempC;
  String feelsMinTempC;
  String feelsMaxTempC;
  String rainTodayRange;
  String rainMorningRange;
  String rainEveningRange;
  bool valid = false;
};

TFT_eSPI tft = TFT_eSPI();

uint32_t lastRefreshMs = 0;
uint32_t lastClockUpdateMs = 0;
String lastError;
WeatherData latestData;
bool ntpConfigured = false;

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

static float parseFirstFloat(const String &s, bool &ok) {
  String t = s;
  t.trim();
  int i = 0;
  while (i < t.length() && !(isdigit(t[i]) || t[i] == '-' || t[i] == '+')) i++;
  int j = i;
  while (j < t.length() && (isdigit(t[j]) || t[j] == '.' || t[j] == '-' || t[j] == '+')) j++;
  if (i >= j) {
    ok = false;
    return 0.0f;
  }
  ok = true;
  return t.substring(i, j).toFloat();
}

static bool parseRangeToFloats(const String &range, float &lo, float &hi) {
  int toPos = range.indexOf(" to ");
  if (toPos < 0) return false;
  String a = range.substring(0, toPos);
  String b = range.substring(toPos + 4);
  bool oka = false, okb = false;
  lo = parseFirstFloat(a, oka);
  hi = parseFirstFloat(b, okb);
  return oka && okb;
}

static String formatRange(float lo, float hi) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%.0f-%.0f", lo, hi);
  return String(buf);
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
  outData.dayMinTempC = String(parsed.dayMinTempC.c_str());
  outData.dayMaxTempC = String(parsed.dayMaxTempC.c_str());
  outData.valid = true;
  return true;
}

static bool fetchForecastRainFromBomFtp(WeatherData &outData, String &error) {
  WiFiClient control;
  if (!control.connect(BOM_HOST, 21)) {
    error = "Forecast FTP connect failed";
    return false;
  }
  control.setTimeout(FTP_TIMEOUT_MS / 1000);

  String resp;
  if (!waitForFtpCode(control, "220", FTP_TIMEOUT_MS, resp)) {
    error = "Forecast no FTP welcome";
    control.stop();
    return false;
  }
  if (!sendFtpCommand(control, "USER anonymous", "331", resp) ||
      !sendFtpCommand(control, "PASS esp32@example.com", "230", resp) ||
      !sendFtpCommand(control, "TYPE I", "200", resp) ||
      !sendFtpCommand(control, "PASV", "227", resp)) {
    error = "Forecast FTP login/PASV failed";
    control.stop();
    return false;
  }

  std::string dataIpStd;
  uint16_t dataPort = 0;
  if (!ParsePasvResponse(std::string(resp.c_str()), dataIpStd, dataPort)) {
    error = "Forecast PASV parse failed";
    control.stop();
    return false;
  }

  WiFiClient data;
  String dataIp = String(dataIpStd.c_str());
  if (!data.connect(dataIp.c_str(), dataPort)) {
    error = "Forecast FTP data connect failed";
    control.stop();
    return false;
  }

  if (!sendFtpCommand(control, String("RETR ") + BOM_FORECAST_FILE_PATH, "150", resp)) {
    error = "Forecast RETR failed";
    data.stop();
    control.stop();
    return false;
  }

  bool inTargetArea = false;
  String periodIndex = "";
  String dayRange = "";
  String fallbackProb0 = "";

  while (data.connected() || data.available()) {
    String line = data.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;

    if (!inTargetArea) {
      if (line.startsWith("<area ") &&
          line.indexOf(String("description=\"") + BOM_FORECAST_LOCATION + "\"") >= 0 &&
          line.indexOf("type=\"location\"") >= 0) {
        inTargetArea = true;
      }
      continue;
    }

    if (line.startsWith("</area")) break;

    if (line.startsWith("<forecast-period ")) {
      periodIndex = "";
      int p = line.indexOf("index=\"");
      if (p >= 0) {
        int s = p + 7;
        int e = line.indexOf('"', s);
        if (e > s) periodIndex = line.substring(s, e);
      }
      continue;
    }

    if (periodIndex == "1" && line.indexOf("type=\"precipitation_range\"") >= 0) {
      int gt = line.indexOf('>');
      int lt = line.indexOf('<', gt + 1);
      if (gt > 0 && lt > gt) dayRange = line.substring(gt + 1, lt);
      continue;
    }

    if (periodIndex == "0" && line.indexOf("type=\"probability_of_precipitation\"") >= 0) {
      int gt = line.indexOf('>');
      int lt = line.indexOf('<', gt + 1);
      if (gt > 0 && lt > gt) fallbackProb0 = line.substring(gt + 1, lt);
      continue;
    }
  }

  data.stop();
  waitForFtpCode(control, "226", FTP_TIMEOUT_MS, resp);
  sendFtpCommand(control, "QUIT", "221", resp);
  control.stop();

  dayRange.trim();
  if (dayRange.isEmpty()) {
    if (!fallbackProb0.isEmpty()) {
      outData.rainTodayRange = "~" + fallbackProb0 + " chance";
      outData.rainMorningRange = "--";
      outData.rainEveningRange = "~" + fallbackProb0;
      return true;
    }
    error = "No forecast rain range";
    return false;
  }

  outData.rainTodayRange = dayRange;
  float lo = 0.0f, hi = 0.0f;
  if (parseRangeToFloats(dayRange, lo, hi)) {
    outData.rainMorningRange = "~" + formatRange(lo / 2.0f, hi / 2.0f) + " mm";
    outData.rainEveningRange = "~" + formatRange(lo / 2.0f, hi / 2.0f) + " mm";
  } else {
    outData.rainMorningRange = "--";
    outData.rainEveningRange = "--";
  }
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

static bool ensureTimeConfigured() {
  if (ntpConfigured) return true;

  configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2);
  struct tm t;
  for (int i = 0; i < 20; ++i) {
    if (getLocalTime(&t, 300)) {
      ntpConfigured = true;
      return true;
    }
    delay(200);
  }
  return false;
}

static String getCurrentTimeString() {
  struct tm t;
  if (!getLocalTime(&t, 50)) {
    return "--:--:--";
  }
  char buf[16];
  strftime(buf, sizeof(buf), "%H:%M:%S", &t);
  return String(buf);
}

static String getCurrentDateString() {
  struct tm t;
  if (!getLocalTime(&t, 50)) {
    return "----/--/--";
  }
  char buf[20];
  strftime(buf, sizeof(buf), "%a %d %b %Y", &t);
  return String(buf);
}

static void drawNowAndDate() {
  // Reserve bottom strip for clock/date and refresh it frequently.
  tft.fillRect(0, 200, tft.width(), 40, TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Now: " + getCurrentTimeString(), 8, 202, 2);
  tft.drawString("Date: " + getCurrentDateString(), 8, 218, 2);
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
  tft.drawString("Temp", 8, 54, 2);
  tft.drawString(w.airTempC + " C", 150, 52, 4);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Feels", 8, 90, 2);
  tft.drawString(w.apparentTempC + " C", 150, 90, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("T range: " + w.dayMinTempC + "-" + w.dayMaxTempC + " C", 8, 112, 1);
  tft.drawString("F range~: " + w.feelsMinTempC + "-" + w.feelsMaxTempC + " C", 8, 124, 1);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Humidity", 8, 136, 2);
  tft.drawString(w.relHumidityPct + " %", 150, 136, 2);

  tft.drawString("Wind", 8, 152, 2);
  tft.drawString(w.windDir + " " + w.windKmh + " km/h", 150, 152, 2);

  tft.drawString("Rain now", 8, 168, 2);
  tft.drawString(w.rainfallMm + " mm", 150, 168, 2);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Rain day: " + w.rainTodayRange, 8, 184, 1);
  tft.drawString("AM~ " + w.rainMorningRange + "  PM~ " + w.rainEveningRange, 8, 194, 1);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Obs: " + w.observedTimeLocal, 8, 184, 1);
  drawNowAndDate();
}

static void drawStatus(const String &status, const String &detail) {
  drawHeader("ESP32 BOM Weather");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(detail, 8, 84, 2);
}

static void refreshWeatherNow() {
  Serial.println("[weather] refresh start");
  if (!ensureWifiConnected()) {
    lastError = "Wi-Fi connect timeout";
    Serial.println("[weather] wifi failed");
    if (latestData.valid) {
      drawWeather(latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 186, 2);
    } else {
      drawStatus("Wi-Fi failed", lastError);
    }
    return;
  }

  ensureTimeConfigured();

  drawStatus("Updating...", "Fetching BOM FTP XML");
  Serial.println("[weather] fetching BOM feed");

  WeatherData fresh;
  String err;
  if (fetchFromBomFtp(fresh, err)) {
    bool okForecast = fetchForecastRainFromBomFtp(fresh, err);
    if (!okForecast) {
      fresh.rainTodayRange = "n/a";
      fresh.rainMorningRange = "--";
      fresh.rainEveningRange = "--";
    }

    bool okAir = false, okFeels = false;
    float air = parseFirstFloat(fresh.airTempC, okAir);
    float feels = parseFirstFloat(fresh.apparentTempC, okFeels);
    if (okAir && okFeels && !fresh.dayMinTempC.isEmpty() && !fresh.dayMaxTempC.isEmpty()) {
      bool okMin = false, okMax = false;
      float minA = parseFirstFloat(fresh.dayMinTempC, okMin);
      float maxA = parseFirstFloat(fresh.dayMaxTempC, okMax);
      if (okMin && okMax) {
        float delta = feels - air;
        fresh.feelsMinTempC = String(minA + delta, 1);
        fresh.feelsMaxTempC = String(maxA + delta, 1);
      }
    }
    if (fresh.feelsMinTempC.isEmpty()) fresh.feelsMinTempC = "--";
    if (fresh.feelsMaxTempC.isEmpty()) fresh.feelsMaxTempC = "--";
    if (fresh.dayMinTempC.isEmpty()) fresh.dayMinTempC = "--";
    if (fresh.dayMaxTempC.isEmpty()) fresh.dayMaxTempC = "--";

    latestData = fresh;
    lastError = "";
    Serial.println("[weather] fetch success");
    drawWeather(latestData);
  } else {
    lastError = err;
    Serial.print("[weather] fetch failed: ");
    Serial.println(lastError);
    if (latestData.valid) {
      drawWeather(latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 186, 2);
    } else {
      drawStatus("BOM fetch failed", lastError);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("[boot] setup start");

  tft.init();
  tft.setRotation(1); // 320x240 landscape
  tft.invertDisplay(false);

#if defined(TFT_BL) && (TFT_BL >= 0)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

  drawStatus("Booting", "Starting Wi-Fi and display");
  Serial.println("[boot] display init done");
  refreshWeatherNow();
  lastRefreshMs = millis();
  Serial.println("[boot] setup complete");
}

void loop() {
  const uint32_t now = millis();
  if (latestData.valid && now - lastClockUpdateMs >= 1000UL) {
    drawNowAndDate();
    lastClockUpdateMs = now;
  }
  if (now - lastRefreshMs >= WEATHER_REFRESH_MS) {
    refreshWeatherNow();
    lastRefreshMs = now;
  }
  delay(100);
}
