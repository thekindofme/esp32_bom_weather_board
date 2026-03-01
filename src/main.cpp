#include <Arduino.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <string>

#include "BomParser.h"
#include "Config.h"
#include "ForecastParser.h"
#include "FtpUtils.h"
#include "WeatherMath.h"

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
  String rainTodayChance;
  String currentIconCode;
  String nextDayLabel[3];
  String nextDayIconCode[3];
  String nextDayMinC[3];
  String nextDayMaxC[3];
  String nextDayRain[3];
  String nextDayRainChance[3];
  bool valid = false;
};

TFT_eSPI tft = TFT_eSPI();

uint32_t lastRefreshMs = 0;
uint32_t lastClockUpdateMs = 0;
uint32_t lastBacklightAdjustMs = 0;
String lastError;
WeatherData latestData;
bool ntpConfigured = false;
bool refreshAnimating = false;
uint32_t lastSpinnerMs = 0;
uint8_t spinnerFrame = 0;
bool hasPwmBacklight = false;
uint16_t themeBg = 0;
uint16_t themeHeader = 0;
uint16_t themePanel = 0;
uint16_t themeEdge = 0;
uint16_t themeText = 0;
uint16_t themeTextMuted = 0;
uint16_t themeAccent = 0;
uint16_t themeGood = 0;

static void animateRefreshTick();
static void drawRefreshIndicator(bool active);
static void applyBacklightForTime();
static void drawNowAndDate();
static void drawWeather(const WeatherData &w);

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
    animateRefreshTick();
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

static String extractXmlValue(const String &line) {
  int gt = line.indexOf('>');
  int lt = line.indexOf('<', gt + 1);
  if (gt < 0 || lt < 0 || lt <= gt + 1) return "";
  return line.substring(gt + 1, lt);
}

static String extractAttributeFromLine(const String &line, const char *attr) {
  String key = String(attr) + "=\"";
  int p = line.indexOf(key);
  if (p < 0) return "";
  int s = p + key.length();
  int e = line.indexOf('"', s);
  if (e <= s) return "";
  return line.substring(s, e);
}

