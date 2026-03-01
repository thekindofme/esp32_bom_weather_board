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

std::string FormatTime12h(int hour24, int minute, int second) {
  if (hour24 < 0 || hour24 > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
    return "--:--:-- --";
  }
  bool pm = hour24 >= 12;
  int hour12 = hour24 % 12;
  if (hour12 == 0) hour12 = 12;
  char buf[20];
  std::snprintf(buf, sizeof(buf), "%d:%02d:%02d %s", hour12, minute, second, pm ? "PM" : "AM");
  return std::string(buf);
}

std::string FormatDateShortFromYMD(int year, int month, int day) {
  if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31) return "--- -- ---";
  std::tm t = {};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_isdst = -1;
  if (std::mktime(&t) < 0) return "--- -- ---";
  static const char* wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  static const char* mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  if (t.tm_mon < 0 || t.tm_mon > 11 || t.tm_wday < 0 || t.tm_wday > 6) return "--- -- ---";
  char buf[24];
  std::snprintf(buf, sizeof(buf), "%s %02d %s", wd[t.tm_wday], t.tm_mday, mon[t.tm_mon]);
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
