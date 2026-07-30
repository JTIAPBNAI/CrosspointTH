#include "AppsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <array>

#include "apps/AppSettings.h"
#include "activities/ActivityManager.h"
#include "activities/apps/ClockCalendarActivity.h"
#include "activities/apps/PomodoroActivity.h"
#include "activities/apps/WeatherActivity.h"
#include "activities/apps/flashcards/FlashcardsActivity.h"
#include "activities/apps/games/GamesActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int ITEM_COUNT = 5;
const std::array<StrId, ITEM_COUNT> ITEMS = {StrId::STR_APP_CLOCK_CALENDAR, StrId::STR_APP_WEATHER,
                                             StrId::STR_APP_POMODORO, StrId::STR_APP_FLASHCARDS,
                                             StrId::STR_APP_GAMES};
}  // namespace

void AppsActivity::onEnter() {
  Activity::onEnter();
  APP_SETTINGS.loadFromFile();
  requestUpdate();
}

void AppsActivity::loop() {
  navigator.onNext([this] {
    selected = ButtonNavigator::nextIndex(selected, ITEM_COUNT);
    requestUpdate();
  });
  navigator.onPrevious([this] {
    selected = ButtonNavigator::previousIndex(selected, ITEM_COUNT);
    requestUpdate();
  });

  int row = -1;
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int top = metrics.topPadding + metrics.headerHeight;
  const auto touch = mappedInput.rowTouch(row, top, metrics.menuRowHeight + metrics.menuSpacing, ITEM_COUNT, 0,
                                          renderer.getScreenWidth(), metrics.menuRowHeight);
  if (touch == MappedInputManager::RowTouch::Down && row != selected) {
    selected = row;
    requestUpdate();
    return;
  }
  const bool activate = touch == MappedInputManager::RowTouch::Tap ||
                        mappedInput.wasReleased(MappedInputManager::Button::Confirm);
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    activityManager.goHome(HomeMenuItem::APPS);
    return;
  }
  if (!activate) return;

  switch (selected) {
    case 0:
      activityManager.pushActivity(std::make_unique<ClockCalendarActivity>(renderer, mappedInput));
      break;
    case 1:
      activityManager.pushActivity(std::make_unique<WeatherActivity>(renderer, mappedInput));
      break;
    case 2:
      activityManager.pushActivity(std::make_unique<PomodoroActivity>(renderer, mappedInput));
      break;
    case 3:
      activityManager.pushActivity(std::make_unique<FlashcardsActivity>(renderer, mappedInput));
      break;
    case 4:
      activityManager.pushActivity(std::make_unique<GamesActivity>(renderer, mappedInput));
      break;
  }
}

void AppsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APPS));
  GUI.drawButtonMenu(renderer,
                     Rect{0, metrics.topPadding + metrics.headerHeight, width,
                          height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight},
                     ITEM_COUNT, selected,
                     [](int index) { return std::string(I18N.get(ITEMS[static_cast<size_t>(index)])); },
                     [](int) { return UIIcon::None; });
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
