#pragma once

#include <string>

struct ParsedWeatherData {
  std::string stationName;
  std::string observedTimeLocal;
  std::string windDir;
  std::string airTempC;
  std::string apparentTempC;
  std::string relHumidityPct;
  std::string windKmh;
  std::string rainfallMm;
  std::string dayMinTempC;
  std::string dayMaxTempC;
  bool valid = false;
};

class BomStationParser {
 public:
  explicit BomStationParser(const std::string& stationId);

  void FeedLine(const std::string& line);
  bool IsDone() const;
  bool Finalize(std::string& error);
  const ParsedWeatherData& Data() const;

 private:
  std::string stationId_;
  ParsedWeatherData data_;
  bool inTargetStation_ = false;
  bool foundStation_ = false;
  bool done_ = false;
};
