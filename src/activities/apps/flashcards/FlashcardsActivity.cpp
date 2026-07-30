#include "FlashcardsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdio>

#include "components/UITheme.h"
#include "fontIds.h"

void FlashcardsActivity::onEnter() {
  Activity::onEnter();
  decks = flashcards::listDecks();
  selectedDeck = 0;
  screen = Screen::Decks;
  requestUpdate();
}

void FlashcardsActivity::openDeck() {
  if (decks.empty()) return;
  reviewed = 0;
  loadNext(0);
}

void FlashcardsActivity::loadNext(uint16_t startIndex) {
  if (flashcards::findDueCard(decks[selectedDeck].path, startIndex, card, review, cardCount))
    screen = Screen::Question;
  else
    screen = Screen::Done;
  requestUpdate();
}

void FlashcardsActivity::applyRating(flashcards::Rating rating) {
  const auto next = flashcards::schedule(review, rating, flashcards::today());
  if (!flashcards::saveReview(decks[selectedDeck].path, card.index, next)) {
    screen = Screen::Invalid;
    requestUpdate();
    return;
  }
  ++reviewed;
  loadNext(static_cast<uint16_t>(card.index + 1));
}

void FlashcardsActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    if (screen == Screen::Decks) {
      finish();
    } else {
      screen = Screen::Decks;
      requestUpdate();
    }
    return;
  }
  if (screen == Screen::Decks) {
    const int count = static_cast<int>(decks.size());
    if (count == 0) return;
    navigator.onNext([this, count] {
      selectedDeck = ButtonNavigator::nextIndex(selectedDeck, count);
      requestUpdate();
    });
    navigator.onPrevious([this, count] {
      selectedDeck = ButtonNavigator::previousIndex(selectedDeck, count);
      requestUpdate();
    });
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) openDeck();
    return;
  }
  if (screen == Screen::Question && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    screen = Screen::Answer;
    requestUpdate();
    return;
  }
  if (screen == Screen::Answer) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Left)) applyRating(flashcards::Rating::Again);
    else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) applyRating(flashcards::Rating::Hard);
    else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) applyRating(flashcards::Rating::Good);
    else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) applyRating(flashcards::Rating::Easy);
  }
}

void FlashcardsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_FLASHCARDS));

  if (screen == Screen::Decks) {
    if (decks.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, height / 2 - 35, tr(STR_APP_NO_DECKS), true, EpdFontFamily::BOLD);
      UITheme::drawCenteredWrappedText(renderer, Rect{30, height / 2 + 10, width - 60, 100}, SMALL_FONT_ID,
                                       tr(STR_APP_NEW_DECK_HINT), 3);
    } else {
      GUI.drawButtonMenu(renderer,
                         Rect{0, metrics.topPadding + metrics.headerHeight, width,
                              height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight},
                         static_cast<int>(decks.size()), selectedDeck,
                         [this](int index) { return decks[static_cast<size_t>(index)].name; },
                         [](int) { return UIIcon::None; });
    }
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), decks.empty() ? "" : tr(STR_SELECT), tr(STR_DIR_UP),
                                              tr(STR_DIR_DOWN));
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (screen == Screen::Question || screen == Screen::Answer) {
    char progress[32];
    snprintf(progress, sizeof(progress), "%u / %u", static_cast<unsigned>(card.index + 1), cardCount);
    renderer.drawCenteredText(SMALL_FONT_ID, metrics.topPadding + metrics.headerHeight + 12, progress);
    const char* content = screen == Screen::Question ? card.front.c_str() : card.back.c_str();
    UITheme::drawCenteredWrappedText(renderer,
                                     Rect{24, metrics.topPadding + metrics.headerHeight + 55, width - 48,
                                          height - metrics.topPadding - metrics.headerHeight - 140},
                                     UI_12_FONT_ID, content, 8, true, EpdFontFamily::REGULAR);
    if (screen == Screen::Question) {
      const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_APP_SHOW_ANSWER), "", "");
      GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
    } else {
      const auto labels = mappedInput.mapLabels(tr(STR_APP_AGAIN), "", tr(STR_APP_HARD), tr(STR_APP_GOOD));
      GUI.drawButtonHints(renderer, labels.btn1, tr(STR_APP_EASY), labels.btn3, labels.btn4);
    }
  } else {
    const char* message = screen == Screen::Done ? tr(STR_APP_NO_CARDS_DUE) : tr(STR_APP_CARD_TOO_LONG);
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2, message, true, EpdFontFamily::BOLD);
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }
  renderer.displayBuffer();
}
