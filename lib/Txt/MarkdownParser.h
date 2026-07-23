#pragma once

#include <EpdFontFamily.h>

#include <cstdint>
#include <string>
#include <vector>

namespace MarkdownParser {

struct Run {
  std::string text;
  EpdFontFamily::Style style = EpdFontFamily::REGULAR;
};

struct Inline {
  std::vector<Run> runs;
};

struct TableRow {
  std::vector<std::string> cells;
  bool hasOuterPipes = false;
};

uint8_t headingLevel(const std::string& line);
std::string transformBlock(const std::string& in, bool firstSegment);

/// Parses a GitHub-style pipe-table row. Escaped pipes and pipes inside inline
/// code stay inside their cell. Returns false for ordinary prose with fewer
/// than two cells.
bool parseTableRow(const std::string& line, TableRow& row);

/// Returns true when every cell is a Markdown table delimiter such as `---`,
/// `:---`, `---:`, or `:---:`.
bool isTableDelimiter(const TableRow& row);

/// Converts a table row into narrow-screen-friendly stacked fields. Header
/// rows become bold field names; data rows use the supplied headers as labels.
std::vector<std::string> formatTableRow(const TableRow& row, const std::vector<std::string>& headers, bool headerRow);

/// Parses the small Markdown subset supported by the TXT reader. The parser
/// appends whole source spans instead of one UTF-8 byte at a time; this keeps
/// long Thai paragraphs linear rather than repeatedly reallocating a growing
/// string during page-index construction.
Inline parseInline(const std::string& in, EpdFontFamily::Style baseStyle);

}  // namespace MarkdownParser
