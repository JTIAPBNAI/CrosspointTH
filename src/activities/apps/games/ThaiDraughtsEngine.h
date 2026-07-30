#pragma once

#include <array>
#include <cstdint>

class ThaiDraughtsEngine {
 public:
  static constexpr int SIZE = 8;
  enum Piece : uint8_t { Empty = 0, HumanMan = 1, AiMan = 2, HumanKing = 3, AiKing = 4 };
  struct Move {
    uint8_t fromRow = 0;
    uint8_t fromCol = 0;
    uint8_t toRow = 0;
    uint8_t toCol = 0;
    int8_t capturedRow = -1;
    int8_t capturedCol = -1;
    bool isCapture() const { return capturedRow >= 0; }
  };
  struct MoveList {
    std::array<Move, 64> moves{};
    uint8_t count = 0;
    void add(const Move& move) {
      if (count < moves.size()) moves[count++] = move;
    }
  };

  void reset();
  Piece at(int row, int col) const;
  void setForTest(int row, int col, Piece piece);
  void clearForTest();
  bool belongsTo(Piece piece, uint8_t player) const;
  MoveList legalMoves(uint8_t player) const;
  MoveList legalMovesFor(int row, int col, bool capturesOnly) const;
  bool apply(const Move& move);
  bool hasPieces(uint8_t player) const;
  bool hasMoves(uint8_t player) const;

 private:
  Piece board[SIZE][SIZE] = {};
  static bool inside(int row, int col);
  static bool isKing(Piece piece);
  static uint8_t owner(Piece piece);
};
