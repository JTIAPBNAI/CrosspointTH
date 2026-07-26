#include <gtest/gtest.h>

#include <ThaiBrk.h>

#include <string>

TEST(ThaiBreaking, HandlesLongMixedThaiAsciiWithoutCapacityGrowthFailure) {
  std::string line;
  const std::string fragment =
      "ท่ามกลางความอ่อนแอของระบบการศึกษาในโรงเรียน (Bray, 1999) shadow education — ";
  while (line.size() < 5795) line += fragment;

  const auto breaks = ThaiBrk::wordBreakByteOffsets(line);

  ASSERT_FALSE(breaks.empty());
  size_t previous = 0;
  for (const size_t offset : breaks) {
    EXPECT_GT(offset, previous);
    EXPECT_LT(offset, line.size());
    // A returned offset must not point into a UTF-8 continuation byte.
    EXPECT_NE(static_cast<unsigned char>(line[offset]) & 0xC0, 0x80);
    previous = offset;
  }
}
