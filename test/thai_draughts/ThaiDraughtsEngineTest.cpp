#include <gtest/gtest.h>

#include "ThaiDraughtsEngine.h"

TEST(ThaiDraughtsEngine, StartsWithEightPiecesPerSide) {
  ThaiDraughtsEngine game;
  game.reset();
  int human = 0;
  int ai = 0;
  for (int row = 0; row < 8; ++row)
    for (int col = 0; col < 8; ++col) {
      human += game.belongsTo(game.at(row, col), 1);
      ai += game.belongsTo(game.at(row, col), 2);
    }
  EXPECT_EQ(human, 8);
  EXPECT_EQ(ai, 8);
}

TEST(ThaiDraughtsEngine, MandatoryCaptureSuppressesQuietMoves) {
  ThaiDraughtsEngine game;
  game.clearForTest();
  game.setForTest(5, 2, ThaiDraughtsEngine::HumanMan);
  game.setForTest(4, 3, ThaiDraughtsEngine::AiMan);
  game.setForTest(5, 6, ThaiDraughtsEngine::HumanMan);
  const auto moves = game.legalMoves(1);
  ASSERT_EQ(moves.count, 1);
  EXPECT_TRUE(moves.moves[0].isCapture());
  EXPECT_EQ(moves.moves[0].toRow, 3);
  EXPECT_EQ(moves.moves[0].toCol, 4);
}

TEST(ThaiDraughtsEngine, KingScansButLandsImmediatelyBehindCapture) {
  ThaiDraughtsEngine game;
  game.clearForTest();
  game.setForTest(6, 1, ThaiDraughtsEngine::HumanKing);
  game.setForTest(3, 4, ThaiDraughtsEngine::AiMan);
  const auto moves = game.legalMovesFor(6, 1, true);
  ASSERT_EQ(moves.count, 1);
  EXPECT_EQ(moves.moves[0].capturedRow, 3);
  EXPECT_EQ(moves.moves[0].capturedCol, 4);
  EXPECT_EQ(moves.moves[0].toRow, 2);
  EXPECT_EQ(moves.moves[0].toCol, 5);
}

TEST(ThaiDraughtsEngine, CaptureCanContinueAndPromotes) {
  ThaiDraughtsEngine game;
  game.clearForTest();
  game.setForTest(4, 1, ThaiDraughtsEngine::HumanMan);
  game.setForTest(3, 2, ThaiDraughtsEngine::AiMan);
  game.setForTest(1, 4, ThaiDraughtsEngine::AiMan);
  auto moves = game.legalMovesFor(4, 1, true);
  ASSERT_EQ(moves.count, 1);
  ASSERT_TRUE(game.apply(moves.moves[0]));
  moves = game.legalMovesFor(2, 3, true);
  ASSERT_EQ(moves.count, 1);
  ASSERT_TRUE(game.apply(moves.moves[0]));
  EXPECT_EQ(game.at(0, 5), ThaiDraughtsEngine::HumanKing);
  EXPECT_FALSE(game.hasPieces(2));
}
