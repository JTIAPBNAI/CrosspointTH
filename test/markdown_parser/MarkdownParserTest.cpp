#include <gtest/gtest.h>

#include <string>

#include "MarkdownParser.h"

TEST(MarkdownParser, ParsesThaiBoldAndLinksWithoutBreakingUtf8) {
  const auto parsed =
      MarkdownParser::parseInline("ภาษาไทย **ตัวหนา** และ [ลิงก์](https://example.com)", EpdFontFamily::REGULAR);
  ASSERT_EQ(parsed.runs.size(), 3u);
  EXPECT_EQ(parsed.runs[0].text, "ภาษาไทย ");
  EXPECT_EQ(parsed.runs[1].text, "ตัวหนา");
  EXPECT_EQ(parsed.runs[1].style, EpdFontFamily::BOLD);
  EXPECT_EQ(parsed.runs[2].text, " และ ลิงก์");
}

TEST(MarkdownParser, HandlesLongThaiParagraphAsOneRun) {
  std::string paragraph;
  paragraph.reserve(48 * 1024);
  for (int i = 0; i < 2048; ++i) paragraph += "ภาษาไทยไม่มีช่องว่าง";

  const auto parsed = MarkdownParser::parseInline(paragraph, EpdFontFamily::REGULAR);
  ASSERT_EQ(parsed.runs.size(), 1u);
  EXPECT_EQ(parsed.runs[0].text, paragraph);
}

TEST(MarkdownParser, TransformsHeadingsAndLists) {
  EXPECT_EQ(MarkdownParser::headingLevel("## หัวข้อ"), 2);
  EXPECT_EQ(MarkdownParser::transformBlock("## หัวข้อ", true), "หัวข้อ");
  EXPECT_EQ(MarkdownParser::transformBlock("- รายการ", true), "• รายการ");
  EXPECT_EQ(MarkdownParser::transformBlock("## หัวข้อ", false), "## หัวข้อ");
}
