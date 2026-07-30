#pragma once

#include <cstdint>

#include "activities/Activity.h"

class MinesweeperActivity final : public Activity {
  static constexpr int ROWS = 12;
  static constexpr int COLS = 9;
  uint8_t values[ROWS][COLS] = {};
  uint8_t state[ROWS][COLS] = {};  // 0 hidden, 1 open, 2 flag
  uint8_t cursorRow = 0;
  uint8_t cursorCol = 0;
  bool gameOver = false;
  bool won = false;
  bool longPressHandled = false;

  void reset();
  void reveal(uint8_t row, uint8_t col);
  void checkWin();

 public:
  MinesweeperActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Minesweeper", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
