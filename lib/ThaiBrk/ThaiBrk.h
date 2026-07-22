#pragma once

#include <cstdint>
#include <string>
#include <vector>

/// Dictionary-based Thai word segmentation for line breaking.
///
/// Thai is written without spaces between words, so line-break opportunities
/// must be recovered with a dictionary. Longest-matching over a flash-resident
/// trie (ThaiDictData.h, built from the libthai word lists); runs at layout
/// time only, so lookup cost never touches the render loop.
namespace ThaiBrk {

/// Thai block, excluding currency sign U+0E3F which behaves like punctuation.
inline bool isThai(const uint32_t cp) { return cp >= 0x0E01 && cp <= 0x0E5B && cp != 0x0E3F; }

/// Dependent signs that must never start a line: upper/lower vowels, tone
/// marks, SARA AA/AM family and the repeat/abbreviation signs that bind left.
inline bool isThaiNoBreakBefore(const uint32_t cp) {
  return cp == 0x0E30                        // SARA A (binds to its consonant)
         || cp == 0x0E31                     // MAI HAN-AKAT
         || cp == 0x0E32 || cp == 0x0E33     // SARA AA, SARA AM
         || (cp >= 0x0E34 && cp <= 0x0E3A)   // SARA I..PHINTHU (upper/lower)
         || cp == 0x0E45 || cp == 0x0E46     // LAKKHANGYAO, MAIYAMOK
         || (cp >= 0x0E47 && cp <= 0x0E4E)   // MAITAIKHU..YAMAKKAN (tones etc.)
         || cp == 0x0E2F;                    // PAIYANNOI (abbreviation sign)
}

/// Leading vowels; a break directly after them would detach them from the
/// consonant they modify.
inline bool isThaiLeadingVowel(const uint32_t cp) { return cp >= 0x0E40 && cp <= 0x0E44; }

/// True if the UTF-8 string contains at least one Thai codepoint.
bool containsThai(const std::string& text);

/// Byte offsets (end-exclusive) inside `text` where a line break is allowed.
/// Word boundaries come from dictionary longest-matching; unknown runs fall
/// back to grapheme-cluster boundaries so OOV text still wraps. Returns an
/// empty vector when the text has no Thai or no break opportunity.
std::vector<size_t> wordBreakByteOffsets(const std::string& text);

}  // namespace ThaiBrk
