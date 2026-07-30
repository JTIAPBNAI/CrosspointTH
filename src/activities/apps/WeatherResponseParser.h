#pragma once

#include <cstdint>
#include <string>

struct WeatherReading {
  int16_t temperatureTenths = 0;
  int16_t apparentTenths = 0;
  uint8_t humidity = 0;
  uint8_t weatherCode = 0;
};

// Parses numeric values only from Open-Meteo's `current` object. The response
// also contains the same keys under `current_units` with string values.
bool parseOpenMeteoCurrent(const std::string& json, WeatherReading& reading);
