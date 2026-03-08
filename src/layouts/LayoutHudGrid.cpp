#include <TFT_eSPI.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

// Layout C: "HUD GRID" -- Maximum density, instrument panel style.
// 240x320 portrait. No toolbar -- date/time are part of the grid.
// Pure grid lines, no rounded rectangles. Uses every pixel.

static void hgDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
  const int cx = 231;
  const int cy = 13;
  const int r = 6;
  static const int8_t dx[8] = {0, 4, 5, 4, 0, -4, -5, -4};
  static const int8_t dy[8] = {-5, -4, 0, 4, 5, 4, 0, -4};

  tft.fillCircle(cx, cy, r + 2, themeBg);
  if (!active) return;

  for (int i = 0; i < 8; ++i) {
    uint16_t c = (i == frame) ? themeAccent : themeTextMuted;
    tft.fillCircle(cx + dx[i], cy + dy[i], (i == frame) ? 2 : 1, c);
  }
}

static void hgDrawNowAndDate(TFT_eSPI &tft) {
  const String dateStr = getCurrentDateStringShort();
  const String timeStr = getCurrentTimeString12h();

  tft.fillRect(0, 0, 220, 28, themeBg);

  tft.setTextColor(themeGood, themeBg);
  tft.drawString(dateStr, 6, 6, 2);

  tft.setTextColor(themeAccent, themeBg);
  int timeW = tft.textWidth(timeStr, 2);
  int timeX = 218 - timeW;
  if (timeX < 100) timeX = 100;
  tft.drawString(timeStr, timeX, 6, 2);
}

static void hgDrawHeader(TFT_eSPI &tft) {
  tft.fillScreen(themeBg);
  tft.setTextDatum(TL_DATUM);
  hgDrawNowAndDate(tft);
  tft.drawFastHLine(0, 28, 240, themeEdge);
  hgDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void hgDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  hgDrawHeader(tft);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(status, 8, 44, 2);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(detail, 8, 72, 2);
}

