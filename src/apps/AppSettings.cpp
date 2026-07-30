#include "AppSettings.h"

namespace {
uint8_t bounded(JsonVariantConst value, uint8_t fallback, uint8_t minimum, uint8_t maximum) {
  const int candidate = value | static_cast<int>(fallback);
  return candidate >= minimum && candidate <= maximum ? static_cast<uint8_t>(candidate) : fallback;
}
}  // namespace

void AppSettings::toJson(JsonDocument& doc) const {
  doc["weatherLocation"] = weatherLocation;
  doc["temperatureUnit"] = temperatureUnit;
  doc["pomodoroFocusMinutes"] = pomodoroFocusMinutes;
  doc["pomodoroShortBreakMinutes"] = pomodoroShortBreakMinutes;
  doc["pomodoroLongBreakMinutes"] = pomodoroLongBreakMinutes;
}

bool AppSettings::fromJson(JsonVariantConst doc) {
  weatherLocation = bounded(doc["weatherLocation"], weatherLocation, 0, 7);
  temperatureUnit = bounded(doc["temperatureUnit"], temperatureUnit, 0, 1);
  pomodoroFocusMinutes = bounded(doc["pomodoroFocusMinutes"], pomodoroFocusMinutes, 5, 60);
  pomodoroShortBreakMinutes = bounded(doc["pomodoroShortBreakMinutes"], pomodoroShortBreakMinutes, 1, 30);
  pomodoroLongBreakMinutes = bounded(doc["pomodoroLongBreakMinutes"], pomodoroLongBreakMinutes, 5, 60);
  return true;
}
