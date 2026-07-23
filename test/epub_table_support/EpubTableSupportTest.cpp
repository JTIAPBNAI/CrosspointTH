#include <gtest/gtest.h>

#include "EpubTableSupport.h"

TEST(EpubTableSupport, NormalizesHeaderWhitespace) {
  EXPECT_EQ(epub_table::normalizeHeaderText("  Main\n  goal\t(หลัก)  "), "Main goal (หลัก)");
}

TEST(EpubTableSupport, PreservesUtf8HeaderText) {
  EXPECT_EQ(epub_table::normalizeHeaderText(" เป้าหมายหลัก "), "เป้าหมายหลัก");
}

TEST(EpubTableSupport, UsesSemanticColumnHeader) {
  const std::vector<std::string> headers = {"แนวคิด", "เป้าหมาย"};
  EXPECT_EQ(epub_table::cellLabel(headers, 1), "เป้าหมาย:");
}

TEST(EpubTableSupport, FallsBackToReadableColumnNumber) {
  const std::vector<std::string> headers = {"แนวคิด"};
  EXPECT_EQ(epub_table::cellLabel(headers, 2), "Column 3:");
}