static void hgDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  hgDrawHeader(tft);

  // ---- 2-column top grid: LEFT temp+icon | RIGHT name+ranges ----
  const int topY = 29;
  const int midX = 96;
  const int topH = 84;

  tft.drawFastVLine(midX, topY, topH, themeEdge);
  tft.drawFastHLine(0, topY + topH, 240, themeEdge);

  // LEFT: temp + icon
  uint16_t tempColor = tft.color565(210, 196, 120);
  tft.setTextColor(tempColor, themeBg);
  tft.drawString(w.airTempC + String((char)0xB0) + "C", 8, topY + 6, 4);
  drawWeatherIcon(tft, 20, topY + 42, 36, w.currentIconCode);

  // RIGHT: station, feels, ranges
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.stationName, midX + 6, topY + 4, 2);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString("Feels " + w.apparentTempC + String((char)0xB0) + "C", midX + 6, topY + 24, 2);
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("Lo " + w.dayMinTempC + String((char)0xB0) + "  Hi " + w.dayMaxTempC + String((char)0xB0), midX + 6, topY + 48, 2);
  tft.drawString("F~Lo " + w.feelsMinTempC + "  F~Hi " + w.feelsMaxTempC, midX + 6, topY + 66, 1);

  // ---- 3-column conditions: RAIN | WIND | HUM ----
  const int condY = topY + topH + 1;
  const int condH = 52;
  const int col1 = 0, col2 = 80, col3 = 160;

  tft.drawFastVLine(col2, condY, condH, themeEdge);
  tft.drawFastVLine(col3, condY, condH, themeEdge);
  tft.drawFastHLine(0, condY + condH, 240, themeEdge);

  // Rain
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("RAIN", col1 + 6, condY + 4, 1);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(w.rainTodayChance, col1 + 6, condY + 16, 2);
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString(w.rainTodayRange, col1 + 6, condY + 36, 1);

  // Wind
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("WIND", col2 + 6, condY + 4, 1);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.windDir, col2 + 6, condY + 16, 2);
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString(w.windKmh + " km/h", col2 + 6, condY + 36, 1);

  // Humidity
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("HUM", col3 + 6, condY + 4, 1);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.relHumidityPct + "%", col3 + 6, condY + 16, 2);

  // ---- AM/PM split row ----
  const int ampmY = condY + condH + 1;
  const int ampmH = 36;

  tft.drawFastVLine(120, ampmY, ampmH, themeEdge);
  tft.drawFastHLine(0, ampmY + ampmH, 240, themeEdge);

  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("AM RAIN", 6, ampmY + 4, 1);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.rainMorningRange, 6, ampmY + 16, 2);

  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("PM RAIN", 126, ampmY + 4, 1);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.rainEveningRange, 126, ampmY + 16, 2);

  // ---- 4-column forecast grid ----
  const int fgY = ampmY + ampmH + 1;
  const int fc1 = 0, fc2 = 42, fc3 = 78, fc4 = 156;
  const int headerH = 16;
  const int rowH = 30;
  const int gridH = headerH + 3 * rowH;

  // Column headers
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("DAY", fc1 + 4, fgY + 3, 1);
  tft.drawString("TEMP", fc3 + 4, fgY + 3, 1);
  tft.drawString("RAIN", fc4 + 4, fgY + 3, 1);

  tft.drawFastHLine(0, fgY + headerH, 240, themeEdge);

  // Grid vertical lines
  tft.drawFastVLine(fc2, fgY, gridH, themeEdge);
  tft.drawFastVLine(fc3, fgY, gridH, themeEdge);
  tft.drawFastVLine(fc4, fgY, gridH, themeEdge);

  for (int i = 0; i < 3; ++i) {
    int rowY = fgY + headerH + 2 + i * rowH;

    String label = w.nextDayLabel[i].isEmpty() ? String("D+") + String(i + 1) : w.nextDayLabel[i];
    tft.setTextColor(themeAccent, themeBg);
    tft.drawString(label, fc1 + 4, rowY + 4, 2);

    // Small icon (16px)
    drawWeatherIcon(tft, fc2 + 6, rowY + 2, 16, w.nextDayIconCode[i]);

    tft.setTextColor(themeText, themeBg);
    tft.drawString(w.nextDayMinC[i] + "-" + w.nextDayMaxC[i] + String((char)0xB0), fc3 + 4, rowY + 4, 2);

    // Rain chance + range on two lines
    String rChance = w.nextDayRainChance[i];
    String rRange = w.nextDayRain[i];
    if (rChance.isEmpty() && rRange.isEmpty()) {
      tft.setTextColor(themeTextMuted, themeBg);
      tft.drawString("--", fc4 + 4, rowY + 4, 1);
    } else {
      if (!rChance.isEmpty() && rChance != "--") {
        tft.setTextColor(themeAccent, themeBg);
        tft.drawString(rChance, fc4 + 4, rowY + 2, 2);
      }
      if (!rRange.isEmpty()) {
        tft.setTextColor(themeTextMuted, themeBg);
        tft.drawString(rRange, fc4 + 4, rowY + 18, 1);
      }
    }

    if (i < 2) tft.drawFastHLine(0, fgY + headerH + (i + 1) * rowH, 240, themeEdge);
  }

  // Bottom border
  tft.drawFastHLine(0, fgY + gridH, 240, themeEdge);

  // Obs footer
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("Obs " + w.observedTimeLocal, 6, fgY + gridH + 3, 1);

  hgDrawNowAndDate(tft);
}

const LayoutFunctions layoutHudGrid = {
  hgDrawWeather,
  hgDrawHeader,
  hgDrawNowAndDate,
  hgDrawStatus,
  hgDrawRefreshIndicator,
  "HUD Grid",
  0 // portrait
};
