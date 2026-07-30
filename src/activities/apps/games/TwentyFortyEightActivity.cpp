#include "TwentyFortyEightActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <esp_random.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>

#include "components/UITheme.h"
#include "fontIds.h"

void TwentyFortyEightActivity::reset() {
  memset(cells, 0, sizeof(cells));
  score = 0;
  gameOver = false;
  addTile();
  addTile();
}

void TwentyFortyEightActivity::addTile() {
  std::array<uint8_t, 16> empty{};
  uint8_t count = 0;
  for (uint8_t r = 0; r < 4; ++r)
    for (uint8_t c = 0; c < 4; ++c)
      if (cells[r][c] == 0) empty[count++] = r * 4 + c;
  if (count == 0) return;
  const uint8_t slot = empty[esp_random() % count];
  cells[slot / 4][slot % 4] = esp_random() % 10 == 0 ? 4 : 2;
}

bool TwentyFortyEightActivity::move(int dr, int dc) {
  bool changed = false;
  for (int outer = 0; outer < 4; ++outer) {
    uint16_t line[4] = {};
    int used = 0;
    for (int inner = 0; inner < 4; ++inner) {
      const int r = dr == 0 ? outer : (dr > 0 ? 3 - inner : inner);
      const int c = dc == 0 ? outer : (dc > 0 ? 3 - inner : inner);
      if (cells[r][c]) line[used++] = cells[r][c];
    }
    uint16_t merged[4] = {};
    int output = 0;
    for (int i = 0; i < used; ++i) {
      if (i + 1 < used && line[i] == line[i + 1]) {
        merged[output] = line[i] * 2;
        score += merged[output++];
        ++i;
      } else {
        merged[output++] = line[i];
      }
    }
    for (int inner = 0; inner < 4; ++inner) {
      const int r = dr == 0 ? outer : (dr > 0 ? 3 - inner : inner);
      const int c = dc == 0 ? outer : (dc > 0 ? 3 - inner : inner);
      if (cells[r][c] != merged[inner]) changed = true;
      cells[r][c] = merged[inner];
    }
  }
  if (changed) addTile();
  gameOver = !canMove();
  return changed;
}

bool TwentyFortyEightActivity::canMove() const {
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (cells[r][c] == 0) return true;
      if (r + 1 < 4 && cells[r][c] == cells[r + 1][c]) return true;
      if (c + 1 < 4 && cells[r][c] == cells[r][c + 1]) return true;
    }
  }
  return false;
}

void TwentyFortyEightActivity::onEnter() {
  Activity::onEnter();
  reset();
  requestUpdate();
}

void TwentyFortyEightActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (gameOver && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    reset();
    requestUpdate();
    return;
  }
  bool moved = false;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) moved = move(0, -1);
  else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) moved = move(0, 1);
  else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) moved = move(-1, 0);
  else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) moved = move(1, 0);
  if (moved || gameOver) requestUpdate();
}

void TwentyFortyEightActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAME_2048));
  char scoreText[48];
  snprintf(scoreText, sizeof(scoreText), "%s: %lu", tr(STR_APP_SCORE), static_cast<unsigned long>(score));
  renderer.drawCenteredText(SMALL_FONT_ID, metrics.topPadding + metrics.headerHeight + 8, scoreText);

  const int availableH = height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight - 50;
  const int boardSize = std::min(width - 24, availableH);
  const int cell = boardSize / 4;
  const int left = (width - cell * 4) / 2;
  const int top = metrics.topPadding + metrics.headerHeight + 42;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      const int x = left + c * cell;
      const int y = top + r * cell;
      renderer.drawRect(x + 2, y + 2, cell - 4, cell - 4, cells[r][c] >= 128 ? 3 : 1, true);
      if (cells[r][c]) {
        char value[8];
        snprintf(value, sizeof(value), "%u", cells[r][c]);
        const int font = cells[r][c] >= 1024 ? SMALL_FONT_ID : UI_10_FONT_ID;
        renderer.drawText(font, x + (cell - renderer.getTextWidth(font, value)) / 2,
                          y + (cell - renderer.getLineHeight(font)) / 2, value, true, EpdFontFamily::BOLD);
      }
    }
  }
  if (gameOver) renderer.drawCenteredText(UI_10_FONT_ID, top + boardSize / 2, tr(STR_APP_GAME_OVER), true,
                                           EpdFontFamily::BOLD);
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), gameOver ? tr(STR_APP_NEW_GAME) : "", tr(STR_DIR_UP),
                                            tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
