#include <TFT_eSPI.h>
#include <time.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

// Layout E: "NIGHTWATCH WIDE" -- Landscape red-on-black.
// 320x240 (rotation 1). Two columns: left=time+date, right=temp+rain.
//
// ┌────────────────────┬─────────────────────┐
// │                    │                     │
// │     10:42 PM       │       23°           │  Time: Font4 | Temp: Font4×2
// │                    │                     │
// │    Sat 08 Mar      │    feels 21°        │  Date: Font4 | Feels: Font4
// │                    │                     │
// │                    ├─────────────────────┤
// │                    │                     │
// │                    │       20%           │  Rain: Font4×2
// │                    │      0-4 mm         │  Range: Font2
// │                    │                     │
// └────────────────────┴─────────────────────┘

// Fixed night palette
static uint16_t nwwBlack;
static uint16_t nwwBright;
static uint16_t nwwDim;
static uint16_t nwwAccent;
static uint16_t nwwLine;
static bool nwwColorsInit = false;

static void nwwInitColors(TFT_eSPI &tft) {
  nwwBlack  = TFT_BLACK;
  nwwBright = tft.color565(180, 0, 0);
  nwwDim    = tft.color565(80, 0, 0);
  nwwAccent = tft.color565(200, 30, 0);
  nwwLine   = tft.color565(40, 0, 0);
}

static void nwwEnsureColors(TFT_eSPI &tft) {
  if (!nwwColorsInit) { nwwInitColors(tft); nwwColorsInit = true; }
}

// 320x240 layout constants
static const int W = 320;
static const int H = 240;
static const int MID_X = 160;  // vertical divider
static const int MID_Y = 130;  // horizontal divider (right column only)

static String nwwTimeNoSeconds() {
  struct tm t;
  if (!getLocalTime(&t, 50)) return "--:-- --";
  bool pm = t.tm_hour >= 12;
  int h = t.tm_hour % 12;
  if (h == 0) h = 12;
  char buf[12];
  snprintf(buf, sizeof(buf), "%d:%02d %s", h, t.tm_min, pm ? "PM" : "AM");
  return String(buf);
}

static void nwwDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
  nwwEnsureColors(tft);
  const int cx = 310;
  const int cy = 10;
  static const int8_t dx[8] = {0, 3, 4, 3, 0, -3, -4, -3};
  static const int8_t dy[8] = {-4, -3, 0, 3, 4, 3, 0, -3};

  tft.fillCircle(cx, cy, 8, nwwBlack);
  if (!active) return;

  for (int i = 0; i < 8; ++i) {
    uint16_t c = (i == frame) ? nwwAccent : nwwDim;
    tft.fillCircle(cx + dx[i], cy + dy[i], (i == frame) ? 2 : 1, c);
  }
}

static void nwwDrawNowAndDate(TFT_eSPI &tft) {
  nwwEnsureColors(tft);

  const String timeStr = nwwTimeNoSeconds();
  const String dateStr = getCurrentDateStringShort();

  // Left column: time + date, vertically centered
  int lx = MID_X / 2; // center of left column

  // Clear left column
  tft.fillRect(0, 0, MID_X - 1, H, nwwBlack);

  // Time -- Font4, centered in left column
  int tw = tft.textWidth(timeStr, 4);
  tft.setTextColor(nwwBright, nwwBlack);
  tft.drawString(timeStr, lx - tw / 2, H / 2 - 40, 4);

  // Date -- Font4, centered below
  int dw = tft.textWidth(dateStr, 4);
  tft.setTextColor(nwwDim, nwwBlack);
  tft.drawString(dateStr, lx - dw / 2, H / 2 + 4, 4);
}

static void nwwDrawHeader(TFT_eSPI &tft) {
  nwwEnsureColors(tft);
  tft.fillScreen(nwwBlack);
  tft.setTextDatum(TL_DATUM);

  // Vertical divider (full height)
  tft.drawFastVLine(MID_X, 0, H, nwwLine);
  // Horizontal divider (right column only)
  tft.drawFastHLine(MID_X, MID_Y, W - MID_X, nwwLine);

  nwwDrawNowAndDate(tft);
  nwwDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void nwwDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  nwwDrawHeader(tft);
  int rx = MID_X + (W - MID_X) / 2;

  int sw = tft.textWidth(status, 4);
  tft.setTextColor(nwwBright, nwwBlack);
  tft.drawString(status, rx - sw / 2, 60, 4);

  int dw = tft.textWidth(detail, 2);
  tft.setTextColor(nwwDim, nwwBlack);
  tft.drawString(detail, rx - dw / 2, 100, 2);
}

static void nwwDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  nwwDrawHeader(tft);

  int rx = MID_X + (W - MID_X) / 2; // center of right column

  // ---- Right top: Temperature ----
  // Huge temp -- Font4 size 2
  String tempStr = w.airTempC + String((char)0xB0);
  tft.setTextSize(2);
  int tw2 = tft.textWidth(tempStr, 4);
  tft.setTextColor(nwwBright, nwwBlack);
  tft.drawString(tempStr, rx - tw2 / 2, 16, 4);
  tft.setTextSize(1);

  // Feels -- Font4
  String feelsStr = "feels " + w.apparentTempC + String((char)0xB0);
  int fw = tft.textWidth(feelsStr, 4);
  tft.setTextColor(nwwDim, nwwBlack);
  tft.drawString(feelsStr, rx - fw / 2, 82, 4);

  // ---- Right bottom: Rain ----
  String rainPct = w.rainTodayChance;
  if (rainPct.isEmpty() || rainPct == "--") rainPct = "0%";

  // Big rain -- Font4 size 2
  tft.setTextSize(2);
  int rw = tft.textWidth(rainPct, 4);
  tft.setTextColor(nwwAccent, nwwBlack);
  tft.drawString(rainPct, rx - rw / 2, MID_Y + 14, 4);
  tft.setTextSize(1);

  // Range
  String rainRange = w.rainTodayRange;
  if (rainRange.isEmpty()) rainRange = "--";
  int rrw = tft.textWidth(rainRange, 2);
  tft.setTextColor(nwwDim, nwwBlack);
  tft.drawString(rainRange, rx - rrw / 2, MID_Y + 78, 2);

  nwwDrawNowAndDate(tft);
}

const LayoutFunctions layoutNightwatchWide = {
  nwwDrawWeather,
  nwwDrawHeader,
  nwwDrawNowAndDate,
  nwwDrawStatus,
  nwwDrawRefreshIndicator,
  "Nightwatch Wide",
  1 // landscape
};
