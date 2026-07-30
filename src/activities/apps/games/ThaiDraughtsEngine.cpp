#include "ThaiDraughtsEngine.h"

#include <cstring>

bool ThaiDraughtsEngine::inside(int row, int col) { return row >= 0 && row < SIZE && col >= 0 && col < SIZE; }
bool ThaiDraughtsEngine::isKing(Piece piece) { return piece == HumanKing || piece == AiKing; }
uint8_t ThaiDraughtsEngine::owner(Piece piece) {
  if (piece == HumanMan || piece == HumanKing) return 1;
  if (piece == AiMan || piece == AiKing) return 2;
  return 0;
}

void ThaiDraughtsEngine::reset() {
  memset(board, 0, sizeof(board));
  for (int row = 0; row < 2; ++row)
    for (int col = 0; col < SIZE; ++col)
      if ((row + col) % 2 == 1) board[row][col] = AiMan;
  for (int row = SIZE - 2; row < SIZE; ++row)
    for (int col = 0; col < SIZE; ++col)
      if ((row + col) % 2 == 1) board[row][col] = HumanMan;
}

ThaiDraughtsEngine::Piece ThaiDraughtsEngine::at(int row, int col) const {
  return inside(row, col) ? board[row][col] : Empty;
}
void ThaiDraughtsEngine::setForTest(int row, int col, Piece piece) {
  if (inside(row, col)) board[row][col] = piece;
}
void ThaiDraughtsEngine::clearForTest() { memset(board, 0, sizeof(board)); }
bool ThaiDraughtsEngine::belongsTo(Piece piece, uint8_t player) const { return owner(piece) == player; }

ThaiDraughtsEngine::MoveList ThaiDraughtsEngine::legalMovesFor(int row, int col, bool capturesOnly) const {
  MoveList captures;
  MoveList quiet;
  const Piece piece = at(row, col);
  const uint8_t player = owner(piece);
  if (!player) return captures;
  constexpr int DIRS[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  if (!isKing(piece)) {
    const int direction = player == 1 ? -1 : 1;
    for (int dc : {-1, 1}) {
      const int middleRow = row + direction;
      const int middleCol = col + dc;
      const int landingRow = row + direction * 2;
      const int landingCol = col + dc * 2;
      if (inside(landingRow, landingCol) && at(landingRow, landingCol) == Empty &&
          owner(at(middleRow, middleCol)) == 3 - player) {
        captures.add({static_cast<uint8_t>(row), static_cast<uint8_t>(col), static_cast<uint8_t>(landingRow),
                      static_cast<uint8_t>(landingCol), static_cast<int8_t>(middleRow),
                      static_cast<int8_t>(middleCol)});
      }
      if (!capturesOnly && inside(middleRow, middleCol) && at(middleRow, middleCol) == Empty) {
        quiet.add({static_cast<uint8_t>(row), static_cast<uint8_t>(col), static_cast<uint8_t>(middleRow),
                   static_cast<uint8_t>(middleCol)});
      }
    }
  } else {
    for (int directionIndex = 0; directionIndex < 4; ++directionIndex) {
      const int dr = DIRS[directionIndex][0];
      const int dc = DIRS[directionIndex][1];
      int scanRow = row + dr;
      int scanCol = col + dc;
      while (inside(scanRow, scanCol) && at(scanRow, scanCol) == Empty) {
        if (!capturesOnly) quiet.add({static_cast<uint8_t>(row), static_cast<uint8_t>(col),
                                     static_cast<uint8_t>(scanRow), static_cast<uint8_t>(scanCol)});
        scanRow += dr;
        scanCol += dc;
      }
      if (!inside(scanRow, scanCol) || owner(at(scanRow, scanCol)) != 3 - player) continue;
      const int landingRow = scanRow + dr;
      const int landingCol = scanCol + dc;
      if (inside(landingRow, landingCol) && at(landingRow, landingCol) == Empty) {
        captures.add({static_cast<uint8_t>(row), static_cast<uint8_t>(col), static_cast<uint8_t>(landingRow),
                      static_cast<uint8_t>(landingCol), static_cast<int8_t>(scanRow), static_cast<int8_t>(scanCol)});
      }
    }
  }
  return captures.count ? captures : quiet;
}

ThaiDraughtsEngine::MoveList ThaiDraughtsEngine::legalMoves(uint8_t player) const {
  MoveList captures;
  MoveList quiet;
  for (int row = 0; row < SIZE; ++row) {
    for (int col = 0; col < SIZE; ++col) {
      if (!belongsTo(at(row, col), player)) continue;
      const MoveList pieceMoves = legalMovesFor(row, col, false);
      for (uint8_t i = 0; i < pieceMoves.count; ++i) {
        if (pieceMoves.moves[i].isCapture())
          captures.add(pieceMoves.moves[i]);
        else
          quiet.add(pieceMoves.moves[i]);
      }
    }
  }
  return captures.count ? captures : quiet;
}

bool ThaiDraughtsEngine::apply(const Move& move) {
  if (!inside(move.fromRow, move.fromCol) || !inside(move.toRow, move.toCol) ||
      board[move.toRow][move.toCol] != Empty) {
    return false;
  }
  Piece piece = board[move.fromRow][move.fromCol];
  if (piece == Empty) return false;
  board[move.fromRow][move.fromCol] = Empty;
  if (move.isCapture()) {
    if (!inside(move.capturedRow, move.capturedCol) || owner(board[move.capturedRow][move.capturedCol]) != 3 - owner(piece)) {
      board[move.fromRow][move.fromCol] = piece;
      return false;
    }
    board[move.capturedRow][move.capturedCol] = Empty;
  }
  if (piece == HumanMan && move.toRow == 0) piece = HumanKing;
  if (piece == AiMan && move.toRow == SIZE - 1) piece = AiKing;
  board[move.toRow][move.toCol] = piece;
  return true;
}

bool ThaiDraughtsEngine::hasPieces(uint8_t player) const {
  for (int row = 0; row < SIZE; ++row)
    for (int col = 0; col < SIZE; ++col)
      if (belongsTo(board[row][col], player)) return true;
  return false;
}
bool ThaiDraughtsEngine::hasMoves(uint8_t player) const { return legalMoves(player).count > 0; }
