#pragma once

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class AppsActivity final : public Activity {
  ButtonNavigator navigator;
  int selected = 0;

 public:
  AppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput) : Activity("Apps", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
