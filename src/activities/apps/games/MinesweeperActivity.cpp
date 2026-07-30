#include "MinesweeperActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <esp_random.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>

#include "components/UITheme.h"
#include "fontIds.h"

void MinesweeperActivity::reset() {
  memset(values, 0, sizeof(values));
  memset(state, 0, sizeof(state));
  cursorRow = cursorCol = 0;
  gameOver = won = longPressHandled = false;
  int placed = 0;
  while (placed < 16) {
    const int slot = esp_random() % (ROWS * COLS);
    const int r = slot / COLS;
    const int c = slot % COLS;
    if (values[r][c] == 9) continue;
    values[r][c] = 9;
    ++placed;
  }
  for (int r = 0; r < ROWS; ++r) {
    for (int c = 0; c < COLS; ++c) {
      if (values[r][c] == 9) continue;
      uint8_t count = 0;
      for (int dr = -1; dr <= 1; ++dr)
        for (int dc = -1; dc <= 1; ++dc) {
          const int nr = r + dr, nc = c + dc;
          if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && values[nr][nc] == 9) ++count;
        }
      values[r][c] = count;
    }
  }
}

void MinesweeperActivity::reveal(uint8_t row, uint8_t col) {
  if (state[row][col] != 0) return;
  if (values[row][col] == 9) {
    state[row][col] = 1;
    gameOver = true;
    won = false;
    return;
  }
  std::array<uint16_t, ROWS * COLS> queue{};
  size_t head = 0, tail = 0;
  queue[tail++] = row * COLS + col;
  while (head < tail) {
    const uint16_t slot = queue[head++];
    const int r = slot / COLS, c = slot % COLS;
    if (state[r][c] != 0) continue;
    state[r][c] = 1;
    if (values[r][c] != 0) continue;
    for (int dr = -1; dr <= 1; ++dr)
      for (int dc = -1; dc <= 1; ++dc) {
        const int nr = r + dr, nc = c + dc;
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && state[nr][nc] == 0 && tail < queue.size())
          queue[tail++] = nr * COLS + nc;
      }
  }
  checkWin();
}

void MinesweeperActivity::checkWin() {
  for (int r = 0; r < ROWS; ++r)
    for (int c = 0; c < COLS; ++c)
      if (values[r][c] != 9 && state[r][c] != 1) return;
  gameOver = won = true;
}

void MinesweeperActivity::onEnter() {
  Activity::onEnter();
  reset();
  requestUpdate();
}

void MinesweeperActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (gameOver && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    reset();
    requestUpdate();
    return;
  }
  if (gameOver) return;
  bool changed = false;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) { cursorCol = (cursorCol + COLS - 1) % COLS; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Right)) { cursorCol = (cursorCol + 1) % COLS; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) { cursorRow = (cursorRow + ROWS - 1) % ROWS; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) { cursorRow = (cursorRow + 1) % ROWS; changed = true; }
  if (mappedInput.isPressed(MappedInputManager::Button::Confirm) && mappedInput.getHeldTime() >= 700 &&
      !longPressHandled) {
    longPressHandled = true;
    if (state[cursorRow][cursorCol] != 1) state[cursorRow][cursorCol] = state[cursorRow][cursorCol] == 2 ? 0 : 2;
    changed = true;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (!longPressHandled) reveal(cursorRow, cursorCol);
    longPressHandled = false;
    changed = true;
  }
  if (changed) requestUpdate();
}

void MinesweeperActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAME_MINES));
  const int availableH = height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight - 12;
  const int cell = std::min((width - 18) / COLS, availableH / ROWS);
  const int left = (width - cell * COLS) / 2;
  const int top = metrics.topPadding + metrics.headerHeight + 6;
  for (int r = 0; r < ROWS; ++r) {
    for (int c = 0; c < COLS; ++c) {
      const int x = left + c * cell, y = top + r * cell;
      renderer.drawRect(x, y, cell, cell, r == cursorRow && c == cursorCol ? 2 : 1, true);
      char text[3] = {};
      if (state[r][c] == 2) strcpy(text, "F");
      else if (state[r][c] == 1 && values[r][c] == 9) strcpy(text, "*");
      else if (state[r][c] == 1 && values[r][c] > 0) snprintf(text, sizeof(text), "%u", values[r][c]);
      if (text[0]) renderer.drawText(SMALL_FONT_ID, x + (cell - renderer.getTextWidth(SMALL_FONT_ID, text)) / 2,
                                     y + (cell - renderer.getLineHeight(SMALL_FONT_ID)) / 2, text, true,
                                     EpdFontFamily::BOLD);
    }
  }
  if (gameOver) renderer.drawCenteredText(UI_10_FONT_ID, top + cell * ROWS / 2,
                                           won ? tr(STR_APP_YOU_WIN) : tr(STR_APP_GAME_OVER), true,
                                           EpdFontFamily::BOLD);
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), gameOver ? tr(STR_APP_NEW_GAME) : tr(STR_APP_REVEAL),
                                            tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
