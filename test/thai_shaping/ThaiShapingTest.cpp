#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "ThaiShaping.h"
#include "Utf8.h"

namespace {

std::vector<uint32_t> codepoints(const std::string& text) {
  std::vector<uint32_t> result;
  const auto* cursor = reinterpret_cast<const unsigned char*>(text.c_str());
  while (const uint32_t cp = utf8NextCodepoint(&cursor)) result.push_back(cp);
  return result;
}

}  // namespace

TEST(ThaiShaping, KeepsToneAtNormalTierWithoutUpperVowel) {
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("อ่")), (std::vector<uint32_t>{0x0E2D, 0x0E48}));
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("ป่")), (std::vector<uint32_t>{0x0E1B, 0xF713}));
}

TEST(ThaiShaping, SelectsHighTierAboveSaraUeAndSaraUee) {
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("อึ่")), (std::vector<uint32_t>{0x0E2D, 0x0E36, 0xF70A}));
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("อื้อ")),
            (std::vector<uint32_t>{0x0E2D, 0x0E37, 0xF70B, 0x0E2D}));
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("ปึ้")), (std::vector<uint32_t>{0x0E1B, 0xF703, 0xF706}));
}

TEST(ThaiShaping, ReordersSaraAmStackOntoHighTier) {
  EXPECT_EQ(codepoints(ThaiShaping::shapeUtf8("อ่ำ")),
            (std::vector<uint32_t>{0x0E2D, 0x0E4D, 0xF70A, 0x0E32}));
}
