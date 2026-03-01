#include "WeatherMath.h"

#include <cmath>
#include <cstdio>
#include <ctime>

bool ParseFirstFloat(const std::string& s, float& value) {
  size_t i = 0;
  while (i < s.size() && !(std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '-' || s[i] == '+')) i++;
  size_t j = i;
  while (j < s.size() && (std::isdigit(static_cast<unsigned char>(s[j])) || s[j] == '.' || s[j] == '-' || s[j] == '+')) j++;
  if (i >= j) return false;
  value = std::stof(s.substr(i, j - i));
  return true;
}

bool ParseRangeToFloats(const std::string& range, float& lo, float& hi) {
  const size_t p = range.find(" to ");
  if (p == std::string::npos) return false;

  float a = 0.0f, b = 0.0f;
  if (!ParseFirstFloat(range.substr(0, p), a)) return false;
  if (!ParseFirstFloat(range.substr(p + 4), b)) return false;
  lo = a;
  hi = b;
  return true;
}

std::string FormatRangeRounded(float lo, float hi) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.0f-%.0f", lo, hi);
  return std::string(buf);
}

std::string DayLabelFromIso(const std::string& isoLocalTime) {
  if (isoLocalTime.size() < 10) return "--";
  std::tm t = {};
  t.tm_year = std::stoi(isoLocalTime.substr(0, 4)) - 1900;
  t.tm_mon = std::stoi(isoLocalTime.substr(5, 2)) - 1;
  t.tm_mday = std::stoi(isoLocalTime.substr(8, 2));
  t.tm_isdst = -1;
  if (std::mktime(&t) < 0) return "--";
  static const char* wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return wd[t.tm_wday];
}

bool EstimateHalfDayRainRange(const std::string& dayRange, std::string& morning, std::string& evening) {
  float lo = 0.0f, hi = 0.0f;
  if (!ParseRangeToFloats(dayRange, lo, hi)) return false;
  morning = "~" + FormatRangeRounded(lo / 2.0f, hi / 2.0f) + " mm";
  evening = "~" + FormatRangeRounded(lo / 2.0f, hi / 2.0f) + " mm";
  return true;
}

bool EstimateFeelsRange(
    const std::string& airTemp,
    const std::string& apparentTemp,
    const std::string& dayMin,
    const std::string& dayMax,
    std::string& feelsMin,
    std::string& feelsMax) {
  float air = 0.0f, feels = 0.0f, minA = 0.0f, maxA = 0.0f;
  if (!ParseFirstFloat(airTemp, air)) return false;
  if (!ParseFirstFloat(apparentTemp, feels)) return false;
  if (!ParseFirstFloat(dayMin, minA)) return false;
  if (!ParseFirstFloat(dayMax, maxA)) return false;
  float delta = feels - air;

  char b1[16], b2[16];
  std::snprintf(b1, sizeof(b1), "%.1f", minA + delta);
  std::snprintf(b2, sizeof(b2), "%.1f", maxA + delta);
  feelsMin = b1;
  feelsMax = b2;
  return true;
}
