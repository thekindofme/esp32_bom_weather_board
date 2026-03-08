#pragma once

#include <Arduino.h>

struct WeatherData {
  String stationName;
  String observedTimeLocal;
  String windDir;
  String airTempC;
  String apparentTempC;
  String relHumidityPct;
  String windKmh;
  String rainfallMm;
  String dayMinTempC;
  String dayMaxTempC;
  String feelsMinTempC;
  String feelsMaxTempC;
  String rainTodayRange;
  String rainMorningRange;
  String rainEveningRange;
  String rainTodayChance;
  String currentIconCode;
  String nextDayLabel[3];
  String nextDayIconCode[3];
  String nextDayMinC[3];
  String nextDayMaxC[3];
  String nextDayRain[3];
  String nextDayRainChance[3];
  bool valid = false;
};
