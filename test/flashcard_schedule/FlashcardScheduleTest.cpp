#include <gtest/gtest.h>

#include "FlashcardStore.h"

using flashcards::Rating;
using flashcards::ReviewState;

TEST(FlashcardSchedule, StartsGoodCardAtOneThenSixDays) {
  auto state = flashcards::schedule({}, Rating::Good, 100);
  EXPECT_EQ(state.intervalDays, 1);
  EXPECT_EQ(state.dueDay, 101U);
  state = flashcards::schedule(state, Rating::Good, 101);
  EXPECT_EQ(state.intervalDays, 6);
  EXPECT_EQ(state.dueDay, 107U);
}

TEST(FlashcardSchedule, AgainResetsAndNeverDropsEaseBelowFloor) {
  ReviewState state{42, 20, 130, 5, {}};
  for (int i = 0; i < 8; ++i) state = flashcards::schedule(state, Rating::Again, 200 + i);
  EXPECT_EQ(state.repetitions, 0);
  EXPECT_EQ(state.intervalDays, 1);
  EXPECT_EQ(state.easePercent, 130);
  EXPECT_EQ(state.dueDay, 208U);
}

TEST(FlashcardSchedule, EasyExpandsIntervalAndRewardsEase) {
  ReviewState state{10, 6, 250, 2, {}};
  const auto next = flashcards::schedule(state, Rating::Easy, 50);
  EXPECT_EQ(next.repetitions, 3);
  EXPECT_GT(next.intervalDays, state.intervalDays);
  EXPECT_EQ(next.easePercent, 265);
  EXPECT_EQ(next.dueDay, 50U + next.intervalDays);
}
