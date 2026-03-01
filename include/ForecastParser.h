#pragma once

#include <string>

struct ForecastDayData {
  std::string label;
  std::string iconCode;
  std::string minC;
  std::string maxC;
  std::string rainRange;
  std::string rainChance;
};

struct ForecastParseResult {
  std::string currentIconCode;
  std::string rainTodayRange;
  std::string rainTodayChance;
  ForecastDayData days[3];
};

class ForecastLocationParser {
 public:
  explicit ForecastLocationParser(const std::string& locationName);

  void FeedLine(const std::string& line);
  bool IsDone() const;
  bool Finalize(std::string& error);
  const ForecastParseResult& Data() const;

 private:
  std::string locationName_;
  ForecastParseResult out_;
  bool inTargetArea_ = false;
  bool foundArea_ = false;
  bool done_ = false;
  std::string periodIndex_;
  std::string periodStartLocal_;
  std::string fallbackProb0_;
};
