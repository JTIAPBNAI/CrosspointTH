#include "ThaiDraughtsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <esp_random.h>

#include <algorithm>

#include "components/UITheme.h"
#include "fontIds.h"

void ThaiDraughtsActivity::reset() {
  game.reset();
  cursorRow = 6;
  cursorCol = 1;
  selectedRow = selectedCol = -1;
  winner = 0;
  quietTurns = 0;
}

const ThaiDraughtsEngine::Move* ThaiDraughtsActivity::findMove(const ThaiDraughtsEngine::MoveList& moves,
                                                               int toRow, int toCol) const {
  for (uint8_t i = 0; i < moves.count; ++i)
    if (moves.moves[i].toRow == toRow && moves.moves[i].toCol == toCol) return &moves.moves[i];
  return nullptr;
}

void ThaiDraughtsActivity::finishTurn(uint8_t nextPlayer, bool captured) {
  quietTurns = captured ? 0 : static_cast<uint8_t>(std::min(80, quietTurns + 1));
  selectedRow = selectedCol = -1;
  if (!game.hasPieces(nextPlayer) || !game.hasMoves(nextPlayer)) winner = 3 - nextPlayer;
  if (quietTurns >= 80) winner = 3;
}

bool ThaiDraughtsActivity::selectOrMove() {
  if (selectedRow < 0) {
    const auto all = game.legalMoves(1);
    for (uint8_t i = 0; i < all.count; ++i) {
      if (all.moves[i].fromRow == cursorRow && all.moves[i].fromCol == cursorCol) {
        selectedRow = cursorRow;
        selectedCol = cursorCol;
        return true;
      }
    }
    return false;
  }

  const bool captureRequired = game.legalMoves(1).count && game.legalMoves(1).moves[0].isCapture();
  const auto moves = game.legalMovesFor(selectedRow, selectedCol, captureRequired);
  const auto* move = findMove(moves, cursorRow, cursorCol);
  if (!move) {
    if (!captureRequired && game.belongsTo(game.at(cursorRow, cursorCol), 1)) {
      selectedRow = cursorRow;
      selectedCol = cursorCol;
      return true;
    }
    return false;
  }
  const bool captured = move->isCapture();
  game.apply(*move);
  if (captured) {
    const auto more = game.legalMovesFor(cursorRow, cursorCol, true);
    if (more.count && more.moves[0].isCapture()) {
      selectedRow = cursorRow;
      selectedCol = cursorCol;
      return true;
    }
  }
  finishTurn(2, captured);
  if (!winner) runAiTurn();
  return true;
}

void ThaiDraughtsActivity::runAiTurn() {
  bool capturedAny = false;
  int continuationRow = -1;
  int continuationCol = -1;
  while (true) {
    auto moves = continuationRow >= 0 ? game.legalMovesFor(continuationRow, continuationCol, true) : game.legalMoves(2);
    if (!moves.count || (continuationRow >= 0 && !moves.moves[0].isCapture())) break;
    int bestScore = -10000;
    uint8_t best = 0;
    for (uint8_t i = 0; i < moves.count; ++i) {
      const auto& move = moves.moves[i];
      int score = move.isCapture() ? 100 : 0;
      const auto moving = game.at(move.fromRow, move.fromCol);
      if (moving == ThaiDraughtsEngine::AiMan && move.toRow == ThaiDraughtsEngine::SIZE - 1) score += 35;
      score += 7 - std::abs(3 - static_cast<int>(move.toCol));
      score += static_cast<int>(esp_random() % 5);
      if (score > bestScore) {
        bestScore = score;
        best = i;
      }
    }
    const auto chosen = moves.moves[best];
    capturedAny = capturedAny || chosen.isCapture();
    game.apply(chosen);
    continuationRow = chosen.toRow;
    continuationCol = chosen.toCol;
    if (!chosen.isCapture()) break;
  }
  finishTurn(1, capturedAny);
}

void ThaiDraughtsActivity::onEnter() {
  Activity::onEnter();
  reset();
  requestUpdate();
}

void ThaiDraughtsActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (winner && mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    reset();
    requestUpdate();
    return;
  }
  if (winner) return;
  bool changed = false;
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    cursorCol = (cursorCol + ThaiDraughtsEngine::SIZE - 1) % ThaiDraughtsEngine::SIZE;
    changed = true;
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    cursorCol = (cursorCol + 1) % ThaiDraughtsEngine::SIZE;
    changed = true;
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    cursorRow = (cursorRow + ThaiDraughtsEngine::SIZE - 1) % ThaiDraughtsEngine::SIZE;
    changed = true;
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    cursorRow = (cursorRow + 1) % ThaiDraughtsEngine::SIZE;
    changed = true;
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    changed = selectOrMove();
  }
  if (changed) requestUpdate();
}

void ThaiDraughtsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_APP_GAME_THAI_DRAUGHTS));
  const int availableH = height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight - 18;
  const int cell = std::min((width - 20) / 8, availableH / 8);
  const int left = (width - cell * 8) / 2;
  const int top = metrics.topPadding + metrics.headerHeight + 8;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const int x = left + col * cell;
      const int y = top + row * cell;
      renderer.drawRect(x, y, cell, cell, 1, true);
      if ((row + col) % 2 == 1) renderer.fillRect(x + 3, y + 3, 3, 3, true);
      if (row == cursorRow && col == cursorCol) renderer.drawRect(x + 4, y + 4, cell - 8, cell - 8, 2, true);
      if (row == selectedRow && col == selectedCol) renderer.drawRect(x + 8, y + 8, cell - 16, cell - 16, 3, true);
      const auto piece = game.at(row, col);
      if (piece == ThaiDraughtsEngine::Empty) continue;
      const bool human = piece == ThaiDraughtsEngine::HumanMan || piece == ThaiDraughtsEngine::HumanKing;
      const bool king = piece == ThaiDraughtsEngine::HumanKing || piece == ThaiDraughtsEngine::AiKing;
      const int inset = std::max(10, cell / 5);
      renderer.drawRoundedRect(x + inset, y + inset, cell - inset * 2, cell - inset * 2, human ? 3 : 1,
                               std::max(3, cell / 3), true);
      if (!human) {
        renderer.drawLine(x + inset + 4, y + inset + 4, x + cell - inset - 4, y + cell - inset - 4, 2, true);
        renderer.drawLine(x + cell - inset - 4, y + inset + 4, x + inset + 4, y + cell - inset - 4, 2, true);
      }
      if (king) renderer.drawRoundedRect(x + inset + 5, y + inset + 5, cell - inset * 2 - 10,
                                         cell - inset * 2 - 10, 2, std::max(2, cell / 4), true);
    }
  }
  if (winner) {
    const char* text = winner == 1 ? tr(STR_APP_YOU_WIN) : winner == 2 ? tr(STR_APP_GAME_AI_WINS) : tr(STR_APP_DRAW);
    renderer.drawCenteredText(UI_10_FONT_ID, top + cell * 4, text, true, EpdFontFamily::BOLD);
  }
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), winner ? tr(STR_APP_NEW_GAME) : tr(STR_SELECT),
                                            tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
