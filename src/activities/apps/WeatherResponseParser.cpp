#include "WeatherResponseParser.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
bool findCurrentObject(const std::string& json, size_t& begin, size_t& end) {
  const size_t key = json.find("\"current\"");
  if (key == std::string::npos) return false;
  begin = json.find('{', key + 9);
  if (begin == std::string::npos) return false;
  end = json.find('}', begin + 1);
  return end != std::string::npos;
}

bool findNumber(const std::string& json, size_t begin, size_t end, const char* key, double& result) {
  const std::string needle = std::string("\"") + key + "\"";
  const size_t keyPos = json.find(needle, begin);
  if (keyPos == std::string::npos || keyPos >= end) return false;
  const size_t colon = json.find(':', keyPos + needle.size());
  if (colon == std::string::npos || colon >= end) return false;
  const char* start = json.c_str() + colon + 1;
  char* parsedEnd = nullptr;
  result = strtod(start, &parsedEnd);
  return parsedEnd != start && static_cast<size_t>(parsedEnd - json.c_str()) <= end;
}
}  // namespace

bool parseOpenMeteoCurrent(const std::string& json, WeatherReading& reading) {
  size_t begin = 0;
  size_t end = 0;
  if (!findCurrentObject(json, begin, end)) return false;

  double temperature = 0;
  double apparent = 0;
  double humidity = 0;
  double code = 0;
  if (!findNumber(json, begin, end, "temperature_2m", temperature) ||
      !findNumber(json, begin, end, "apparent_temperature", apparent) ||
      !findNumber(json, begin, end, "relative_humidity_2m", humidity) ||
      !findNumber(json, begin, end, "weather_code", code)) {
    return false;
  }

  reading.temperatureTenths = static_cast<int16_t>(lround(temperature * 10.0));
  reading.apparentTenths = static_cast<int16_t>(lround(apparent * 10.0));
  reading.humidity = static_cast<uint8_t>(std::clamp(humidity, 0.0, 100.0));
  reading.weatherCode = static_cast<uint8_t>(std::clamp(code, 0.0, 255.0));
  return true;
}
