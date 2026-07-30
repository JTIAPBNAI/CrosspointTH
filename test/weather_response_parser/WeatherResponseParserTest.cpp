#include <gtest/gtest.h>

#include "WeatherResponseParser.h"

TEST(WeatherResponseParser, IgnoresCurrentUnitsStringsAndReadsCurrentNumbers) {
  const std::string response = R"({
    "current_units":{"temperature_2m":"°C","relative_humidity_2m":"%",
      "apparent_temperature":"°C","weather_code":"wmo code"},
    "current":{"time":"2026-07-30T09:00","temperature_2m":31.4,
      "relative_humidity_2m":72,"apparent_temperature":37.8,"weather_code":3}
  })";
  WeatherReading reading;
  ASSERT_TRUE(parseOpenMeteoCurrent(response, reading));
  EXPECT_EQ(reading.temperatureTenths, 314);
  EXPECT_EQ(reading.apparentTenths, 378);
  EXPECT_EQ(reading.humidity, 72);
  EXPECT_EQ(reading.weatherCode, 3);
}

TEST(WeatherResponseParser, RejectsUnitsOnlyResponse) {
  WeatherReading reading;
  EXPECT_FALSE(parseOpenMeteoCurrent(
      R"({"current_units":{"temperature_2m":"°C","relative_humidity_2m":"%"}})", reading));
}
