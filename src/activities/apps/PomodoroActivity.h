#pragma once

#include "activities/Activity.h"

class PomodoroActivity final : public Activity {
  enum class Phase : uint8_t { Ready, Focus, ShortBreak, LongBreak, Paused };
  Phase phase = Phase::Ready;
  Phase pausedPhase = Phase::Focus;
  uint8_t sessions = 0;
  uint32_t durationMs = 0;
  uint32_t elapsedBeforePause = 0;
  unsigned long startedAt = 0;
  unsigned long lastDisplayedMinute = UINT32_MAX;

  uint32_t remainingMs() const;
  void start(Phase next, uint8_t minutes);
  void advance();

 public:
  PomodoroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Pomodoro", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool preventAutoSleep() override { return phase != Phase::Ready && phase != Phase::Paused; }
};
