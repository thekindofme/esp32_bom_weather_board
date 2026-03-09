#include <TFT_eSPI.h>
#include "DisplayCommon.h"
#include "DisplayLayout.h"
#include "WeatherTypes.h"

const bool hasGeneratedCustomLayout = false;
const uint32_t generatedCustomLayoutFingerprint = 0;

static void generatedCustomDrawRefreshIndicator(TFT_eSPI &tft, bool active, uint8_t frame) {
  const int cx = tft.width() - 17;
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

static void generatedCustomDrawNowAndDate(TFT_eSPI &tft) {
  (void)tft;
}

static void generatedCustomDrawHeader(TFT_eSPI &tft) {
  tft.fillScreen(themeBg);
  tft.fillRect(0, 0, tft.width(), 28, themeHeader);
  tft.setTextDatum(TL_DATUM);
  tft.drawFastHLine(0, 28, tft.width(), themeEdge);
  generatedCustomDrawRefreshIndicator(tft, refreshAnimating, spinnerFrame);
}

static void generatedCustomDrawStatus(TFT_eSPI &tft, const String &status, const String &detail) {
  generatedCustomDrawHeader(tft);
  tft.setTextColor(themeAccent, themeBg);
  tft.drawString(status, 8, 54, 2);
  tft.setTextColor(themeText, themeBg);
  tft.drawString(detail, 8, 84, 2);
}

static void generatedCustomDrawWeather(TFT_eSPI &tft, const WeatherData &w) {
  (void)w;
  generatedCustomDrawStatus(tft, "Custom layout", "Flash from the web designer");
}

const LayoutFunctions layoutCustomGenerated = {
  generatedCustomDrawWeather,
  generatedCustomDrawHeader,
  generatedCustomDrawNowAndDate,
  generatedCustomDrawStatus,
  generatedCustomDrawRefreshIndicator,
  "Generated Custom",
  0
};
