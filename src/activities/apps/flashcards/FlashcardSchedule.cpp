#include "FlashcardStore.h"

#include <algorithm>

namespace flashcards {

ReviewState schedule(const ReviewState& current, Rating rating, uint32_t reviewDay) {
  ReviewState next = current;
  next.easePercent = std::clamp<uint16_t>(next.easePercent, 130, 350);
  switch (rating) {
    case Rating::Again:
      next.repetitions = 0;
      next.intervalDays = 1;
      next.easePercent = std::max<uint16_t>(130, next.easePercent - 20);
      break;
    case Rating::Hard:
      ++next.repetitions;
      next.intervalDays = std::max<uint16_t>(1, next.intervalDays == 0 ? 1 : next.intervalDays * 12 / 10);
      next.easePercent = std::max<uint16_t>(130, next.easePercent - 15);
      break;
    case Rating::Good:
      ++next.repetitions;
      if (next.repetitions == 1)
        next.intervalDays = 1;
      else if (next.repetitions == 2)
        next.intervalDays = 6;
      else
        next.intervalDays = std::max<uint16_t>(1, next.intervalDays * next.easePercent / 100);
      break;
    case Rating::Easy:
      ++next.repetitions;
      next.intervalDays = next.intervalDays == 0
                              ? 4
                              : std::max<uint16_t>(2, next.intervalDays * (next.easePercent + 30) / 100);
      next.easePercent = std::min<uint16_t>(350, next.easePercent + 15);
      break;
  }
  next.dueDay = reviewDay + next.intervalDays;
  return next;
}

}  // namespace flashcards
