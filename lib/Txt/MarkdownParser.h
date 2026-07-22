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

uint8_t headingLevel(const std::string& line);
std::string transformBlock(const std::string& in, bool firstSegment);

/// Parses the small Markdown subset supported by the TXT reader. The parser
/// appends whole source spans instead of one UTF-8 byte at a time; this keeps
/// long Thai paragraphs linear rather than repeatedly reallocating a growing
/// string during page-index construction.
Inline parseInline(const std::string& in, EpdFontFamily::Style baseStyle);

}  // namespace MarkdownParser
