#include "BomParser.h"

#include <algorithm>

namespace {

std::string TrimCopy(const std::string& s) {
  const auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
  if (first == s.end()) return "";
  const auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
  return std::string(first, last);
}

std::string ExtractAttribute(const std::string& line, const char* attr) {
  const std::string key = std::string(attr) + "=\"";
  const std::size_t keyPos = line.find(key);
  if (keyPos == std::string::npos) return "";
  const std::size_t valueStart = keyPos + key.size();
  const std::size_t valueEnd = line.find('"', valueStart);
  if (valueEnd == std::string::npos) return "";
  return line.substr(valueStart, valueEnd - valueStart);
}

std::string ExtractElementType(const std::string& line) {
  return ExtractAttribute(line, "type");
}

std::string ExtractElementValue(const std::string& line) {
  const std::size_t gt = line.find('>');
  if (gt == std::string::npos) return "";
  const std::size_t lt = line.find('<', gt + 1);
  if (lt == std::string::npos || lt <= gt + 1) return "";
  return TrimCopy(line.substr(gt + 1, lt - (gt + 1)));
}

}  // namespace

BomStationParser::BomStationParser(const std::string& stationId) : stationId_(stationId) {}

void BomStationParser::FeedLine(const std::string& rawLine) {
  if (done_) return;
  const std::string line = TrimCopy(rawLine);
  if (line.empty()) return;

  if (!inTargetStation_) {
    if (line.rfind("<station ", 0) == 0 &&
        line.find(std::string("bom-id=\"") + stationId_ + "\"") != std::string::npos) {
      inTargetStation_ = true;
      foundStation_ = true;
      data_.stationName = ExtractAttribute(line, "description");
    }
    return;
  }

  if (line.rfind("</station", 0) == 0) {
    done_ = true;
    return;
  }

  if (line.rfind("<period ", 0) == 0 && data_.observedTimeLocal.empty()) {
    data_.observedTimeLocal = ExtractAttribute(line, "time-local");
    return;
  }

  if (line.find("<element ") != std::string::npos) {
    const std::string type = ExtractElementType(line);
    const std::string value = ExtractElementValue(line);
    if (value.empty()) return;

    if (type == "air_temperature") data_.airTempC = value;
    else if (type == "apparent_temp") data_.apparentTempC = value;
    else if (type == "rel-humidity") data_.relHumidityPct = value;
    else if (type == "wind_spd_kmh") data_.windKmh = value;
    else if (type == "wind_dir") data_.windDir = value;
    else if (type == "rainfall") data_.rainfallMm = value;
    else if (type == "minimum_air_temperature") data_.dayMinTempC = value;
    else if (type == "maximum_air_temperature") data_.dayMaxTempC = value;
  }
}

bool BomStationParser::IsDone() const {
  return done_;
}

bool BomStationParser::Finalize(std::string& error) {
  if (!foundStation_) {
    error = "Station not found in XML";
    return false;
  }
  if (data_.airTempC.empty()) {
    error = "Missing temp data";
    return false;
  }
  data_.valid = true;
  return true;
}

const ParsedWeatherData& BomStationParser::Data() const {
  return data_;
}
