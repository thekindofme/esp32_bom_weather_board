#include <Arduino.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <string>

#include "BomParser.h"
#include "Config.h"
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "ForecastParser.h"
#include "FtpUtils.h"
#include "WeatherMath.h"
#include "WeatherTypes.h"

TFT_eSPI tft = TFT_eSPI();

uint32_t lastRefreshMs = 0;
uint32_t lastClockUpdateMs = 0;
uint32_t lastBacklightAdjustMs = 0;
String lastError;
WeatherData latestData;
bool ntpConfigured = false;
uint32_t lastSpinnerMs = 0;
bool hasPwmBacklight = false;
bool touchWasActive = false;
uint32_t lastTouchToggleMs = 0;
uint32_t touchDownMs = 0;

// Layout cycling
static constexpr uint8_t kMaxLayoutCount = 6;
static const LayoutFunctions *allLayouts[kMaxLayoutCount] = {};
const LayoutFunctions *activeLayout = &layoutHudGrid;
static uint8_t currentLayoutIndex = 0;
static uint8_t layoutCount = 0;
static Preferences prefs;

#define LONG_PRESS_MS 800UL

static void animateRefreshTick();

static void initAvailableLayouts() {
  layoutCount = 0;
  allLayouts[layoutCount++] = &layoutHudGrid;
  allLayouts[layoutCount++] = &layoutHeroTemp;
  allLayouts[layoutCount++] = &layoutRainFirst;
  allLayouts[layoutCount++] = &layoutNightwatch;
  allLayouts[layoutCount++] = &layoutNightwatchWide;
  if (hasGeneratedCustomLayout && layoutCount < kMaxLayoutCount) {
    allLayouts[layoutCount++] = &layoutCustomGenerated;
  }
}

static void selectLayoutByIndex(uint8_t index) {
  if (layoutCount == 0) {
    initAvailableLayouts();
  }
  if (index >= layoutCount) {
    index = 0;
  }
  currentLayoutIndex = index;
  activeLayout = allLayouts[currentLayoutIndex];
}

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


static void animateRefreshTick() {
  if (!refreshAnimating) return;
  uint32_t now = millis();
  if (now - lastSpinnerMs < 120UL) return;
  lastSpinnerMs = now;
  spinnerFrame = (spinnerFrame + 1) % 8;
  activeLayout->drawRefreshIndicator(tft, true, spinnerFrame);
}

static void applyThemeAndRedraw() {
  initThemeColors(tft);
  applyBacklightForTime();
  if (latestData.valid) {
    activeLayout->drawWeather(tft, latestData);
  } else {
    activeLayout->drawStatus(tft, "Ready", "Waiting for first weather update");
  }
}

static void applyLayoutRotation() {
  tft.setRotation(activeLayout->rotation);
}

static void cycleLayout() {
  if (layoutCount == 0) return;
  selectLayoutByIndex((currentLayoutIndex + 1) % layoutCount);
  prefs.putUChar("layout", currentLayoutIndex);
  applyLayoutRotation();
  applyThemeAndRedraw();
}

static void pollTouchActions() {
  // XPT2046 IRQ is active low while the panel is touched.
  const bool touchActive = (digitalRead(TOUCH_IRQ_PIN) == LOW);
  const uint32_t now = millis();

  if (touchActive && !touchWasActive) {
    // Finger just went down
    touchDownMs = now;
  } else if (!touchActive && touchWasActive && (now - lastTouchToggleMs >= TOUCH_TOGGLE_DEBOUNCE_MS)) {
    // Finger just lifted
    uint32_t held = now - touchDownMs;
    lastTouchToggleMs = now;
    if (held >= LONG_PRESS_MS) {
      // Long press: cycle layout
      cycleLayout();
    } else {
      // Short tap: toggle theme
      isLightTheme = !isLightTheme;
      applyThemeAndRedraw();
    }
  }
  touchWasActive = touchActive;
}

static void refreshWeatherNow() {
  Serial.println("[weather] refresh start");
  refreshAnimating = true;
  spinnerFrame = 0;
  activeLayout->drawRefreshIndicator(tft, true, spinnerFrame);
  if (!ensureWifiConnected()) {
    lastError = "Wi-Fi connect timeout";
    Serial.println("[weather] wifi failed");
    if (latestData.valid) {
      activeLayout->drawWeather(tft, latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 186, 2);
    } else {
      activeLayout->drawStatus(tft, "Wi-Fi failed", lastError);
    }
    refreshAnimating = false;
    activeLayout->drawRefreshIndicator(tft, false, spinnerFrame);
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
    activeLayout->drawWeather(tft, latestData);
  } else {
    lastError = err;
    Serial.print("[weather] fetch failed: ");
    Serial.println(lastError);
    if (latestData.valid) {
      activeLayout->drawWeather(tft, latestData);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WARN: " + lastError, 8, 186, 2);
    } else {
      activeLayout->drawStatus(tft, "BOM fetch failed", lastError);
    }
  }
  refreshAnimating = false;
  activeLayout->drawRefreshIndicator(tft, false, spinnerFrame);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("[boot] setup start");

  tft.init();
  tft.invertDisplay(false);

#if defined(TFT_BL) && (TFT_BL >= 0)
  ledcSetup(0, 5000, 8);
  ledcAttachPin(TFT_BL, 0);
  hasPwmBacklight = true;
  setBacklightPercent(20);
#endif

  pinMode(TOUCH_IRQ_PIN, INPUT);

  // Restore saved layout from NVS
  initAvailableLayouts();
  prefs.begin("weather", false);
  const uint32_t savedCustomFingerprint = prefs.getUInt("custom_fp", 0);
  const bool customLayoutChanged =
      hasGeneratedCustomLayout && savedCustomFingerprint != generatedCustomLayoutFingerprint;

  if (customLayoutChanged) {
    prefs.putUInt("custom_fp", generatedCustomLayoutFingerprint);
    const uint8_t customIndex = (layoutCount > 0) ? layoutCount - 1 : 0;
    prefs.putUChar("layout", customIndex);
    selectLayoutByIndex(customIndex);
  } else {
    selectLayoutByIndex(prefs.getUChar("layout", 0));
  }
  applyLayoutRotation();

  initThemeColors(tft);
  activeLayout->drawStatus(tft, "Booting", "Starting Wi-Fi and display");
  Serial.println("[boot] display init done");
  refreshWeatherNow();
  lastRefreshMs = millis();
  lastBacklightAdjustMs = millis();
  Serial.println("[boot] setup complete");
}

void loop() {
  const uint32_t now = millis();
  pollTouchActions();
  if (latestData.valid && now - lastClockUpdateMs >= 1000UL) {
    activeLayout->drawNowAndDate(tft);
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
