#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>

#include "Config.h"
#include "DisplayCommon.h"
#include "WeatherMath.h"

// Theme color globals
uint16_t themeBg = 0;
uint16_t themeHeader = 0;
uint16_t themePanel = 0;
uint16_t themeEdge = 0;
uint16_t themeText = 0;
uint16_t themeTextMuted = 0;
uint16_t themeAccent = 0;
uint16_t themeGood = 0;

// State globals
bool isLightTheme = false;
bool refreshAnimating = false;
uint8_t spinnerFrame = 0;

extern bool hasPwmBacklight;

void initThemeColors(TFT_eSPI &tft) {
  if (isLightTheme) {
    themeBg = tft.color565(225, 234, 239);
    themeHeader = tft.color565(205, 222, 232);
    themePanel = tft.color565(243, 248, 251);
    themeEdge = tft.color565(98, 142, 168);
    themeText = tft.color565(24, 48, 64);
    themeTextMuted = tft.color565(88, 110, 124);
    themeAccent = tft.color565(31, 104, 146);
    themeGood = tft.color565(20, 128, 78);
  } else {
    themeBg = tft.color565(1, 6, 12);
    themeHeader = tft.color565(6, 14, 22);
    themePanel = tft.color565(9, 20, 30);
    themeEdge = tft.color565(25, 70, 95);
    themeText = tft.color565(180, 205, 220);
    themeTextMuted = tft.color565(95, 130, 150);
    themeAccent = tft.color565(120, 180, 205);
    themeGood = tft.color565(105, 185, 135);
  }
}

void setBacklightPercent(uint8_t percent) {
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

void applyBacklightForTime() {
#if defined(TFT_BL) && (TFT_BL >= 0)
  uint8_t level = isLightTheme ? 60 : 30;
  struct tm t;
  if (getLocalTime(&t, 20)) {
    if (t.tm_hour >= 20 || t.tm_hour < 6) level = isLightTheme ? 35 : 8;
    else if (t.tm_hour >= 6 && t.tm_hour < 9) level = isLightTheme ? 50 : 18;
    else level = isLightTheme ? 70 : 35;
  }
  setBacklightPercent(level);
#endif
}

String getCurrentTimeString12h() {
  struct tm t;
  if (!getLocalTime(&t, 50)) {
    return "--:--:-- --";
  }
  return String(FormatTime12h(t.tm_hour, t.tm_min, t.tm_sec).c_str());
}

String getCurrentDateStringShort() {
  struct tm t;
  if (!getLocalTime(&t, 50)) {
    return "--- -- ---";
  }
  return String(FormatDateShortFromYMD(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday).c_str());
}

void drawWeatherIcon(TFT_eSPI &tft, int x, int y, int size, const String &iconCode) {
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
