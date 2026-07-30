#pragma once

#include "ThaiDraughtsEngine.h"
#include "activities/Activity.h"

class ThaiDraughtsActivity final : public Activity {
  ThaiDraughtsEngine game;
  uint8_t cursorRow = 6;
  uint8_t cursorCol = 1;
  int8_t selectedRow = -1;
  int8_t selectedCol = -1;
  uint8_t winner = 0;
  uint8_t quietTurns = 0;

  void reset();
  bool selectOrMove();
  void runAiTurn();
  void finishTurn(uint8_t nextPlayer, bool captured);
  const ThaiDraughtsEngine::Move* findMove(const ThaiDraughtsEngine::MoveList& moves, int toRow, int toCol) const;

 public:
  ThaiDraughtsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ThaiDraughts", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
