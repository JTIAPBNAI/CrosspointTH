#include "FlashcardStore.h"

#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace flashcards {
namespace {
constexpr char DIRECTORY[] = "/flashcards";
constexpr uint32_t STATE_MAGIC = 0x31534654;  // TFS1

struct StateHeader {
  uint32_t magic;
  uint16_t cardCount;
  uint16_t recordSize;
};

bool endsWithCsv(const char* name) {
  const size_t n = strlen(name);
  return n >= 4 && strcasecmp(name + n - 4, ".csv") == 0;
}

std::string statePath(const std::string& deckPath) { return deckPath + ".srs"; }

bool readLine(HalFile& file, char* buffer, size_t size, bool& tooLong) {
  tooLong = false;
  size_t used = 0;
  bool readAny = false;
  while (file.available()) {
    const int value = file.read();
    if (value < 0) break;
    readAny = true;
    if (value == '\n') break;
    if (value == '\r') continue;
    if (used + 1 < size)
      buffer[used++] = static_cast<char>(value);
    else
      tooLong = true;
  }
  buffer[used] = '\0';
  return readAny;
}

size_t parseCsv(char* line, std::array<std::string, 8>& fields) {
  size_t count = 0;
  char* cursor = line;
  while (*cursor && count < fields.size()) {
    std::string value;
    bool quoted = *cursor == '"';
    if (quoted) ++cursor;
    while (*cursor) {
      if (quoted && *cursor == '"') {
        if (cursor[1] == '"') {
          value.push_back('"');
          cursor += 2;
          continue;
        }
        ++cursor;
        while (*cursor && *cursor != ',') ++cursor;
        break;
      }
      if (!quoted && *cursor == ',') break;
      value.push_back(*cursor++);
    }
    if (*cursor == ',') ++cursor;
    fields[count++] = std::move(value);
  }
  return count;
}

bool inspectDeck(const std::string& path, uint16_t& cardCount, int& frontColumn, int& backColumn) {
  HalFile file;
  if (!Storage.openFileForRead("FLASH", path, file)) return false;
  char line[MAX_CARD_LINE];
  bool tooLong = false;
  if (!readLine(file, line, sizeof(line), tooLong) || tooLong) {
    file.close();
    return false;
  }
  std::array<std::string, 8> fields;
  const size_t headerCount = parseCsv(line, fields);
  frontColumn = -1;
  backColumn = -1;
  for (size_t i = 0; i < headerCount; ++i) {
    if (fields[i] == "front" || fields[i] == "front_content") frontColumn = static_cast<int>(i);
    if (fields[i] == "back" || fields[i] == "back_content") backColumn = static_cast<int>(i);
  }
  if (frontColumn < 0 || backColumn < 0) {
    file.close();
    return false;
  }
  cardCount = 0;
  while (cardCount < MAX_CARDS && readLine(file, line, sizeof(line), tooLong)) {
    if (line[0] != '\0') ++cardCount;
  }
  file.close();
  return cardCount > 0;
}

bool prepareState(const std::string& deckPath, uint16_t cardCount) {
  const std::string path = statePath(deckPath);
  StateHeader header{};
  HalFile existing;
  if (Storage.openFileForRead("FLASH", path, existing)) {
    const int read = existing.read(&header, sizeof(header));
    const size_t expected = sizeof(StateHeader) + static_cast<size_t>(cardCount) * sizeof(ReviewState);
    const bool valid = read == static_cast<int>(sizeof(header)) && header.magic == STATE_MAGIC &&
                       header.cardCount == cardCount && header.recordSize == sizeof(ReviewState) &&
                       existing.size() == expected;
    existing.close();
    if (valid) return true;
  }

  HalFile output;
  if (!Storage.openFileForWrite("FLASH", path, output)) return false;
  header = {STATE_MAGIC, cardCount, sizeof(ReviewState)};
  if (output.write(&header, sizeof(header)) != sizeof(header)) {
    output.close();
    return false;
  }
  ReviewState blank;
  for (uint16_t i = 0; i < cardCount; ++i) {
    if (output.write(&blank, sizeof(blank)) != sizeof(blank)) {
      output.close();
      return false;
    }
  }
  output.flush();
  output.close();
  return true;
}

bool readCard(const std::string& path, uint16_t target, int frontColumn, int backColumn, Card& card) {
  HalFile file;
  if (!Storage.openFileForRead("FLASH", path, file)) return false;
  char line[MAX_CARD_LINE];
  bool tooLong = false;
  if (!readLine(file, line, sizeof(line), tooLong)) {
    file.close();
    return false;
  }
  uint16_t index = 0;
  while (readLine(file, line, sizeof(line), tooLong)) {
    if (line[0] == '\0') continue;
    if (index++ != target) continue;
    if (tooLong) {
      file.close();
      return false;
    }
    std::array<std::string, 8> fields;
    const size_t count = parseCsv(line, fields);
    if (frontColumn >= static_cast<int>(count) || backColumn >= static_cast<int>(count)) {
      file.close();
      return false;
    }
    card.index = target;
    card.front = std::move(fields[frontColumn]);
    card.back = std::move(fields[backColumn]);
    file.close();
    return !card.front.empty();
  }
  file.close();
  return false;
}
}  // namespace

