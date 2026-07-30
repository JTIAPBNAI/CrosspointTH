#include "PomodoroActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdio>

#include "apps/AppSettings.h"
#include "components/UITheme.h"
#include "fontIds.h"

uint32_t PomodoroActivity::remainingMs() const {
  if (phase == Phase::Ready) return APP_SETTINGS.pomodoroFocusMinutes * 60000UL;
  const uint32_t elapsed = elapsedBeforePause +
                           (phase == Phase::Paused ? 0 : static_cast<uint32_t>(millis() - startedAt));
  return elapsed >= durationMs ? 0 : durationMs - elapsed;
}

void PomodoroActivity::start(Phase next, uint8_t minutes) {
  phase = next;
  durationMs = minutes * 60000UL;
  elapsedBeforePause = 0;
  startedAt = millis();
  lastDisplayedMinute = UINT32_MAX;
  requestUpdate();
}

void PomodoroActivity::advance() {
  if (phase == Phase::Focus || (phase == Phase::Paused && pausedPhase == Phase::Focus)) {
    ++sessions;
    if (sessions % 4 == 0)
      start(Phase::LongBreak, APP_SETTINGS.pomodoroLongBreakMinutes);
    else
      start(Phase::ShortBreak, APP_SETTINGS.pomodoroShortBreakMinutes);
  } else {
    start(Phase::Focus, APP_SETTINGS.pomodoroFocusMinutes);
  }
}

void PomodoroActivity::onEnter() {
  Activity::onEnter();
  APP_SETTINGS.loadFromFile();
  phase = Phase::Ready;
  sessions = 0;
  requestUpdate();
}

void PomodoroActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (phase == Phase::Ready) {
      start(Phase::Focus, APP_SETTINGS.pomodoroFocusMinutes);
    } else if (phase == Phase::Paused) {
      phase = pausedPhase;
      startedAt = millis();
      requestUpdate();
    } else {
      pausedPhase = phase;
      elapsedBeforePause += millis() - startedAt;
      phase = Phase::Paused;
      requestUpdate();
    }
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Right) && phase != Phase::Ready) {
    advance();
    return;
  }
  if (phase != Phase::Ready && phase != Phase::Paused) {
    const uint32_t remaining = remainingMs();
    if (remaining == 0) {
      advance();
      return;
    }
    const uint32_t minute = (remaining + 59999) / 60000;
    if (minute != lastDisplayedMinute) {
      lastDisplayedMinute = minute;
      requestUpdate();
    }
  }
}

void PomodoroActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_POMODORO));

  const char* label = tr(STR_APP_POMO_READY);
  if (phase == Phase::Focus) label = tr(STR_APP_POMO_FOCUS);
  if (phase == Phase::ShortBreak) label = tr(STR_APP_POMO_SHORT_BREAK);
  if (phase == Phase::LongBreak) label = tr(STR_APP_POMO_LONG_BREAK);
  if (phase == Phase::Paused) label = tr(STR_APP_POMO_PAUSED);
  renderer.drawCenteredText(UI_10_FONT_ID, height / 2 - 90, label, true, EpdFontFamily::BOLD);

  const uint32_t seconds = (remainingMs() + 999) / 1000;
  char timer[12];
  snprintf(timer, sizeof(timer), "%02lu:%02lu", static_cast<unsigned long>(seconds / 60),
           static_cast<unsigned long>(seconds % 60));
  renderer.drawCenteredText(NOTOSANS_18_FONT_ID, height / 2 - 35, timer, true, EpdFontFamily::BOLD);
  char sessionText[48];
  snprintf(sessionText, sizeof(sessionText), "%s: %u", tr(STR_APP_POMO_SESSIONS), sessions);
  renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 50, sessionText);

  const char* confirm = phase == Phase::Ready ? tr(STR_APP_START) :
                        phase == Phase::Paused ? tr(STR_APP_RESUME) : tr(STR_APP_PAUSE);
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), confirm, "", phase == Phase::Ready ? "" : tr(STR_APP_SKIP));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
