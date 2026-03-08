#include <TFT_eSPI.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

// Layout B: "RAIN FIRST" -- Rain gets a hero band in the center.
// 240x320 portrait, header 28px.

static void rfDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
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

static void rfDrawNowAndDate(TFT_eSPI &tft) {
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

static void rfDrawHeader(TFT_eSPI &tft) {
  tft.fillScreen(themeBg);
  tft.fillRect(0, 0, tft.width(), 28, themeHeader);
  tft.setTextColor(themeAccent, themeHeader);
  tft.setTextDatum(TL_DATUM);
  tft.drawFastHLine(0, 28, tft.width(), themeEdge);
  rfDrawNowAndDate(tft);
  rfDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void rfDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  rfDrawHeader(tft);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(detail, 8, 84, 2);
}

static uint16_t rainChanceColor(TFT_eSPI &tft, const String &chance) {
  int pct = chance.toInt();
  if (pct >= 70) return tft.color565(255, 120, 40); // orange
  if (pct >= 40) return tft.color565(120, 180, 205); // accent-ish
  return tft.color565(105, 185, 135); // green
}

static void rfDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  rfDrawHeader(tft);

  uint16_t card = themePanel;
  uint16_t edge = themeEdge;

  // Compact temp row: station + icon + Font4 temp
  tft.setTextColor(themeText, themeBg);
  tft.drawString(w.stationName, 14, 32, 2);
  drawWeatherIcon(tft, 150, 30, 36, w.currentIconCode);

  uint16_t tempColor = tft.color565(210, 196, 120);
  tft.setTextColor(tempColor, themeBg);
  tft.drawString(w.airTempC + String((char)0xB0), 194, 32, 4);

  // Feels + ranges
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString("Feels " + w.apparentTempC + String((char)0xB0), 14, 52, 2);
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString(w.dayMinTempC + "-" + w.dayMaxTempC, 130, 54, 1);
  tft.drawString("F~" + w.feelsMinTempC + "-" + w.feelsMaxTempC, 180, 54, 1);

  // ---- RAIN HERO BAND (tinted background) ----
  int ry = 74;
  int rh = 96;
  uint16_t rainBg = isLightTheme ? tft.color565(215, 230, 242) : tft.color565(6, 18, 32);
  tft.fillRect(0, ry, 240, rh, rainBg);
  tft.drawFastHLine(0, ry, 240, edge);
  tft.drawFastHLine(0, ry + rh - 1, 240, edge);

  // "RAIN TODAY" label + big chance %
  tft.setTextColor(themeText, rainBg);
  tft.drawString("RAIN TODAY", 10, ry + 6, 2);
  uint16_t chanceCol = rainChanceColor(tft, w.rainTodayChance);
  tft.setTextColor(chanceCol, rainBg);
  tft.drawString(w.rainTodayChance, 170, ry + 2, 4);

  // AM/PM sub-cards
  int cx1 = 10, cx2 = 124;
  int cy = ry + 28;
  int cw = 106, ch = 38;

  tft.fillRoundRect(cx1, cy, cw, ch, 5, card);
  tft.drawRoundRect(cx1, cy, cw, ch, 5, edge);
  tft.setTextColor(themeAccent, card);
  tft.drawString("MORNING", cx1 + 6, cy + 4, 1);
  tft.setTextColor(themeText, card);
  tft.drawString(w.rainMorningRange, cx1 + 6, cy + 16, 2);

  tft.fillRoundRect(cx2, cy, cw, ch, 5, card);
  tft.drawRoundRect(cx2, cy, cw, ch, 5, edge);
  tft.setTextColor(themeAccent, card);
  tft.drawString("AFTERNOON", cx2 + 6, cy + 4, 1);
  tft.setTextColor(themeText, card);
  tft.drawString(w.rainEveningRange, cx2 + 6, cy + 16, 2);

  // Total rain
  tft.setTextColor(themeTextMuted, rainBg);
  tft.drawString("Total: " + w.rainTodayRange, 10, ry + rh - 18, 1);

  // Wind + humidity line
  int wy = ry + rh + 4;
  tft.setTextColor(themeText, themeBg);
  tft.drawString("WIND " + w.windDir + " " + w.windKmh + "km/h", 10, wy, 2);
  tft.drawString("HUM " + w.relHumidityPct + "%", 176, wy, 2);

  // Forecast table
  int fy = wy + 22;
  tft.drawFastHLine(0, fy, 240, edge);
  fy += 4;

  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("DAY", 8, fy, 1);
  tft.drawString("ICON", 48, fy, 1);
  tft.drawString("TEMP", 96, fy, 1);
  tft.drawString("RAIN", 168, fy, 1);
  fy += 14;

  for (int i = 0; i < 3; ++i) {
    int rowY = fy + i * 22;
    String label = w.nextDayLabel[i].isEmpty() ? String("D+") + String(i + 1) : w.nextDayLabel[i];
    tft.setTextColor(themeAccent, themeBg);
    tft.drawString(label, 8, rowY, 2);
    drawWeatherIcon(tft, 44, rowY - 2, 20, w.nextDayIconCode[i]);
    tft.setTextColor(themeText, themeBg);
    tft.drawString(w.nextDayMinC[i] + "-" + w.nextDayMaxC[i] + String((char)0xB0), 80, rowY, 2);

    String rChance = w.nextDayRainChance[i];
    String rRange = w.nextDayRain[i];
    String rLine = "";
    if (!rChance.isEmpty() && rChance != "--") rLine += rChance;
    if (!rRange.isEmpty()) {
      if (!rLine.isEmpty()) rLine += " ";
      rLine += rRange;
    }
    if (rLine.isEmpty()) rLine = "--";
    uint16_t rc = rainChanceColor(tft, rChance);
    tft.setTextColor(rc, themeBg);
    tft.drawString(rLine, 155, rowY, 2);
  }

  // Obs time
  tft.setTextColor(themeTextMuted, themeBg);
  tft.drawString("Obs " + w.observedTimeLocal, 8, 310, 1);

  rfDrawNowAndDate(tft);
}

const LayoutFunctions layoutRainFirst = {
  rfDrawWeather,
  rfDrawHeader,
  rfDrawNowAndDate,
  rfDrawStatus,
  rfDrawRefreshIndicator,
  "Rain First",
  0 // portrait
};