static String dayLabelFromIso(const String &iso) {
  if (iso.length() < 10) return "--";
  int y = iso.substring(0, 4).toInt();
  int m = iso.substring(5, 7).toInt();
  int d = iso.substring(8, 10).toInt();
  struct tm t = {};
  t.tm_year = y - 1900;
  t.tm_mon = m - 1;
  t.tm_mday = d;
  t.tm_isdst = -1;
  if (mktime(&t) < 0) return String("D") + String(d);
  const char *wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return String(wd[t.tm_wday]);
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
      animateRefreshTick();
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

  ForecastLocationParser parser(BOM_FORECAST_LOCATION);

  while (data.connected() || data.available()) {
    String line = data.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) {
      animateRefreshTick();
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

  std::string pErr;
  if (!parser.Finalize(pErr)) {
    error = String(pErr.c_str());
    return false;
  }

  const ForecastParseResult& r = parser.Data();
  outData.rainTodayRange = String(r.rainTodayRange.c_str());
  outData.rainTodayChance = String(r.rainTodayChance.c_str());
  outData.currentIconCode = String(r.currentIconCode.c_str());

  std::string am;
  std::string pm;
  if (EstimateHalfDayRainRange(r.rainTodayRange, am, pm)) {
    outData.rainMorningRange = String(am.c_str());
    outData.rainEveningRange = String(pm.c_str());
  } else {
    outData.rainMorningRange = "--";
    outData.rainEveningRange = "--";
  }

  for (int i = 0; i < 3; ++i) {
    outData.nextDayLabel[i] = String(r.days[i].label.c_str());
    outData.nextDayIconCode[i] = String(r.days[i].iconCode.c_str());
    outData.nextDayMinC[i] = String(r.days[i].minC.c_str());
    outData.nextDayMaxC[i] = String(r.days[i].maxC.c_str());
    outData.nextDayRain[i] = String(r.days[i].rainRange.c_str());
    outData.nextDayRainChance[i] = String(r.days[i].rainChance.c_str());
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

static void setBacklightPercent(uint8_t percent) {
#if defined(TFT_BL) && (TFT_BL >= 0)
  percent = percent > 100 ? 100 : percent;
  if (hasPwmBacklight) {
    uint32_t duty = (255UL * percent) / 100UL;
    ledcWrite(0, duty);
  } else {
    digitalWrite(TFT_BL, percent > 0 ? TFT_BACKLIGHT_ON : !TFT_BACKLIGHT_ON);
  }
#else
  (void)percent;
#endif
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

static void initThemeColors() {
  // Night-first palette: muted contrast and no bright white fills.
  themeBg = tft.color565(1, 6, 12);
  themeHeader = tft.color565(6, 14, 22);
  themePanel = tft.color565(9, 20, 30);
  themeEdge = tft.color565(25, 70, 95);
  themeText = tft.color565(180, 205, 220);
  themeTextMuted = tft.color565(95, 130, 150);
  themeAccent = tft.color565(120, 180, 205);
  themeGood = tft.color565(105, 185, 135);
}

static void drawWeatherIcon(int x, int y, int size, const String &iconCode) {
  int code = iconCode.toInt();
  uint16_t sun = tft.color565(255, 196, 0);
  uint16_t cloud = tft.color565(170, 195, 215);
  uint16_t rain = tft.color565(90, 180, 255);
  uint16_t storm = tft.color565(255, 120, 40);

  auto drawSun = [&](int cx, int cy, int r) {
    tft.fillCircle(cx, cy, r, sun);
    for (int i = 0; i < 8; ++i) {
      float a = i * 3.14159f / 4.0f;
      int x1 = cx + (int)((r + 2) * cosf(a));
      int y1 = cy + (int)((r + 2) * sinf(a));
      int x2 = cx + (int)((r + 7) * cosf(a));
      int y2 = cy + (int)((r + 7) * sinf(a));
      tft.drawLine(x1, y1, x2, y2, sun);
    }
  };

  auto drawCloud = [&](int cx, int cy) {
    tft.fillCircle(cx - 8, cy, 7, cloud);
    tft.fillCircle(cx, cy - 4, 9, cloud);
    tft.fillCircle(cx + 10, cy, 7, cloud);
    tft.fillRoundRect(cx - 16, cy, 34, 11, 5, cloud);
  };

  if (code == 16 || code == 17) {
    drawCloud(x + size / 2, y + size / 2 - 2);
    tft.fillTriangle(x + size / 2 - 3, y + size / 2 + 10, x + size / 2 + 3, y + size / 2 + 10, x + size / 2 - 1, y + size / 2 + 18, storm);
    return;
  }
  if (code == 12 || code == 13 || code == 14) {
    drawCloud(x + size / 2, y + size / 2 - 4);
    tft.drawLine(x + size / 2 - 8, y + size / 2 + 10, x + size / 2 - 12, y + size / 2 + 16, rain);
    tft.drawLine(x + size / 2, y + size / 2 + 10, x + size / 2 - 4, y + size / 2 + 16, rain);
    tft.drawLine(x + size / 2 + 8, y + size / 2 + 10, x + size / 2 + 4, y + size / 2 + 16, rain);
    return;
  }
  if (code == 3 || code == 4 || code == 6 || code == 7 || code == 8) {
    drawSun(x + 12, y + 12, 6);
    drawCloud(x + size / 2 + 4, y + size / 2 - 2);
    return;
  }
  drawSun(x + size / 2, y + size / 2, 8);
}

static void drawNowAndDate() {
  tft.fillRect(8, 284, 224, 30, themePanel);
  tft.drawRoundRect(8, 284, 224, 30, 5, themeEdge);
  tft.setTextColor(themeGood, themePanel);
  tft.drawString(getCurrentTimeString(), 16, 287, 2);
  tft.drawString(getCurrentDateString(), 92, 287, 2);
}

static void drawHeader(const char *title) {
  tft.fillScreen(themeBg);
  tft.fillRect(0, 0, tft.width(), 30, themeHeader);
  tft.setTextColor(themeAccent, themeHeader);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, 8, 8, 2);
  tft.drawFastHLine(0, 28, tft.width(), themeEdge);
  drawRefreshIndicator(refreshAnimating);
}

static void drawRefreshIndicator(bool active) {
  const int cx = 223;
  const int cy = 14;
  const int r = 7;
  static const int8_t dx[8] = {0, 4, 6, 4, 0, -4, -6, -4};
  static const int8_t dy[8] = {-6, -4, 0, 4, 6, 4, 0, -4};

  tft.fillCircle(cx, cy, r + 2, themeHeader);
  tft.drawCircle(cx, cy, r, themeEdge);
  if (!active) return;

  for (int i = 0; i < 8; ++i) {
    uint16_t c = (i == spinnerFrame) ? themeAccent : themeTextMuted;
    tft.fillCircle(cx + dx[i], cy + dy[i], (i == spinnerFrame) ? 2 : 1, c);
  }
}

static void animateRefreshTick() {
  if (!refreshAnimating) return;
  uint32_t now = millis();
  if (now - lastSpinnerMs < 120UL) return;
  lastSpinnerMs = now;
  spinnerFrame = (spinnerFrame + 1) % 8;
  drawRefreshIndicator(true);
}

static void applyBacklightForTime() {
#if defined(TFT_BL) && (TFT_BL >= 0)
  uint8_t level = 30; // conservative default
  struct tm t;
  if (getLocalTime(&t, 20)) {
    // Darker at night to reduce emitted light
    if (t.tm_hour >= 20 || t.tm_hour < 6) level = 8;
    else if (t.tm_hour >= 6 && t.tm_hour < 9) level = 18;
    else level = 35;
  }
  setBacklightPercent(level);
#endif
}

static void drawWeather(const WeatherData &w) {
  drawHeader("BOM Weather");

  uint16_t card = themePanel;
  uint16_t cardEdge = themeEdge;

  // Main card
  tft.fillRoundRect(8, 36, 224, 130, 8, card);
  tft.drawRoundRect(8, 36, 224, 130, 8, cardEdge);
  tft.setTextColor(themeText, card);
  tft.drawString(w.stationName, 16, 42, 2);
  drawWeatherIcon(176, 50, 42, w.currentIconCode);

  tft.setTextColor(tft.color565(210, 196, 120), card);
  tft.drawString(w.airTempC + "C", 16, 66, 4);
  tft.setTextColor(themeAccent, card);
  tft.drawString("Feels " + w.apparentTempC + "C", 16, 102, 2);

  tft.setTextColor(themeTextMuted, card);
  tft.drawString("T " + w.dayMinTempC + "-" + w.dayMaxTempC + "C", 16, 124, 1);
  tft.drawString("F~ " + w.feelsMinTempC + "-" + w.feelsMaxTempC + "C", 120, 124, 1);

  // Details strip
  tft.fillRoundRect(8, 172, 224, 46, 7, card);
  tft.drawRoundRect(8, 172, 224, 46, 7, cardEdge);
  tft.setTextColor(themeText, card);
  tft.drawString("Hum " + w.relHumidityPct + "%", 14, 178, 2);
  tft.drawString("Wind " + w.windDir + " " + w.windKmh + "km/h", 110, 178, 2);
  tft.drawString("Rain day " + w.rainTodayRange, 14, 196, 1);
  tft.drawString("AM~ " + w.rainMorningRange + " PM~ " + w.rainEveningRange, 118, 196, 1);

  // Next days cards
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString("Next 3 Days", 8, 224, 2);
  for (int i = 0; i < 3; ++i) {
    int x = 8 + i * 76;
    int y = 246;
    tft.fillRoundRect(x, y, 72, 34, 6, card);
    tft.drawRoundRect(x, y, 72, 34, 6, cardEdge);
    drawWeatherIcon(x + 2, y + 2, 20, w.nextDayIconCode[i]);
    tft.setTextColor(themeText, card);
    String label = w.nextDayLabel[i].isEmpty() ? String("D+") + String(i + 1) : w.nextDayLabel[i];
    tft.drawString(label, x + 24, y + 3, 1);
    tft.drawString(w.nextDayMinC[i] + "-" + w.nextDayMaxC[i] + "C", x + 24, y + 14, 1);
    String rain = w.nextDayRain[i].isEmpty() ? w.nextDayRainChance[i] : w.nextDayRain[i];
    if (rain.isEmpty()) rain = "--";
    tft.drawString(rain, x + 24, y + 24, 1);
  }

  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("Obs " + w.observedTimeLocal, 8, 314, 1);
  drawNowAndDate();
}

static void drawStatus(const String &status, const String &detail) {
  drawHeader("ESP32 BOM Weather");
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(detail, 8, 84, 2);
}

static void refreshWeatherNow() {
  Serial.println("[weather] refresh start");
  refreshAnimating = true;
  spinnerFrame = 0;
  drawRefreshIndicator(true);
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
    refreshAnimating = false;
    drawRefreshIndicator(false);
    return;
  }

  ensureTimeConfigured();
  applyBacklightForTime();
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

    std::string fmin;
    std::string fmax;
    if (EstimateFeelsRange(
            std::string(fresh.airTempC.c_str()),
            std::string(fresh.apparentTempC.c_str()),
            std::string(fresh.dayMinTempC.c_str()),
            std::string(fresh.dayMaxTempC.c_str()),
            fmin,
            fmax)) {
      fresh.feelsMinTempC = String(fmin.c_str());
      fresh.feelsMaxTempC = String(fmax.c_str());
    }
    if (fresh.feelsMinTempC.isEmpty()) fresh.feelsMinTempC = "--";
    if (fresh.feelsMaxTempC.isEmpty()) fresh.feelsMaxTempC = "--";
    if (fresh.dayMinTempC.isEmpty()) fresh.dayMinTempC = "--";
    if (fresh.dayMaxTempC.isEmpty()) fresh.dayMaxTempC = "--";
    if (fresh.currentIconCode.isEmpty()) fresh.currentIconCode = "3";
    if (fresh.rainTodayChance.isEmpty()) fresh.rainTodayChance = "--";
    for (int i = 0; i < 3; ++i) {
      if (fresh.nextDayMinC[i].isEmpty()) fresh.nextDayMinC[i] = "--";
      if (fresh.nextDayMaxC[i].isEmpty()) fresh.nextDayMaxC[i] = "--";
    }

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
  refreshAnimating = false;
  drawRefreshIndicator(false);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("[boot] setup start");

  tft.init();
  tft.setRotation(0); // 240x320 portrait
  tft.invertDisplay(false);

#if defined(TFT_BL) && (TFT_BL >= 0)
  ledcSetup(0, 5000, 8);
  ledcAttachPin(TFT_BL, 0);
  hasPwmBacklight = true;
  setBacklightPercent(20);
#endif

  initThemeColors();
  drawStatus("Booting", "Starting Wi-Fi and display");
  Serial.println("[boot] display init done");
  refreshWeatherNow();
  lastRefreshMs = millis();
  lastBacklightAdjustMs = millis();
  Serial.println("[boot] setup complete");
}

void loop() {
  const uint32_t now = millis();
  if (latestData.valid && now - lastClockUpdateMs >= 1000UL) {
    drawNowAndDate();
    lastClockUpdateMs = now;
  }
  if (now - lastBacklightAdjustMs >= 60000UL) {
    applyBacklightForTime();
    lastBacklightAdjustMs = now;
  }
  animateRefreshTick();
  if (now - lastRefreshMs >= WEATHER_REFRESH_MS) {
    refreshWeatherNow();
    lastRefreshMs = now;
  }
  delay(100);
}
