#include <gtest/gtest.h>

#include <limits>

#include "ReadingStatsCodec.h"

TEST(ReadingStatsCodec, RoundTripsBookSnapshot) {
  const ReadingStatsSnapshot expected{17, 123456, 789, 1};
  const auto encoded = ReadingStatsCodec::encodeBook(expected);

  ReadingStatsSnapshot actual;
  ASSERT_TRUE(ReadingStatsCodec::decodeBook(encoded.data(), encoded.size(), actual));
  EXPECT_EQ(actual.sessions, expected.sessions);
  EXPECT_EQ(actual.readingSeconds, expected.readingSeconds);
  EXPECT_EQ(actual.forwardPages, expected.forwardPages);
  EXPECT_EQ(actual.finishedBooks, 1U);
}

TEST(ReadingStatsCodec, RoundTripsGlobalSnapshot) {
  const ReadingStatsSnapshot expected{40000, 987654321, 123456, 321};
  const auto encoded = ReadingStatsCodec::encodeGlobal(expected);

  ReadingStatsSnapshot actual;
  ASSERT_TRUE(ReadingStatsCodec::decodeGlobal(encoded.data(), encoded.size(), actual));
  EXPECT_EQ(actual.sessions, expected.sessions);
  EXPECT_EQ(actual.readingSeconds, expected.readingSeconds);
  EXPECT_EQ(actual.forwardPages, expected.forwardPages);
  EXPECT_EQ(actual.finishedBooks, expected.finishedBooks);
}

TEST(ReadingStatsCodec, RejectsWrongVersionAndSize) {
  auto encoded = ReadingStatsCodec::encodeBook({});
  encoded[0] = ReadingStatsCodec::FORMAT_VERSION + 1;
  ReadingStatsSnapshot actual;
  EXPECT_FALSE(ReadingStatsCodec::decodeBook(encoded.data(), encoded.size(), actual));
  EXPECT_FALSE(ReadingStatsCodec::decodeBook(encoded.data(), encoded.size() - 1, actual));
}

TEST(ReadingStatsCodec, SaturatesCounters) {
  const uint32_t maximum = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(ReadingStatsCodec::addSaturated(maximum - 2, 10), maximum);
  EXPECT_EQ(ReadingStatsCodec::addSaturated(12, 30), 42U);
}
