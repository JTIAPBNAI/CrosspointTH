#include "CaroActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <esp_random.h>

#include <algorithm>
#include <array>
#include <cstring>

#include "components/UITheme.h"
#include "fontIds.h"

void CaroActivity::reset() {
  memset(cells, 0, sizeof(cells));
  cursorRow = cursorCol = SIZE / 2;
  winner = 0;
}

bool CaroActivity::wins(int row, int col, uint8_t player) const {
  constexpr int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};
  for (const auto& direction : directions) {
    int count = 1;
    for (int sign : {-1, 1}) {
      int r = row + direction[0] * sign;
      int c = col + direction[1] * sign;
      while (r >= 0 && r < SIZE && c >= 0 && c < SIZE && cells[r][c] == player) {
        ++count;
        r += direction[0] * sign;
        c += direction[1] * sign;
      }
    }
    if (count >= 5) return true;
  }
  return false;
}

void CaroActivity::aiMove(int humanRow, int humanCol) {
  auto tryWinning = [this](uint8_t player, int& outR, int& outC) {
    for (int r = 0; r < SIZE; ++r)
      for (int c = 0; c < SIZE; ++c) {
        if (cells[r][c]) continue;
        cells[r][c] = player;
        const bool result = wins(r, c, player);
        cells[r][c] = 0;
        if (result) { outR = r; outC = c; return true; }
      }
    return false;
  };
  int row = -1, col = -1;
  if (!tryWinning(2, row, col) && !tryWinning(1, row, col)) {
    std::array<uint16_t, SIZE * SIZE> nearby{};
    size_t count = 0;
    for (int radius = 1; radius <= 2 && count == 0; ++radius)
      for (int r = std::max(0, humanRow - radius); r <= std::min(SIZE - 1, humanRow + radius); ++r)
        for (int c = std::max(0, humanCol - radius); c <= std::min(SIZE - 1, humanCol + radius); ++c)
          if (!cells[r][c]) nearby[count++] = r * SIZE + c;
    if (count) { const auto slot = nearby[esp_random() % count]; row = slot / SIZE; col = slot % SIZE; }
  }
  if (row < 0) { winner = 3; return; }
  cells[row][col] = 2;
  if (wins(row, col, 2)) winner = 2;
}

void CaroActivity::onEnter() {
  Activity::onEnter();
  reset();
  requestUpdate();
}

void CaroActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) { finish(); return; }
  if (winner && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    reset(); requestUpdate(); return;
  }
  if (winner) return;
  bool changed = false;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) { cursorCol = (cursorCol + SIZE - 1) % SIZE; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Right)) { cursorCol = (cursorCol + 1) % SIZE; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) { cursorRow = (cursorRow + SIZE - 1) % SIZE; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) { cursorRow = (cursorRow + 1) % SIZE; changed = true; }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) && cells[cursorRow][cursorCol] == 0) {
    cells[cursorRow][cursorCol] = 1;
    if (wins(cursorRow, cursorCol, 1)) winner = 1;
    else aiMove(cursorRow, cursorCol);
    changed = true;
  }
  if (changed) requestUpdate();
}

void CaroActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAME_CARO));
  const int availableH = height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight - 12;
  const int cell = std::min((width - 16) / SIZE, availableH / SIZE);
  const int left = (width - cell * SIZE) / 2;
  const int top = metrics.topPadding + metrics.headerHeight + 6;
  for (int i = 0; i <= SIZE; ++i) {
    renderer.drawLine(left, top + i * cell, left + SIZE * cell, top + i * cell);
    renderer.drawLine(left + i * cell, top, left + i * cell, top + SIZE * cell);
  }
  for (int r = 0; r < SIZE; ++r) {
    for (int c = 0; c < SIZE; ++c) {
      const int x = left + c * cell, y = top + r * cell;
      if (r == cursorRow && c == cursorCol) renderer.drawRect(x + 3, y + 3, cell - 6, cell - 6, 2, true);
      if (cells[r][c] == 1) {
        renderer.drawLine(x + 7, y + 7, x + cell - 7, y + cell - 7, 2, true);
        renderer.drawLine(x + cell - 7, y + 7, x + 7, y + cell - 7, 2, true);
      } else if (cells[r][c] == 2) {
        const int inset = std::max(6, cell / 5);
        renderer.drawRoundedRect(x + inset, y + inset, cell - inset * 2, cell - inset * 2, 2,
                                 std::max(2, cell / 3), true);
      }
    }
  }
  if (winner) {
    const char* message = winner == 1 ? tr(STR_APP_YOU_WIN) : winner == 2 ? tr(STR_APP_GAME_AI_WINS) : tr(STR_APP_DRAW);
    renderer.drawCenteredText(UI_10_FONT_ID, top + cell * SIZE / 2, message, true, EpdFontFamily::BOLD);
  }
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), winner ? tr(STR_APP_NEW_GAME) : tr(STR_SELECT),
                                            tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
