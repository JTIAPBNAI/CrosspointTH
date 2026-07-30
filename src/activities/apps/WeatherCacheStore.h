#pragma once

#include <cstdint>

struct WeatherCacheData {
  uint32_t updatedAt = 0;
  int16_t temperatureTenths = 0;
  int16_t apparentTenths = 0;
  uint8_t humidity = 0;
  uint8_t weatherCode = 0;
  uint8_t location = 0;
};

bool loadWeatherCache(uint8_t location, WeatherCacheData& data);
bool saveWeatherCache(const WeatherCacheData& data);
