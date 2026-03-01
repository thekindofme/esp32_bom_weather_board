#include <unity.h>

#include <fstream>
#include <string>
#include <vector>

#include "BomParser.h"
#include "FtpUtils.h"

void test_parse_pasv_response_success() {
  std::string ip;
  uint16_t port = 0;
  const bool ok = ParsePasvResponse("227 Entering Passive Mode (192,168,1,15,195,80)", ip, port);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING("192.168.1.15", ip.c_str());
  TEST_ASSERT_EQUAL_UINT16(50000, port);
}

void test_parse_pasv_response_invalid() {
  std::string ip;
  uint16_t port = 0;
  const bool ok = ParsePasvResponse("227 Entering Passive Mode (192,168,1,15,300,80)", ip, port);
  TEST_ASSERT_FALSE(ok);
}

void test_parse_station_success() {
  const std::vector<std::string> lines = {
      "<observations>",
      "<station bom-id=\"086111\" description=\"Not Target\">",
      "<period time-local=\"2026-03-01T20:10:00+11:00\">",
      "<level><element type=\"air_temperature\">99.9</element></level>",
      "</station>",
      "<station bom-id=\"086282\" description=\"Melbourne Airport\">",
      "<period index=\"0\" time-local=\"2026-03-01T20:10:00+11:00\">",
      "<level index=\"0\" type=\"surface\">",
      "<element units=\"Celsius\" type=\"apparent_temp\">20.2</element>",
      "<element units=\"Celsius\" type=\"air_temperature\">20.9</element>",
      "<element units=\"%\" type=\"rel-humidity\">87</element>",
      "<element type=\"wind_dir\">NNE</element>",
      "<element units=\"km/h\" type=\"wind_spd_kmh\">20</element>",
      "<element units=\"mm\" type=\"rainfall\">5.0</element>",
      "</level>",
      "</period>",
      "</station>",
      "</observations>",
  };

  BomStationParser parser("086282");
  for (const auto& l : lines) {
    parser.FeedLine(l);
  }

  std::string error;
  TEST_ASSERT_TRUE(parser.Finalize(error));
  const ParsedWeatherData& out = parser.Data();
  TEST_ASSERT_TRUE(out.valid);
  TEST_ASSERT_EQUAL_STRING("Melbourne Airport", out.stationName.c_str());
  TEST_ASSERT_EQUAL_STRING("2026-03-01T20:10:00+11:00", out.observedTimeLocal.c_str());
  TEST_ASSERT_EQUAL_STRING("20.9", out.airTempC.c_str());
  TEST_ASSERT_EQUAL_STRING("20.2", out.apparentTempC.c_str());
  TEST_ASSERT_EQUAL_STRING("87", out.relHumidityPct.c_str());
  TEST_ASSERT_EQUAL_STRING("NNE", out.windDir.c_str());
  TEST_ASSERT_EQUAL_STRING("20", out.windKmh.c_str());
  TEST_ASSERT_EQUAL_STRING("5.0", out.rainfallMm.c_str());
}

void test_parse_station_not_found() {
  BomStationParser parser("000000");
  parser.FeedLine("<station bom-id=\"086282\" description=\"Melbourne Airport\">");
  parser.FeedLine("<element type=\"air_temperature\">20.9</element>");
  parser.FeedLine("</station>");

  std::string error;
  TEST_ASSERT_FALSE(parser.Finalize(error));
  TEST_ASSERT_EQUAL_STRING("Station not found in XML", error.c_str());
}

void test_parse_station_missing_temp() {
  BomStationParser parser("086282");
  parser.FeedLine("<station bom-id=\"086282\" description=\"Melbourne Airport\">");
  parser.FeedLine("<element type=\"wind_dir\">NNE</element>");
  parser.FeedLine("</station>");

  std::string error;
  TEST_ASSERT_FALSE(parser.Finalize(error));
  TEST_ASSERT_EQUAL_STRING("Missing temp data", error.c_str());
}

void test_parse_real_bom_fixture_melbourne_airport() {
  std::ifstream fixture("test/fixtures/IDV60920_live_2026-03-01.xml");
  TEST_ASSERT_TRUE_MESSAGE(fixture.is_open(), "Fixture file not found");

  BomStationParser parser("086282");
  std::string line;
  while (std::getline(fixture, line)) {
    parser.FeedLine(line);
    if (parser.IsDone()) {
      break;
    }
  }

  std::string error;
  TEST_ASSERT_TRUE_MESSAGE(parser.Finalize(error), error.c_str());

  const ParsedWeatherData& out = parser.Data();
  TEST_ASSERT_TRUE(out.valid);
  TEST_ASSERT_EQUAL_STRING("Melbourne Airport", out.stationName.c_str());
  TEST_ASSERT_EQUAL_STRING("2026-03-01T20:20:00+11:00", out.observedTimeLocal.c_str());
  TEST_ASSERT_EQUAL_STRING("20.9", out.airTempC.c_str());
  TEST_ASSERT_EQUAL_STRING("19.7", out.apparentTempC.c_str());
  TEST_ASSERT_EQUAL_STRING("86", out.relHumidityPct.c_str());
  TEST_ASSERT_EQUAL_STRING("NNE", out.windDir.c_str());
  TEST_ASSERT_EQUAL_STRING("22", out.windKmh.c_str());
  TEST_ASSERT_EQUAL_STRING("5.0", out.rainfallMm.c_str());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_pasv_response_success);
  RUN_TEST(test_parse_pasv_response_invalid);
  RUN_TEST(test_parse_station_success);
  RUN_TEST(test_parse_station_not_found);
  RUN_TEST(test_parse_station_missing_temp);
  RUN_TEST(test_parse_real_bom_fixture_melbourne_airport);
  return UNITY_END();
}
