#include "ForecastParser.h"

#include "WeatherMath.h"

#include <algorithm>

namespace {

std::string TrimCopy(const std::string& s) {
  const auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
  if (first == s.end()) return "";
  const auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
  return std::string(first, last);
}

std::string ExtractXmlValue(const std::string& line) {
  const std::size_t gt = line.find('>');
  if (gt == std::string::npos) return "";
  const std::size_t lt = line.find('<', gt + 1);
  if (lt == std::string::npos || lt <= gt + 1) return "";
  return line.substr(gt + 1, lt - (gt + 1));
}

std::string ExtractAttr(const std::string& line, const char* attr) {
  const std::string key = std::string(attr) + "=\"";
  const std::size_t p = line.find(key);
  if (p == std::string::npos) return "";
  const std::size_t s = p + key.size();
  const std::size_t e = line.find('"', s);
  if (e == std::string::npos || e <= s) return "";
  return line.substr(s, e - s);
}

int DaySlotFromPeriod(const std::string& periodIndex) {
  if (periodIndex == "1") return 0;
  if (periodIndex == "2") return 1;
  if (periodIndex == "3") return 2;
  return -1;
}

}  // namespace

ForecastLocationParser::ForecastLocationParser(const std::string& locationName) : locationName_(locationName) {}

void ForecastLocationParser::FeedLine(const std::string& rawLine) {
  if (done_) return;
  const std::string line = TrimCopy(rawLine);
  if (line.empty()) return;

  if (!inTargetArea_) {
    if (line.rfind("<area ", 0) == 0 &&
        line.find(std::string("description=\"") + locationName_ + "\"") != std::string::npos &&
        line.find("type=\"location\"") != std::string::npos) {
      inTargetArea_ = true;
      foundArea_ = true;
    }
    return;
  }

  if (line.rfind("</area", 0) == 0) {
    done_ = true;
    return;
  }

  if (line.rfind("<forecast-period ", 0) == 0) {
    periodIndex_ = ExtractAttr(line, "index");
    periodStartLocal_ = ExtractAttr(line, "start-time-local");
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) {
      out_.days[slot].label = DayLabelFromIso(periodStartLocal_);
    }
    return;
  }

  if (line.find("type=\"forecast_icon_code\"") != std::string::npos) {
    const std::string v = ExtractXmlValue(line);
    if (periodIndex_ == "0") out_.currentIconCode = v;
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) out_.days[slot].iconCode = v;
    return;
  }

  if (line.find("type=\"air_temperature_minimum\"") != std::string::npos) {
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) out_.days[slot].minC = ExtractXmlValue(line);
    return;
  }

  if (line.find("type=\"air_temperature_maximum\"") != std::string::npos) {
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) out_.days[slot].maxC = ExtractXmlValue(line);
    return;
  }

  if (line.find("type=\"precipitation_range\"") != std::string::npos) {
    const std::string v = ExtractXmlValue(line);
    if (periodIndex_ == "1") out_.rainTodayRange = v;
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) out_.days[slot].rainRange = v;
    return;
  }

  if (line.find("type=\"probability_of_precipitation\"") != std::string::npos) {
    const std::string v = ExtractXmlValue(line);
    if (periodIndex_ == "0") fallbackProb0_ = v;
    if (periodIndex_ == "1") out_.rainTodayChance = v;
    const int slot = DaySlotFromPeriod(periodIndex_);
    if (slot >= 0) out_.days[slot].rainChance = v;
    return;
  }
}

bool ForecastLocationParser::IsDone() const {
  return done_;
}

bool ForecastLocationParser::Finalize(std::string& error) {
  if (!foundArea_) {
    error = "Forecast location not found";
    return false;
  }
  if (out_.rainTodayRange.empty()) {
    if (!fallbackProb0_.empty()) {
      out_.rainTodayRange = "~" + fallbackProb0_ + " chance";
    } else {
      error = "No forecast rain range";
      return false;
    }
  }
  return true;
}

const ForecastParseResult& ForecastLocationParser::Data() const {
  return out_;
}
