#include "GamesActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <array>

#include "activities/ActivityManager.h"
#include "activities/apps/games/CaroActivity.h"
#include "activities/apps/games/MinesweeperActivity.h"
#include "activities/apps/games/SudokuActivity.h"
#include "activities/apps/games/TwentyFortyEightActivity.h"
#include "activities/apps/games/ThaiDraughtsActivity.h"
#include "components/UITheme.h"

namespace {
constexpr int COUNT = 5;
const std::array<StrId, COUNT> NAMES = {StrId::STR_APP_GAME_2048, StrId::STR_APP_GAME_SUDOKU,
                                        StrId::STR_APP_GAME_MINES, StrId::STR_APP_GAME_CARO,
                                        StrId::STR_APP_GAME_THAI_DRAUGHTS};
}  // namespace

void GamesActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void GamesActivity::loop() {
  navigator.onNext([this] {
    selected = ButtonNavigator::nextIndex(selected, COUNT);
    requestUpdate();
  });
  navigator.onPrevious([this] {
    selected = ButtonNavigator::previousIndex(selected, COUNT);
    requestUpdate();
  });
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (!mappedInput.wasReleased(MappedInputManager::Button::Confirm)) return;
  if (selected == 0) activityManager.pushActivity(std::make_unique<TwentyFortyEightActivity>(renderer, mappedInput));
  if (selected == 1) activityManager.pushActivity(std::make_unique<SudokuActivity>(renderer, mappedInput));
  if (selected == 2) activityManager.pushActivity(std::make_unique<MinesweeperActivity>(renderer, mappedInput));
  if (selected == 3) activityManager.pushActivity(std::make_unique<CaroActivity>(renderer, mappedInput));
  if (selected == 4) activityManager.pushActivity(std::make_unique<ThaiDraughtsActivity>(renderer, mappedInput));
}

void GamesActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAMES));
  GUI.drawButtonMenu(renderer,
                     Rect{0, metrics.topPadding + metrics.headerHeight, width,
                          height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight},
                     COUNT, selected, [](int index) { return std::string(I18N.get(NAMES[static_cast<size_t>(index)])); },
                     [](int) { return UIIcon::None; });
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
