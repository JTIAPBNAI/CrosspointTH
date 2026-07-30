#pragma once

#include <cstdint>

#include "activities/Activity.h"

class CaroActivity final : public Activity {
  static constexpr int SIZE = 11;
  uint8_t cells[SIZE][SIZE] = {};
  uint8_t cursorRow = SIZE / 2;
  uint8_t cursorCol = SIZE / 2;
  uint8_t winner = 0;

  void reset();
  bool wins(int row, int col, uint8_t player) const;
  void aiMove(int humanRow, int humanCol);

 public:
  CaroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput) : Activity("Caro", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
