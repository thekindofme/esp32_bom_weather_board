#pragma once

#include <string>

bool ParseFirstFloat(const std::string& s, float& value);
bool ParseRangeToFloats(const std::string& range, float& lo, float& hi);
std::string FormatRangeRounded(float lo, float hi);
std::string DayLabelFromIso(const std::string& isoLocalTime);
bool EstimateHalfDayRainRange(const std::string& dayRange, std::string& morning, std::string& evening);
bool EstimateFeelsRange(
    const std::string& airTemp,
    const std::string& apparentTemp,
    const std::string& dayMin,
    const std::string& dayMax,
    std::string& feelsMin,
    std::string& feelsMax);
