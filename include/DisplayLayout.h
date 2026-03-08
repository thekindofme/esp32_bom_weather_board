#pragma once

#include <TFT_eSPI.h>
#include "WeatherTypes.h"

// Function pointer table that each layout provides.
struct LayoutFunctions {
  void (*drawWeather)(TFT_eSPI &tft, const WeatherData &w);
  void (*drawHeader)(TFT_eSPI &tft);
  void (*drawNowAndDate)(TFT_eSPI &tft);
  void (*drawStatus)(TFT_eSPI &tft, const String &status, const String &detail);
  void (*drawRefreshIndicator)(TFT_eSPI &tft, bool active, uint8_t frame);
  const char *name;
  uint8_t rotation; // 0=portrait, 1=landscape
};

// Layout instances (defined in each Layout*.cpp)
extern const LayoutFunctions layoutHeroTemp;
extern const LayoutFunctions layoutRainFirst;
extern const LayoutFunctions layoutHudGrid;
extern const LayoutFunctions layoutNightwatch;
extern const LayoutFunctions layoutNightwatchWide;

#define LAYOUT_COUNT 5

// Active layout pointer (managed by main.cpp)
extern const LayoutFunctions *activeLayout;
