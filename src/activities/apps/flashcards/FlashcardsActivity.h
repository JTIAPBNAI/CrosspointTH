#pragma once

#include "FlashcardStore.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class FlashcardsActivity final : public Activity {
  enum class Screen : uint8_t { Decks, Question, Answer, Done, Invalid };
  Screen screen = Screen::Decks;
  ButtonNavigator navigator;
  std::vector<flashcards::DeckInfo> decks;
  int selectedDeck = 0;
  flashcards::Card card;
  flashcards::ReviewState review;
  uint16_t cardCount = 0;
  uint16_t reviewed = 0;

  void openDeck();
  void loadNext(uint16_t startIndex);
  void applyRating(flashcards::Rating rating);

 public:
  FlashcardsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Flashcards", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
