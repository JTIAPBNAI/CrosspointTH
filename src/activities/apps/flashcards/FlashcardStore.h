#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace flashcards {

constexpr uint16_t MAX_CARDS = 2000;
constexpr size_t MAX_CARD_LINE = 768;

struct DeckInfo {
  std::string name;
  std::string path;
};

struct Card {
  uint16_t index = 0;
  std::string front;
  std::string back;
};

struct ReviewState {
  uint32_t dueDay = 0;
  uint16_t intervalDays = 0;
  uint16_t easePercent = 250;
  uint8_t repetitions = 0;
  uint8_t reserved[3] = {};
};

enum class Rating : uint8_t { Again, Hard, Good, Easy };

std::vector<DeckInfo> listDecks();
uint32_t today();
bool findDueCard(const std::string& deckPath, uint16_t startIndex, Card& card, ReviewState& review,
                 uint16_t& cardCount);
bool saveReview(const std::string& deckPath, uint16_t cardIndex, const ReviewState& review);
ReviewState schedule(const ReviewState& current, Rating rating, uint32_t reviewDay);

}  // namespace flashcards
