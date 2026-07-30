#pragma once

#include <cstdint>

#include "activities/Activity.h"

class WeatherActivity final : public Activity {
  enum class State : uint8_t { Ready, Connecting, FetchPending, Loading, Error };
  State state = State::Ready;
  bool hasData = false;
  bool cached = false;
  bool startedWifi = false;
  int16_t temperatureTenths = 0;
  int16_t apparentTenths = 0;
  uint8_t humidity = 0;
  uint8_t weatherCode = 0;
  uint32_t updatedAt = 0;

  void loadCache();
  void saveCache() const;
  void beginRefresh();
  void fetchWeather();
  bool parseResponse(const std::string& response);

 public:
  WeatherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Weather", renderer, mappedInput) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool preventAutoSleep() override { return state == State::Connecting || state == State::Loading; }
};
