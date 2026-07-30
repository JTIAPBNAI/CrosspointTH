#pragma once

#include <ctime>

#include "activities/Activity.h"

class ClockCalendarActivity final : public Activity {
  unsigned long lastMinuteCheck = 0;
  int monthOffset = 0;

  bool localDateTime(struct tm& out) const;

 public:
  ClockCalendarActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ClockCalendar", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
