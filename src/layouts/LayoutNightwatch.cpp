#include <TFT_eSPI.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

// Layout D: "NIGHTWATCH" -- Red-on-black, minimal, zero light pollution.
// Three horizontal bands. Everything centered. No chrome, no labels, no clutter.
// Ignores theme toggle -- always black/red.
//
// ┌──────────────────────────────────────┐
// │                                      │
// │            10:42:35 PM               │  TIME  (Font4, bright red)
// │            Sat 08 Mar                │  DATE  (Font2, dim red)
// │                                      │
// ├──────────────────────────────────────┤
// │                                      │
// │               23°                    │  TEMP  (Font4×2, bright red)
// │            feels 21°                 │  FEELS (Font2, dim red)
// │                                      │
// ├──────────────────────────────────────┤
// │                                      │
// │             20%                      │  RAIN% (Font4, accent red)
// │            0-4 mm                    │  RANGE (Font2, dim red)
// │                                      │
// └──────────────────────────────────────┘

// Fixed night palette -- never changes.
static uint16_t nwBlack;
static uint16_t nwBright;
static uint16_t nwDim;
static uint16_t nwAccent;
static uint16_t nwLine;

static void nwInitColors(TFT_eSPI &tft) {
  nwBlack  = TFT_BLACK;
  nwBright = tft.color565(180, 0, 0);
  nwDim    = tft.color565(80, 0, 0);
  nwAccent = tft.color565(200, 30, 0);
  nwLine   = tft.color565(40, 0, 0);
}

static bool nwColorsInit = false;

static void nwEnsureColors(TFT_eSPI &tft) {
  if (!nwColorsInit) {
    nwInitColors(tft);
    nwColorsInit = true;
  }
}

// Band boundaries -- generous sizing for across-room readability
static const int BAND1_Y = 0;
static const int BAND1_H = 80;
static const int LINE1_Y = 80;
static const int BAND2_Y = 81;
static const int BAND2_H = 130;
static const int LINE2_Y = 211;
static const int BAND3_Y = 212;
static const int BAND3_H = 108; // to y=320

static void nwDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
  nwEnsureColors(tft);
  const int cx = 231;
  const int cy = 10;
  const int r = 5;
  static const int8_t dx[8] = {0, 3, 4, 3, 0, -3, -4, -3};
  static const int8_t dy[8] = {-4, -3, 0, 3, 4, 3, 0, -3};

  tft.fillCircle(cx, cy, r + 2, nwBlack);
  if (!active) return;

  for (int i = 0; i < 8; ++i) {
    uint16_t c = (i == frame) ? nwAccent : nwDim;
    tft.fillCircle(cx + dx[i], cy + dy[i], (i == frame) ? 2 : 1, c);
  }
}

static String nwTimeNoSeconds() {
  struct tm t;
  if (!getLocalTime(&t, 50)) return "--:-- --";
  bool pm = t.tm_hour >= 12;
  int h = t.tm_hour % 12;
  if (h == 0) h = 12;
  char buf[12];
  snprintf(buf, sizeof(buf), "%d:%02d %s", h, t.tm_min, pm ? "PM" : "AM");
  return String(buf);
}

static void nwDrawNowAndDate(TFT_eSPI &tft) {
  nwEnsureColors(tft);

  const String timeStr = nwTimeNoSeconds();
  const String dateStr = getCurrentDateStringShort();

  // Clear band 1 (except spinner corner)
  tft.fillRect(0, BAND1_Y, 222, BAND1_H, nwBlack);

  // Time -- Font4, centered
  int tw = tft.textWidth(timeStr, 4);
  int tx = (240 - tw) / 2;
  tft.setTextColor(nwBright, nwBlack);
  tft.drawString(timeStr, tx, BAND1_Y + 8, 4);

  // Date -- Font4, prominent, centered below
  int dw = tft.textWidth(dateStr, 4);
  int dx = (240 - dw) / 2;
  tft.setTextColor(nwDim, nwBlack);
  tft.drawString(dateStr, dx, BAND1_Y + 42, 4);
}

static void nwDrawHeader(TFT_eSPI &tft) {
  nwEnsureColors(tft);
  tft.fillScreen(nwBlack);
  tft.setTextDatum(TL_DATUM);

  // Hairlines
  tft.drawFastHLine(0, LINE1_Y, 240, nwLine);
  tft.drawFastHLine(0, LINE2_Y, 240, nwLine);

  nwDrawNowAndDate(tft);
  nwDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void nwDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  nwEnsureColors(tft);
  nwDrawHeader(tft);

  int sw = tft.textWidth(status, 4);
  tft.setTextColor(nwBright, nwBlack);
  tft.drawString(status, (240 - sw) / 2, BAND2_Y + 30, 4);

  int dw = tft.textWidth(detail, 2);
  tft.setTextColor(nwDim, nwBlack);
  tft.drawString(detail, (240 - dw) / 2, BAND2_Y + 65, 2);
}

static void nwDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  nwDrawHeader(tft);

  // ---- BAND 2: Temperature ----
  // Huge temp centered -- Font4 at size 2 (~52px tall)
  String tempStr = w.airTempC + String((char)0xB0);
  tft.setTextSize(2);
  int tw2 = tft.textWidth(tempStr, 4);
  int tx2 = (240 - tw2) / 2;
  tft.setTextColor(nwBright, nwBlack);
  tft.drawString(tempStr, tx2, BAND2_Y + 10, 4);
  tft.setTextSize(1);

  // Feels -- Font4 normal, centered below
  String feelsStr = "feels " + w.apparentTempC + String((char)0xB0);
  int fw = tft.textWidth(feelsStr, 4);
  tft.setTextColor(nwDim, nwBlack);
  tft.drawString(feelsStr, (240 - fw) / 2, BAND2_Y + 82, 4);

  // ---- BAND 3: Rain ----
  // Big rain chance -- Font4 at size 2
  String rainPct = w.rainTodayChance;
  if (rainPct.isEmpty() || rainPct == "--") rainPct = "0%";

  tft.setTextSize(2);
  int rw = tft.textWidth(rainPct, 4);
  tft.setTextColor(nwAccent, nwBlack);
  tft.drawString(rainPct, (240 - rw) / 2, BAND3_Y + 10, 4);
  tft.setTextSize(1);

  // Range -- Font2, centered below
  String rainRange = w.rainTodayRange;
  if (rainRange.isEmpty()) rainRange = "--";
  int rrw = tft.textWidth(rainRange, 2);
  tft.setTextColor(nwDim, nwBlack);
  tft.drawString(rainRange, (240 - rrw) / 2, BAND3_Y + 72, 2);

  nwDrawNowAndDate(tft);
}

const LayoutFunctions layoutNightwatch = {
  nwDrawWeather,
  nwDrawHeader,
  nwDrawNowAndDate,
  nwDrawStatus,
  nwDrawRefreshIndicator,
  "Nightwatch",
  0 // portrait
};
