#include "SudokuActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr uint8_t BASE_PUZZLE[9][9] = {{5,3,0,0,7,0,0,0,0}, {6,0,0,1,9,5,0,0,0}, {0,9,8,0,0,0,0,6,0},
                                        {8,0,0,0,6,0,0,0,3}, {4,0,0,8,0,3,0,0,1}, {7,0,0,0,2,0,0,0,6},
                                        {0,6,0,0,0,0,2,8,0}, {0,0,0,4,1,9,0,0,5}, {0,0,0,0,8,0,0,7,9}};
constexpr uint8_t BASE_SOLUTION[9][9] = {{5,3,4,6,7,8,9,1,2}, {6,7,2,1,9,5,3,4,8}, {1,9,8,3,4,2,5,6,7},
                                          {8,5,9,7,6,1,4,2,3}, {4,2,6,8,5,3,7,9,1}, {7,1,3,9,2,4,8,5,6},
                                          {9,6,1,5,3,7,2,8,4}, {2,8,7,4,1,9,6,3,5}, {3,4,5,2,8,6,1,7,9}};
uint8_t shifted(uint8_t value, uint8_t shift) { return value == 0 ? 0 : static_cast<uint8_t>((value - 1 + shift) % 9 + 1); }
}  // namespace

void SudokuActivity::reset() {
  variation = static_cast<uint8_t>((variation + 1) % 9);
  for (int r = 0; r < 9; ++r) {
    for (int c = 0; c < 9; ++c) {
      puzzle[r][c] = shifted(BASE_PUZZLE[r][c], variation);
      solution[r][c] = shifted(BASE_SOLUTION[r][c], variation);
      fixed[r][c] = puzzle[r][c] != 0;
    }
  }
  cursorRow = cursorCol = 0;
  complete = false;
}

void SudokuActivity::updateComplete() { complete = memcmp(puzzle, solution, sizeof(puzzle)) == 0; }

void SudokuActivity::onEnter() {
  Activity::onEnter();
  variation = 8;
  reset();
  requestUpdate();
}

void SudokuActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (complete && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    reset();
    requestUpdate();
    return;
  }
  bool changed = false;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) { cursorCol = (cursorCol + 8) % 9; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Right)) { cursorCol = (cursorCol + 1) % 9; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) { cursorRow = (cursorRow + 8) % 9; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) { cursorRow = (cursorRow + 1) % 9; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) && !fixed[cursorRow][cursorCol]) {
    puzzle[cursorRow][cursorCol] = static_cast<uint8_t>((puzzle[cursorRow][cursorCol] + 1) % 10);
    updateComplete();
    changed = true;
  }
  if (changed) requestUpdate();
}

void SudokuActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAME_SUDOKU));
  const int availableH = height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight - 20;
  const int board = std::min(width - 20, availableH);
  const int cell = board / 9;
  const int left = (width - cell * 9) / 2;
  const int top = metrics.topPadding + metrics.headerHeight + 10;
  for (int i = 0; i <= 9; ++i) {
    const int thickness = i % 3 == 0 ? 3 : 1;
    renderer.drawLine(left, top + i * cell, left + 9 * cell, top + i * cell, thickness, true);
    renderer.drawLine(left + i * cell, top, left + i * cell, top + 9 * cell, thickness, true);
  }
  for (int r = 0; r < 9; ++r) {
    for (int c = 0; c < 9; ++c) {
      const bool cursor = r == cursorRow && c == cursorCol;
      if (cursor) renderer.drawRect(left + c * cell + 4, top + r * cell + 4, cell - 8, cell - 8, 2, true);
      if (!puzzle[r][c]) continue;
      char value[2] = {static_cast<char>('0' + puzzle[r][c]), '\0'};
      const int font = cell >= 45 ? UI_10_FONT_ID : SMALL_FONT_ID;
      renderer.drawText(font, left + c * cell + (cell - renderer.getTextWidth(font, value)) / 2,
                        top + r * cell + (cell - renderer.getLineHeight(font)) / 2, value, true,
                        fixed[r][c] ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR);
    }
  }
  if (complete) renderer.drawCenteredText(UI_10_FONT_ID, top + board / 2, tr(STR_APP_YOU_WIN), true,
                                           EpdFontFamily::BOLD);
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), complete ? tr(STR_APP_NEW_GAME) : tr(STR_APP_NUMBER),
                                            tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