std::vector<DeckInfo> listDecks() {
  Storage.ensureDirectoryExists(DIRECTORY);
  std::vector<DeckInfo> result;
  HalFile directory = Storage.open(DIRECTORY);
  if (!directory || !directory.isDirectory()) return result;
  while (result.size() < 64) {
    HalFile entry = directory.openNextFile();
    if (!entry) break;
    char name[160] = {};
    entry.getName(name, sizeof(name));
    const bool usable = !entry.isDirectory() && endsWithCsv(name);
    entry.close();
    if (!usable) continue;
    std::string display = name;
    const size_t slash = display.find_last_of('/');
    if (slash != std::string::npos) display.erase(0, slash + 1);
    const std::string filename = display;
    if (display.size() >= 4) display.resize(display.size() - 4);
    result.push_back({display, std::string(DIRECTORY) + "/" + filename});
  }
  directory.close();
  std::sort(result.begin(), result.end(), [](const DeckInfo& a, const DeckInfo& b) { return a.name < b.name; });
  return result;
}

uint32_t today() {
  const time_t now = time(nullptr);
  constexpr time_t EPOCH_2024 = 1704067200;
  return now >= EPOCH_2024 ? static_cast<uint32_t>((now - EPOCH_2024) / 86400) + 1 : 1;
}

bool findDueCard(const std::string& deckPath, uint16_t startIndex, Card& card, ReviewState& review,
                 uint16_t& cardCount) {
  int frontColumn = -1;
  int backColumn = -1;
  if (!inspectDeck(deckPath, cardCount, frontColumn, backColumn) || !prepareState(deckPath, cardCount)) return false;
  HalFile state;
  if (!Storage.openFileForRead("FLASH", statePath(deckPath), state)) return false;
  const uint32_t reviewDay = today();
  for (uint16_t pass = 0; pass < cardCount; ++pass) {
    const uint16_t index = static_cast<uint16_t>((startIndex + pass) % cardCount);
    const size_t offset = sizeof(StateHeader) + static_cast<size_t>(index) * sizeof(ReviewState);
    if (!state.seekSet(offset) || state.read(&review, sizeof(review)) != static_cast<int>(sizeof(review))) break;
    if (review.dueDay == 0 || review.dueDay <= reviewDay) {
      state.close();
      return readCard(deckPath, index, frontColumn, backColumn, card);
    }
  }
  state.close();
  return false;
}

bool saveReview(const std::string& deckPath, uint16_t cardIndex, const ReviewState& review) {
  HalFile file = Storage.open(statePath(deckPath).c_str(), O_RDWR);
  if (!file) return false;
  const size_t offset = sizeof(StateHeader) + static_cast<size_t>(cardIndex) * sizeof(ReviewState);
  const bool ok = file.seekSet(offset) && file.write(&review, sizeof(review)) == sizeof(review);
  file.flush();
  file.close();
  return ok;
}

}  // namespace flashcards
