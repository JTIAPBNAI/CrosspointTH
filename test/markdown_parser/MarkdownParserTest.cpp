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

TEST(MarkdownParser, ParsesPipeTableRowsAndDelimiterAlignment) {
  MarkdownParser::TableRow header;
  ASSERT_TRUE(MarkdownParser::parseTableRow("| รายการ | จำนวน | ราคา |", header));
  ASSERT_EQ(header.cells.size(), 3u);
  EXPECT_EQ(header.cells[0], "รายการ");
  EXPECT_EQ(header.cells[1], "จำนวน");
  EXPECT_EQ(header.cells[2], "ราคา");
  EXPECT_TRUE(header.hasOuterPipes);

  MarkdownParser::TableRow delimiter;
  ASSERT_TRUE(MarkdownParser::parseTableRow("| :--- | ---: | :---: |", delimiter));
  EXPECT_TRUE(MarkdownParser::isTableDelimiter(delimiter));
}

TEST(MarkdownParser, KeepsEscapedAndInlineCodePipesInsideCells) {
  MarkdownParser::TableRow row;
  ASSERT_TRUE(MarkdownParser::parseTableRow(R"(| A \| B | `x|y` | ปกติ |)", row));
  ASSERT_EQ(row.cells.size(), 3u);
  EXPECT_EQ(row.cells[0], "A | B");
  EXPECT_EQ(row.cells[1], "`x|y`");
  EXPECT_EQ(row.cells[2], "ปกติ");
}

TEST(MarkdownParser, FormatsTableAsStackedLabeledFields) {
  MarkdownParser::TableRow row;
  ASSERT_TRUE(MarkdownParser::parseTableRow("| กาแฟ | 18 กรัม | 120 บาท |", row));
  const auto fields = MarkdownParser::formatTableRow(row, {"รายการ", "จำนวน", "ราคา"}, false);
  ASSERT_EQ(fields.size(), 3u);
  EXPECT_EQ(fields[0], "**รายการ:** กาแฟ");
  EXPECT_EQ(fields[1], "**จำนวน:** 18 กรัม");
  EXPECT_EQ(fields[2], "**ราคา:** 120 บาท");
}

TEST(MarkdownParser, DoesNotTreatOrdinaryTextAsTable) {
  MarkdownParser::TableRow row;
  EXPECT_FALSE(MarkdownParser::parseTableRow("ข้อความธรรมดา", row));

  // A pipe-separated line is syntactically parseable, but without outer
  // pipes the reader only treats it as a table inside a confirmed table block.
  ASSERT_TRUE(MarkdownParser::parseTableRow("A | B", row));
  EXPECT_FALSE(row.hasOuterPipes);

  ASSERT_TRUE(MarkdownParser::parseTableRow("| -- | --- |", row));
  EXPECT_FALSE(MarkdownParser::isTableDelimiter(row));
}
