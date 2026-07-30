#pragma once

#include <ArduinoJson.h>
#include <PersistableStore.h>

#include <cstdint>

class AppSettings final : public PersistableStore<AppSettings> {
 private:
  AppSettings() = default;
  friend class PersistableStore<AppSettings>;

 public:
  uint8_t weatherLocation = 0;
  uint8_t temperatureUnit = 0;
  uint8_t pomodoroFocusMinutes = 25;
  uint8_t pomodoroShortBreakMinutes = 5;
  uint8_t pomodoroLongBreakMinutes = 15;

  static const char* getFilePath() { return "/.crosspoint/apps.json"; }
  void toJson(JsonDocument& doc) const;
  bool fromJson(JsonVariantConst doc);
};

#define APP_SETTINGS AppSettings::getInstance()
