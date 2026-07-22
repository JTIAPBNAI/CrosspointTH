#include "MarkdownParser.h"

namespace MarkdownParser {

uint8_t headingLevel(const std::string& line) {
  size_t hashes = 0;
  while (hashes < line.size() && line[hashes] == '#') ++hashes;
  return hashes >= 1 && hashes <= 6 && hashes < line.size() && line[hashes] == ' ' ? hashes : 0;
}

std::string transformBlock(const std::string& in, const bool firstSegment) {
  size_t start = 0;
  std::string prefix;
  if (firstSegment) {
    const uint8_t level = headingLevel(in);
    if (level != 0) {
      start = level + 1;
    } else if (in.size() >= 2 && (in[0] == '-' || in[0] == '*' || in[0] == '+') && in[1] == ' ') {
      start = 2;
      prefix = "\xE2\x80\xA2 ";  // U+2022 bullet
    } else {
      size_t digits = 0;
      while (digits < in.size() && in[digits] >= '0' && in[digits] <= '9') ++digits;
      if (digits > 0 && digits + 1 < in.size() && in[digits] == '.' && in[digits + 1] == ' ') {
        prefix.assign(in, 0, digits + 1);
        prefix += ' ';
        start = digits + 2;
      } else if (in.size() >= 2 && in[0] == '>' && in[1] == ' ') {
        prefix = "\xE2\x94\x82 ";  // U+2502 quote bar
        start = 2;
      }
    }
  }
  prefix.append(in, start, std::string::npos);
  return prefix;
}

Inline parseInline(const std::string& in, const EpdFontFamily::Style baseStyle) {
  Inline parsed;
  parsed.runs.reserve(4);
  bool bold = (baseStyle & EpdFontFamily::BOLD) != 0;
  bool italic = (baseStyle & EpdFontFamily::ITALIC) != 0;
  bool code = false;

  const auto append = [&](const size_t start, const size_t length, Inline& result) {
    if (length == 0) return;
    auto style = baseStyle;
    if (bold || code) style = static_cast<EpdFontFamily::Style>(style | EpdFontFamily::BOLD);
    if (italic) style = static_cast<EpdFontFamily::Style>(style | EpdFontFamily::ITALIC);
    if (!result.runs.empty() && result.runs.back().style == style) {
      result.runs.back().text.append(in, start, length);
    } else {
      result.runs.push_back({in.substr(start, length), style});
    }
  };

  size_t spanStart = 0;
  size_t i = 0;
  while (i < in.size()) {
    size_t markerLength = 0;
    enum class Toggle : uint8_t { NONE, BOLD, ITALIC, CODE } toggle = Toggle::NONE;
    if (i + 1 < in.size() && ((in[i] == '*' && in[i + 1] == '*') || (in[i] == '_' && in[i + 1] == '_'))) {
      markerLength = 2;
      toggle = Toggle::BOLD;
    } else if (in[i] == '*' || in[i] == '_') {
      markerLength = 1;
      toggle = Toggle::ITALIC;
    } else if (in[i] == '`') {
      markerLength = 1;
      toggle = Toggle::CODE;
    }

    if (markerLength != 0) {
      append(spanStart, i - spanStart, parsed);
      if (toggle == Toggle::BOLD) bold = !bold;
      if (toggle == Toggle::ITALIC) italic = !italic;
      if (toggle == Toggle::CODE) code = !code;
      i += markerLength;
      spanStart = i;
      continue;
    }

    if (in[i] == '[') {
      const size_t close = in.find(']', i + 1);
      if (close != std::string::npos && close + 1 < in.size() && in[close + 1] == '(') {
        const size_t parenClose = in.find(')', close + 2);
        if (parenClose != std::string::npos) {
          append(spanStart, i - spanStart, parsed);
          append(i + 1, close - i - 1, parsed);
          i = parenClose + 1;
          spanStart = i;
          continue;
        }
      }
    }
    ++i;
  }
  append(spanStart, in.size() - spanStart, parsed);
  return parsed;
}

}  // namespace MarkdownParser
