#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

struct ReadingStatsSnapshot {
  uint32_t sessions = 0;
  uint32_t readingSeconds = 0;
  uint32_t forwardPages = 0;
  uint32_t finishedBooks = 0;
};

namespace ReadingStatsCodec {

constexpr uint8_t FORMAT_VERSION = 1;
constexpr size_t BOOK_FILE_SIZE = 14;
constexpr size_t GLOBAL_FILE_SIZE = 17;

inline uint32_t addSaturated(const uint32_t lhs, const uint32_t rhs) {
  return std::numeric_limits<uint32_t>::max() - lhs < rhs ? std::numeric_limits<uint32_t>::max() : lhs + rhs;
}

inline uint32_t readLe32(const uint8_t* data, const size_t offset) {
  return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8) |
         (static_cast<uint32_t>(data[offset + 2]) << 16) | (static_cast<uint32_t>(data[offset + 3]) << 24);
}

inline void writeLe32(uint8_t* data, const size_t offset, const uint32_t value) {
  data[offset] = static_cast<uint8_t>(value);
  data[offset + 1] = static_cast<uint8_t>(value >> 8);
  data[offset + 2] = static_cast<uint8_t>(value >> 16);
  data[offset + 3] = static_cast<uint8_t>(value >> 24);
}

inline std::array<uint8_t, BOOK_FILE_SIZE> encodeBook(const ReadingStatsSnapshot& stats) {
  std::array<uint8_t, BOOK_FILE_SIZE> data{};
  data[0] = FORMAT_VERSION;
  writeLe32(data.data(), 1, stats.sessions);
  writeLe32(data.data(), 5, stats.readingSeconds);
  writeLe32(data.data(), 9, stats.forwardPages);
  data[13] = stats.finishedBooks > 0 ? 1 : 0;
  return data;
}

inline bool decodeBook(const uint8_t* data, const size_t size, ReadingStatsSnapshot& stats) {
  if (data == nullptr || size != BOOK_FILE_SIZE || data[0] != FORMAT_VERSION) return false;
  stats.sessions = readLe32(data, 1);
  stats.readingSeconds = readLe32(data, 5);
  stats.forwardPages = readLe32(data, 9);
  stats.finishedBooks = data[13] != 0 ? 1 : 0;
  return true;
}

inline std::array<uint8_t, GLOBAL_FILE_SIZE> encodeGlobal(const ReadingStatsSnapshot& stats) {
  std::array<uint8_t, GLOBAL_FILE_SIZE> data{};
  data[0] = FORMAT_VERSION;
  writeLe32(data.data(), 1, stats.sessions);
  writeLe32(data.data(), 5, stats.readingSeconds);
  writeLe32(data.data(), 9, stats.forwardPages);
  writeLe32(data.data(), 13, stats.finishedBooks);
  return data;
}

inline bool decodeGlobal(const uint8_t* data, const size_t size, ReadingStatsSnapshot& stats) {
  if (data == nullptr || size != GLOBAL_FILE_SIZE || data[0] != FORMAT_VERSION) return false;
  stats.sessions = readLe32(data, 1);
  stats.readingSeconds = readLe32(data, 5);
  stats.forwardPages = readLe32(data, 9);
  stats.finishedBooks = readLe32(data, 13);
  return true;
}

}  // namespace ReadingStatsCodec
