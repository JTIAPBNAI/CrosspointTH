#include "WeatherCacheStore.h"

#include <HalStorage.h>

namespace {
constexpr char CACHE_PATH[] = "/.crosspoint/weather-v1.bin";
constexpr uint32_t CACHE_MAGIC = 0x31574854;  // THW1

struct StoredWeatherCache {
  uint32_t magic;
  uint32_t updatedAt;
  int16_t temperatureTenths;
  int16_t apparentTenths;
  uint8_t humidity;
  uint8_t weatherCode;
  uint8_t location;
  // cppcheck-suppress unusedStructMember
  uint8_t reserved;
};
}  // namespace

bool loadWeatherCache(uint8_t location, WeatherCacheData& data) {
  StoredWeatherCache stored{};
  HalFile file;
  if (!Storage.openFileForRead("WEATHER", CACHE_PATH, file)) return false;
  const int read = file.read(&stored, sizeof(stored));
  file.close();
  if (read != static_cast<int>(sizeof(stored)) || stored.magic != CACHE_MAGIC || stored.location != location) {
    return false;
  }
  data = {stored.updatedAt, stored.temperatureTenths, stored.apparentTenths, stored.humidity,
          stored.weatherCode, stored.location};
  return true;
}

bool saveWeatherCache(const WeatherCacheData& data) {
  const StoredWeatherCache stored{CACHE_MAGIC, data.updatedAt, data.temperatureTenths, data.apparentTenths,
                                  data.humidity, data.weatherCode, data.location, 0};
  HalFile file;
  if (!Storage.openFileForWrite("WEATHER", CACHE_PATH, file)) return false;
  const bool ok = file.write(&stored, sizeof(stored)) == sizeof(stored);
  file.flush();
  file.close();
  return ok;
}
