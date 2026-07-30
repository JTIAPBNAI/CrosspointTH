#pragma once

#include <cstdint>

#include "activities/Activity.h"

class SudokuActivity final : public Activity {
  uint8_t puzzle[9][9] = {};
  uint8_t solution[9][9] = {};
  bool fixed[9][9] = {};
  uint8_t cursorRow = 0;
  uint8_t cursorCol = 0;
  uint8_t variation = 0;
  bool complete = false;

  void reset();
  void updateComplete();

 public:
  SudokuActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Sudoku", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
