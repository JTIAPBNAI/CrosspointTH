#include "MarkdownParser.h"

#include <algorithm>
#include <cctype>

namespace MarkdownParser {
namespace {

std::string trim(const std::string& value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) ++start;
  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
  return value.substr(start, end - start);
}

bool isDelimiterCell(const std::string& rawCell) {
  const std::string cell = trim(rawCell);
  if (cell.empty()) return false;

  size_t start = cell.front() == ':' ? 1 : 0;
  size_t end = cell.size();
  if (end > start && cell[end - 1] == ':') --end;
  if (end - start < 3) return false;
  for (size_t i = start; i < end; ++i) {
    if (cell[i] != '-') return false;
  }
  return true;
}

}  // namespace

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

bool parseTableRow(const std::string& line, TableRow& row) {
  row = {};
  const std::string stripped = trim(line);
  if (stripped.empty()) return false;

  const bool leadingPipe = stripped.front() == '|';
  const bool trailingPipe = stripped.back() == '|';
  row.hasOuterPipes = leadingPipe && trailingPipe;

  std::vector<std::string> cells;
  std::string cell;
  bool inCode = false;
  size_t separators = 0;
  for (size_t i = 0; i < stripped.size(); ++i) {
    const char ch = stripped[i];
    if (ch == '\\' && i + 1 < stripped.size() && stripped[i + 1] == '|') {
      cell += '|';
      ++i;
      continue;
    }
    if (ch == '`') {
      inCode = !inCode;
      cell += ch;
      continue;
    }
    if (ch == '|' && !inCode) {
      cells.push_back(trim(cell));
      cell.clear();
      ++separators;
      continue;
    }
    cell += ch;
  }
  cells.push_back(trim(cell));

  if (leadingPipe && !cells.empty() && cells.front().empty()) cells.erase(cells.begin());
  if (trailingPipe && !cells.empty() && cells.back().empty()) cells.pop_back();
  if (separators == 0 || cells.size() < 2) return false;

  row.cells = std::move(cells);
  return true;
}

bool isTableDelimiter(const TableRow& row) {
  return row.cells.size() >= 2 &&
         std::all_of(row.cells.begin(), row.cells.end(), [](const std::string& cell) { return isDelimiterCell(cell); });
}

std::vector<std::string> formatTableRow(const TableRow& row, const std::vector<std::string>& headers,
                                        const bool headerRow) {
  std::vector<std::string> fields;
  if (headerRow) {
    fields.reserve(row.cells.size());
    for (const auto& cell : row.cells) {
      if (!cell.empty()) fields.push_back("**" + cell + "**");
    }
    return fields;
  }

  const size_t count = std::max(row.cells.size(), headers.size());
  fields.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    const std::string value = i < row.cells.size() ? row.cells[i] : "";
    const std::string label = i < headers.size() ? headers[i] : "";
    if (!label.empty()) {
      fields.push_back("**" + label + ":** " + value);
    } else if (!value.empty()) {
      fields.push_back("\xE2\x80\xA2 " + value);  // U+2022 bullet
    }
  }
  return fields;
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
