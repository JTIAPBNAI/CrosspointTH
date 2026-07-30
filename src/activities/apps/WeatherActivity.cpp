#include "WeatherActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <WiFi.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

#include "apps/AppSettings.h"
#include "activities/network/WifiSelectionActivity.h"
#include "activities/apps/WeatherResponseParser.h"
#include "activities/apps/WeatherCacheStore.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

namespace {
struct Location {
  const char* english;
  const char* thai;
  const char* latitude;
  const char* longitude;
};

constexpr std::array<Location, 8> LOCATIONS = {{{"Bangkok", "กรุงเทพฯ", "13.7563", "100.5018"},
                                                {"Chiang Mai", "เชียงใหม่", "18.7883", "98.9853"},
                                                {"Khon Kaen", "ขอนแก่น", "16.4322", "102.8236"},
                                                {"Ubon Ratchathani", "อุบลราชธานี", "15.2447", "104.8473"},
                                                {"Nakhon Ratchasima", "นครราชสีมา", "14.9799", "102.0978"},
                                                {"Chon Buri", "ชลบุรี", "13.3611", "100.9847"},
                                                {"Phuket", "ภูเก็ต", "7.8804", "98.3923"},
                                                {"Hat Yai", "หาดใหญ่", "7.0084", "100.4747"}}};

const char* condition(uint8_t code) {
  if (code == 0) return tr(STR_APP_WEATHER_CLEAR);
  if (code <= 3) return tr(STR_APP_WEATHER_CLOUDY);
  if (code == 45 || code == 48) return tr(STR_APP_WEATHER_FOG);
  if (code >= 51 && code <= 67) return tr(STR_APP_WEATHER_RAIN);
  if (code >= 80 && code <= 82) return tr(STR_APP_WEATHER_SHOWERS);
  if (code >= 95) return tr(STR_APP_WEATHER_STORM);
  return tr(STR_APP_WEATHER_CLOUDY);
}
}  // namespace

void WeatherActivity::onEnter() {
  Activity::onEnter();
  APP_SETTINGS.loadFromFile();
  loadCache();
  requestUpdate();
}

void WeatherActivity::onExit() {
  Activity::onExit();
  if (startedWifi) {
    WiFi.disconnect(false);
    WiFi.mode(WIFI_OFF);
  }
}

void WeatherActivity::loadCache() {
  hasData = false;
  WeatherCacheData value;
  if (!loadWeatherCache(APP_SETTINGS.weatherLocation, value)) return;
  updatedAt = value.updatedAt;
  temperatureTenths = value.temperatureTenths;
  apparentTenths = value.apparentTenths;
  humidity = value.humidity;
  weatherCode = value.weatherCode;
  hasData = true;
  cached = true;
}

void WeatherActivity::saveCache() const {
  saveWeatherCache({updatedAt, temperatureTenths, apparentTenths, humidity, weatherCode,
                    APP_SETTINGS.weatherLocation});
}

void WeatherActivity::beginRefresh() {
  if (WiFi.status() == WL_CONNECTED) {
    state = State::FetchPending;
    requestUpdate();
    return;
  }
  startedWifi = true;
  state = State::Connecting;
  startActivityForResult(std::make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this](const ActivityResult& result) {
                           state = result.isCancelled ? State::Ready : State::FetchPending;
                           requestUpdate();
                         });
}

bool WeatherActivity::parseResponse(const std::string& response) {
  WeatherReading reading;
  if (!parseOpenMeteoCurrent(response, reading)) return false;
  temperatureTenths = reading.temperatureTenths;
  apparentTenths = reading.apparentTenths;
  humidity = reading.humidity;
  weatherCode = reading.weatherCode;
  updatedAt = static_cast<uint32_t>(time(nullptr));
  return true;
}

void WeatherActivity::fetchWeather() {
  state = State::Loading;
  requestUpdateAndWait();
  const auto& location = LOCATIONS[APP_SETTINGS.weatherLocation];
  char url[320];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&"
           "current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code&timezone=Asia%%2FBangkok",
           location.latitude, location.longitude);
  std::string response;
  response.reserve(2048);
  if (!HttpDownloader::fetchUrl(url, response) || response.size() > 8192 || !parseResponse(response)) {
    LOG_ERR("WEATHER", "Weather request or parse failed");
    state = State::Error;
    requestUpdate();
    return;
  }
  hasData = true;
  cached = false;
  state = State::Ready;
  saveCache();
  requestUpdate();
}

void WeatherActivity::loop() {
  if (state == State::FetchPending) {
    fetchWeather();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (state == State::Connecting || state == State::Loading) return;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left) ||
      mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    const int delta = mappedInput.wasReleased(MappedInputManager::Button::Right) ? 1 : -1;
    APP_SETTINGS.weatherLocation = static_cast<uint8_t>(
        (APP_SETTINGS.weatherLocation + static_cast<int>(LOCATIONS.size()) + delta) % LOCATIONS.size());
    APP_SETTINGS.saveToFile();
    loadCache();
    state = State::Ready;
    requestUpdate();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) beginRefresh();
}

void WeatherActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_WEATHER));

  const bool thai = I18N.getLanguage() == Language::TH;
  const auto& location = LOCATIONS[APP_SETTINGS.weatherLocation];
  renderer.drawCenteredText(UI_10_FONT_ID, metrics.topPadding + metrics.headerHeight + 24,
                            thai ? location.thai : location.english, true, EpdFontFamily::BOLD);
  if (WiFi.status() == WL_CONNECTED) {
    const std::string wifiStatus = std::string("Wi-Fi: ") + tr(STR_CONNECTED);
    renderer.drawCenteredText(SMALL_FONT_ID, metrics.topPadding + metrics.headerHeight + 60, wifiStatus.c_str());
  }
  if (state == State::Connecting || state == State::Loading) {
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2, tr(STR_APP_WEATHER_LOADING));
  } else if (state == State::Error && !hasData) {
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2, tr(STR_APP_WEATHER_ERROR));
  } else if (hasData) {
    const float multiplier = APP_SETTINGS.temperatureUnit == 0 ? 1.0f : 1.8f;
    const float addition = APP_SETTINGS.temperatureUnit == 0 ? 0.0f : 32.0f;
    const float temp = temperatureTenths / 10.0f * multiplier + addition;
    const float feels = apparentTenths / 10.0f * multiplier + addition;
    char temperature[24];
    snprintf(temperature, sizeof(temperature), "%.1f °%c", temp, APP_SETTINGS.temperatureUnit == 0 ? 'C' : 'F');
    renderer.drawCenteredText(UI_12_FONT_ID, height / 2 - 95, temperature, true, EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2 - 20, condition(weatherCode));
    char detail[96];
    snprintf(detail, sizeof(detail), "%s %.1f °%c   %s %u%%", tr(STR_APP_FEELS_LIKE), feels,
             APP_SETTINGS.temperatureUnit == 0 ? 'C' : 'F', tr(STR_APP_HUMIDITY), humidity);
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 45, detail);
    if (cached) renderer.drawCenteredText(SMALL_FONT_ID, height / 2 + 95, tr(STR_APP_WEATHER_CACHED));
  } else {
    UITheme::drawCenteredWrappedText(renderer, Rect{30, height / 2 - 50, width - 60, 120}, UI_10_FONT_ID,
                                     tr(STR_APP_WEATHER_PRESS_REFRESH), 3);
  }
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_APP_REFRESH), tr(STR_DIR_LEFT), tr(STR_DIR_RIGHT));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
