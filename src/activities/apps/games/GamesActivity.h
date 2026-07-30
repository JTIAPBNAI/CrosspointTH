#pragma once

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class GamesActivity final : public Activity {
  ButtonNavigator navigator;
  int selected = 0;

 public:
  GamesActivity(GfxRenderer& renderer, MappedInputManager& mappedInput) : Activity("Games", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
