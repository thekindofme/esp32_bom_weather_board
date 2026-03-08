#pragma once

#include <TFT_eSPI.h>

// Theme color globals
extern uint16_t themeBg;
extern uint16_t themeHeader;
extern uint16_t themePanel;
extern uint16_t themeEdge;
extern uint16_t themeText;
extern uint16_t themeTextMuted;
extern uint16_t themeAccent;
extern uint16_t themeGood;

// State globals
extern bool isLightTheme;
extern bool refreshAnimating;
extern uint8_t spinnerFrame;

// Theme management
void initThemeColors(TFT_eSPI &tft);

// Backlight
void setBacklightPercent(uint8_t percent);
void applyBacklightForTime();

// Time string helpers
String getCurrentTimeString12h();
String getCurrentDateStringShort();

// Shared drawing helpers
void drawWeatherIcon(TFT_eSPI &tft, int x, int y, int size, const String &iconCode);
