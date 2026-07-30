#pragma once

#include <cstdint>

#include "activities/Activity.h"

class TwentyFortyEightActivity final : public Activity {
  uint16_t cells[4][4] = {};
  uint32_t score = 0;
  bool gameOver = false;

  void reset();
  void addTile();
  bool move(int dr, int dc);
  bool canMove() const;

 public:
  TwentyFortyEightActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("2048", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
