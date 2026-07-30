#include "ClockCalendarActivity.h"

#include <GfxRenderer.h>
#include <HalClock.h>
#include <I18n.h>

#include <array>
#include <cstdio>
#include <ctime>

#include "CrossPointSettings.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr time_t VALID_TIME = 1704067200;

bool leapYear(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }

int daysInMonth(int year, int month) {
  static constexpr int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return month == 1 && leapYear(year) ? 29 : DAYS[month];
}

void shiftMonth(struct tm& value, int offset) {
  int absolute = value.tm_year * 12 + value.tm_mon + offset;
  value.tm_year = absolute / 12;
  value.tm_mon = absolute % 12;
  if (value.tm_mon < 0) {
    value.tm_mon += 12;
    --value.tm_year;
  }
  value.tm_mday = 1;
  value.tm_hour = 12;
  mktime(&value);
}

const char* thaiMonths[] = {"มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน", "พฤษภาคม", "มิถุนายน",
                            "กรกฎาคม", "สิงหาคม", "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"};
const char* englishMonths[] = {"January", "February", "March", "April", "May", "June",
                              "July", "August", "September", "October", "November", "December"};
const char* thaiWeekdays[] = {"อา", "จ", "อ", "พ", "พฤ", "ศ", "ส"};
const char* englishWeekdays[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
}  // namespace

bool ClockCalendarActivity::localDateTime(struct tm& out) const {
  const time_t now = time(nullptr);
  if (now < VALID_TIME) return false;
  const int quarterHours = static_cast<int>(SETTINGS.clockUtcOffsetQ) - 48;
  const time_t localEpoch = now + quarterHours * 15 * 60;
  gmtime_r(&localEpoch, &out);
  return true;
}

void ClockCalendarActivity::onEnter() {
  Activity::onEnter();
  lastMinuteCheck = millis();
  monthOffset = 0;
  requestUpdate();
}

void ClockCalendarActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    --monthOffset;
    requestUpdate();
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    ++monthOffset;
    requestUpdate();
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    monthOffset = 0;
    requestUpdate();
  }
  if (millis() - lastMinuteCheck >= 60000) {
    lastMinuteCheck = millis();
    requestUpdate();
  }
}

void ClockCalendarActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_CLOCK_CALENDAR));

  struct tm now{};
  if (!localDateTime(now)) {
    char fallback[12] = {};
    if (halClock.formatTime(fallback, sizeof(fallback), SETTINGS.clockUtcOffsetQ, SETTINGS.clockFormat == 1)) {
      renderer.drawCenteredText(UI_12_FONT_ID, height / 2 - 30, fallback, true, EpdFontFamily::BOLD);
    }
    UITheme::drawCenteredWrappedText(renderer, Rect{24, height / 2, width - 48, 100}, UI_10_FONT_ID,
                                     tr(STR_APP_CLOCK_NEEDS_SYNC), 3);
  } else {
    char timeText[12];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", now.tm_hour, now.tm_min);
    renderer.drawCenteredText(UI_12_FONT_ID, metrics.topPadding + metrics.headerHeight + 18, timeText, true,
                              EpdFontFamily::BOLD);

    struct tm shown = now;
    shiftMonth(shown, monthOffset);
    const bool thai = I18N.getLanguage() == Language::TH;
    char title[96];
    snprintf(title, sizeof(title), "%s %d", thai ? thaiMonths[shown.tm_mon] : englishMonths[shown.tm_mon],
             shown.tm_year + 1900 + (thai ? 543 : 0));
    renderer.drawCenteredText(UI_10_FONT_ID, metrics.topPadding + metrics.headerHeight + 80, title, true,
                              EpdFontFamily::BOLD);

    const int gridTop = metrics.topPadding + metrics.headerHeight + 125;
    const int cellW = width / 7;
    const int cellH = (height - gridTop - metrics.buttonHintsHeight - 12) / 7;
    for (int col = 0; col < 7; ++col) {
      const char* day = thai ? thaiWeekdays[col] : englishWeekdays[col];
      const int x = col * cellW + (cellW - renderer.getTextWidth(SMALL_FONT_ID, day)) / 2;
      renderer.drawText(SMALL_FONT_ID, x, gridTop, day, true, EpdFontFamily::BOLD);
    }
    const int firstWeekday = shown.tm_wday;
    const int count = daysInMonth(shown.tm_year + 1900, shown.tm_mon);
    for (int day = 1; day <= count; ++day) {
      const int slot = firstWeekday + day - 1;
      const int row = slot / 7;
      const int col = slot % 7;
      const int x0 = col * cellW;
      const int y0 = gridTop + cellH + row * cellH;
      const bool today = monthOffset == 0 && day == now.tm_mday;
      if (today) renderer.fillRect(x0 + 3, y0 - 3, cellW - 6, cellH - 2, true);
      char dayText[4];
      snprintf(dayText, sizeof(dayText), "%d", day);
      const int x = x0 + (cellW - renderer.getTextWidth(UI_10_FONT_ID, dayText)) / 2;
      renderer.drawText(UI_10_FONT_ID, x, y0, dayText, !today, today ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR);
    }
  }
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_APP_TODAY), tr(STR_DIR_LEFT), tr(STR_DIR_RIGHT));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
