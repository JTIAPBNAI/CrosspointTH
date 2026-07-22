#include "ThaiBrk.h"

#include <Utf8.h>

#include "ThaiDictData.h"

namespace ThaiBrk {
namespace {

// Reads a 24-bit little-endian child offset byte-by-byte: the trie is packed,
// so wider loads would fault on the ESP32-C3 (RISC-V unaligned access).
inline uint32_t readChildOffset(const uint8_t* edge) {
  return static_cast<uint32_t>(edge[1]) | (static_cast<uint32_t>(edge[2]) << 8) |
         (static_cast<uint32_t>(edge[3]) << 16);
}

// Follows one trie edge. Returns the child node offset, or UINT32_MAX if the
// node has no edge for `ch`. Edges are sorted by ch — binary search.
uint32_t trieStep(const uint32_t nodeOffset, const uint8_t ch) {
  const uint8_t* node = thaiDict::TRIE + nodeOffset;
  uint32_t lo = 0;
  uint32_t hi = node[0] & 0x7F;
  const uint8_t* edges = node + 1;
  while (lo < hi) {
    const uint32_t mid = (lo + hi) / 2;
    const uint8_t edgeCh = edges[mid * 4];
    if (edgeCh == ch) return readChildOffset(edges + mid * 4);
    if (edgeCh < ch) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  return UINT32_MAX;
}

inline bool trieIsTerminal(const uint32_t nodeOffset) { return (thaiDict::TRIE[nodeOffset] & 0x80) != 0; }

struct ThaiChar {
  uint32_t cp;
  size_t endOffset;  // byte offset just past this codepoint
};

// True if a break is allowed between chars[i] and chars[i+1] by the
// script-structural rules alone (used for both dictionary boundaries and the
// out-of-vocabulary cluster fallback).
bool clusterBreakAllowed(const std::vector<ThaiChar>& chars, const size_t i) {
  const uint32_t cur = chars[i].cp;
  const uint32_t next = chars[i + 1].cp;
  if (isThaiLeadingVowel(cur)) return false;    // เ แ โ ใ ไ bind right
  if (isThaiNoBreakBefore(next)) return false;  // marks/สระ bind left
  // Keep digit runs together (Thai digits ๐-๙).
  if (cur >= 0x0E50 && cur <= 0x0E59 && next >= 0x0E50 && next <= 0x0E59) return false;
  return true;
}

}  // namespace

bool containsThai(const std::string& text) {
  const auto* ptr = reinterpret_cast<const unsigned char*>(text.c_str());
  while (*ptr) {
    if (isThai(utf8NextCodepoint(&ptr))) return true;
  }
  return false;
}

std::vector<size_t> wordBreakByteOffsets(const std::string& text) {
  std::vector<ThaiChar> chars;
  chars.reserve(text.size() / 3);  // Thai codepoints are 3 UTF-8 bytes
  bool hasThai = false;

  const auto* ptr = reinterpret_cast<const unsigned char*>(text.c_str());
  const auto* const start = ptr;
  while (*ptr) {
    const uint32_t cp = utf8NextCodepoint(&ptr);
    if (cp == 0) break;
    if (isThai(cp)) hasThai = true;
    chars.push_back({cp, static_cast<size_t>(ptr - start)});
  }
  if (!hasThai || chars.size() < 2) return {};

  std::vector<size_t> breaks;
  breaks.reserve(chars.size() / 3);

  size_t i = 0;
  while (i < chars.size()) {
    if (!isThai(chars[i].cp)) {
      // Non-Thai run: no internal break opportunities from us; allow a break
      // at the transition back into Thai.
      while (i < chars.size() && !isThai(chars[i].cp)) ++i;
      if (i > 0 && i < chars.size()) breaks.push_back(chars[i - 1].endOffset);
      continue;
    }

    // Longest dictionary match starting at i.
    uint32_t node = 0;
    size_t matchEnd = 0;  // index just past the longest terminal match
    for (size_t j = i; j < chars.size() && isThai(chars[j].cp); ++j) {
      const uint32_t cp = chars[j].cp;
      if (cp < 0x0E00 || cp > 0x0E7F) break;
      node = trieStep(node, static_cast<uint8_t>(cp - 0x0E00));
      if (node == UINT32_MAX) break;
      if (trieIsTerminal(node)) matchEnd = j + 1;
    }

    size_t wordEnd;
    if (matchEnd > i) {
      wordEnd = matchEnd;
      // A dictionary word must still not orphan trailing bound signs
      // (e.g. ๆ or ฯ directly after the word).
      while (wordEnd < chars.size() && isThaiNoBreakBefore(chars[wordEnd].cp)) ++wordEnd;
    } else {
      // Out of vocabulary: advance one grapheme cluster.
      wordEnd = i + 1;
      while (wordEnd < chars.size() && isThai(chars[wordEnd].cp) && !clusterBreakAllowed(chars, wordEnd - 1)) {
        ++wordEnd;
      }
    }

    if (wordEnd < chars.size()) {
      breaks.push_back(chars[wordEnd - 1].endOffset);
    }
    i = wordEnd;
  }

  return breaks;
}

}  // namespace ThaiBrk
