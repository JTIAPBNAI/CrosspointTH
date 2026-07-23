#pragma once

#include <cctype>
#include <string>
#include <vector>

namespace epub_table {

inline std::string normalizeHeaderText(const std::string& text) {
  std::string normalized;
  normalized.reserve(text.size());
  bool pendingSpace = false;

  for (const unsigned char ch : text) {
    if (std::isspace(ch)) {
      pendingSpace = !normalized.empty();
      continue;
    }
    if (pendingSpace) {
      normalized.push_back(' ');
      pendingSpace = false;
    }
    normalized.push_back(static_cast<char>(ch));
  }

  return normalized;
}

inline std::string cellLabel(const std::vector<std::string>& headers, const size_t columnIndex) {
  if (columnIndex < headers.size() && !headers[columnIndex].empty()) {
    return headers[columnIndex] + ":";
  }
  return "Column " + std::to_string(columnIndex + 1) + ":";
}

}  // namespace epub_table
