#include <TFT_eSPI.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

// Layout A: "HERO TEMP" -- Temperature-dominant, readable from across the room.
// 240x320 portrait, header 28px.

static void htDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
  const int cx = 223;
  const int cy = 14;
  const int r = 7;
  static const int8_t dx[8] = {0, 4, 6, 4, 0, -4, -6, -4};
  static const int8_t dy[8] = {-6, -4, 0, 4, 6, 4, 0, -4};

  tft.fillCircle(cx, cy, r + 2, themeHeader);
  tft.drawCircle(cx, cy, r, themeEdge);
  if (!active) return;

  for (int i = 0; i < 8; ++i) {
    uint16_t c = (i == frame) ? themeAccent : themeTextMuted;
    tft.fillCircle(cx + dx[i], cy + dy[i], (i == frame) ? 2 : 1, c);
  }
}

static void htDrawNowAndDate(TFT_eSPI &tft) {
  const String dateStr = getCurrentDateStringShort();
  const String timeStr = getCurrentTimeString12h();

  tft.fillRect(4, 2, 210, 24, themeHeader);
  tft.setTextColor(themeGood, themeHeader);
  tft.drawString(dateStr, 8, 8, 2);

  int timeW = tft.textWidth(timeStr, 2);
  int timeX = tft.width() - 28 - timeW;
  if (timeX < 92) timeX = 92;
  tft.drawString(timeStr, timeX, 8, 2);
}

static void htDrawHeader(TFT_eSPI &tft) {
  tft.fillScreen(themeBg);
  tft.fillRect(0, 0, tft.width(), 28, themeHeader);
  tft.setTextColor(themeAccent, themeHeader);
  tft.setTextDatum(TL_DATUM);
  tft.drawFastHLine(0, 28, tft.width(), themeEdge);
  htDrawNowAndDate(tft);
  htDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void htDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  htDrawHeader(tft);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(detail, 8, 84, 2);
}

static void htDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  htDrawHeader(tft);

  uint16_t card = themePanel;
  uint16_t edge = themeEdge;

  // Station name + weather icon (48x48)
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.stationName, 14, 34, 2);
  drawWeatherIcon(tft, 180, 34, 48, w.currentIconCode);

  // BIG temperature -- Font4 at setTextSize(2) for ~44px tall digits
  uint16_t tempColor = tft.color565(210, 196, 120);
  tft.setTextColor(tempColor, themeBg);
  tft.setTextSize(2);
  tft.drawString(w.airTempC + String((char)0xB0), 14, 58, 4);
  tft.setTextSize(1);

  // Feels like
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString("Feels " + w.apparentTempC + String((char)0xB0), 14, 108, 2);

  // Temp ranges
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString(w.dayMinTempC + "-" + w.dayMaxTempC + String((char)0xB0) + "C", 14, 126, 1);
  tft.drawString("F~ " + w.feelsMinTempC + "-" + w.feelsMaxTempC + String((char)0xB0) + "C", 120, 126, 1);

  // Rain row
  int ry = 140;
  tft.drawFastHLine(8, ry - 2, 224, themeEdge);
  tft.setTextColor(themeText, themeBg);
  tft.drawString("RAIN", 14, ry + 2, 2);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(w.rainTodayChance, 62, ry + 2, 2);
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString(w.rainTodayRange, 130, ry + 2, 2);

  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("AM~ " + w.rainMorningRange, 14, ry + 20, 1);
  tft.drawString("PM~ " + w.rainEveningRange, 120, ry + 20, 1);

  // Wind + humidity
  tft.setTextColor(themeText, themeBg);
  tft.drawString("WIND " + w.windDir + " " + w.windKmh + "km/h", 14, ry + 34, 2);
  tft.drawString("HUM " + w.relHumidityPct + "%", 170, ry + 34, 2);

  // Forecast cards (3 cards, 76px wide, 58px tall)
  int fy = 198;
  tft.drawFastHLine(8, fy - 4, 224, themeEdge);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString("Next 3 Days", 8, fy, 2);
  fy += 20;

  for (int i = 0; i < 3; ++i) {
    int x = 4 + i * 78;
    tft.fillRoundRect(x, fy, 76, 58, 6, card);
    tft.drawRoundRect(x, fy, 76, 58, 6, edge);

    drawWeatherIcon(tft, x + 4, fy + 2, 20, w.nextDayIconCode[i]);
    String label = w.nextDayLabel[i].isEmpty() ? String("D+") + String(i + 1) : w.nextDayLabel[i];
    tft.setTextColor(themeAccent, card);
    tft.drawString(label, x + 28, fy + 4, 2);
    tft.setTextColor(themeText, card);
    tft.drawString(w.nextDayMinC[i] + "-" + w.nextDayMaxC[i] + String((char)0xB0), x + 6, fy + 24, 2);

    String rain = w.nextDayRainChance[i];
    if (rain.isEmpty() || rain == "--") rain = "";
    String rRange = w.nextDayRain[i];
    String rLine = "";
    if (!rain.isEmpty()) rLine += rain;
    if (!rRange.isEmpty()) {
      if (!rLine.isEmpty()) rLine += " ";
      rLine += rRange;
    }
    if (rLine.isEmpty()) rLine = "--";
    tft.setTextColor(themeTextMuted, card);
    tft.drawString(rLine, x + 6, fy + 42, 1);
  }

  // Obs time
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("Obs " + w.observedTimeLocal, 8, 310, 1);

  htDrawNowAndDate(tft);
}

const LayoutFunctions layoutHeroTemp = {
  htDrawWeather,
  htDrawHeader,
  htDrawNowAndDate,
  htDrawStatus,
  htDrawRefreshIndicator,
  "Hero Temp",
  0 // portrait
};
