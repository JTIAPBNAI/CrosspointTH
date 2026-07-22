#include "ReadingStatsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <utility>

#include "MappedInputManager.h"
#include "ReadingStats.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {

void drawNumberRow(GfxRenderer& renderer, const int x, const int y, const char* label, const uint32_t value) {
  char line[96];
  snprintf(line, sizeof(line), "%s: %lu", label, static_cast<unsigned long>(value));
  renderer.drawText(UI_10_FONT_ID, x, y, line, true);
}

void drawDurationRow(GfxRenderer& renderer, const int x, const int y, const char* label, const uint32_t seconds) {
  char duration[32];
  char line[112];
  ReadingStatsStore::formatDuration(seconds, duration, sizeof(duration));
  snprintf(line, sizeof(line), "%s: %s", label, duration);
  renderer.drawText(UI_10_FONT_ID, x, y, line, true);
}

}  // namespace

ReadingStatsActivity::ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string title,
                                           const ReadingStatsSnapshot& book, const ReadingStatsSnapshot& global)
    : Activity("ReadingStats", renderer, mappedInput),
      bookTitle(std::move(title)),
      bookStats(book),
      globalStats(global) {}

void ReadingStatsActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void ReadingStatsActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) ||
      mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    finish();
  }
}

void ReadingStatsActivity::render(RenderLock&&) {
  renderer.clearScreen();
  const int width = renderer.getScreenWidth();
  const int margin = 24;
  int y = 24;

  renderer.drawCenteredText(UI_12_FONT_ID, y, tr(STR_READING_STATS), true, EpdFontFamily::BOLD);
  y += 42;
  const std::string title = renderer.truncatedText(UI_10_FONT_ID, bookTitle.c_str(), width - margin * 2);
  renderer.drawCenteredText(UI_10_FONT_ID, y, title.c_str(), true);
  y += 42;

  renderer.drawText(UI_10_FONT_ID, margin, y, tr(STR_STATS_THIS_BOOK), true, EpdFontFamily::BOLD);
  y += 34;
  drawNumberRow(renderer, margin, y, tr(STR_STATS_SESSIONS), bookStats.sessions);
  y += 32;
  drawDurationRow(renderer, margin, y, tr(STR_STATS_TOTAL_TIME), bookStats.readingSeconds);
  y += 32;
  drawNumberRow(renderer, margin, y, tr(STR_STATS_PAGES_TURNED), bookStats.forwardPages);
  y += 32;
  const uint32_t average = bookStats.sessions > 0 ? bookStats.readingSeconds / bookStats.sessions : 0;
  drawDurationRow(renderer, margin, y, tr(STR_STATS_AVG_SESSION), average);
  y += 44;

  renderer.drawLine(margin, y, width - margin, y, true);
  y += 28;
  renderer.drawText(UI_10_FONT_ID, margin, y, tr(STR_STATS_ALL_BOOKS), true, EpdFontFamily::BOLD);
  y += 34;
  drawNumberRow(renderer, margin, y, tr(STR_STATS_BOOKS_FINISHED), globalStats.finishedBooks);
  y += 32;
  drawNumberRow(renderer, margin, y, tr(STR_STATS_SESSIONS), globalStats.sessions);
  y += 32;
  drawDurationRow(renderer, margin, y, tr(STR_STATS_TOTAL_TIME), globalStats.readingSeconds);
  y += 32;
  drawNumberRow(renderer, margin, y, tr(STR_STATS_PAGES_TURNED), globalStats.forwardPages);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
