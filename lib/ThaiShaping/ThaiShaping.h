#pragma once

#include <cstdint>
#include <string>

/// Contextual glyph shaping for Thai, following the WTT 2.0 rules described in
/// https://linux.thai.net/~thep/th-otf/shaping.html and the de-facto C90 PUA
/// convention (U+F700-F71A) for pre-composed variant glyphs:
///
///   - SARA AM (U+0E33) decomposes to NIKHAHIT + SARA AA; a preceding tone
///     mark is reordered after the NIKHAHIT.
///   - Upper vowels / NIKHAHIT / MAITAIKHU shift left over ascender
///     consonants (ป ฝ ฟ ฬ).
///   - Tone marks use their normal consonant tier when no upper vowel is
///     present, and the font's high/small tier above SARA I/II/UE/UEE;
///     ascender consonants additionally select left-shifted variants.
///   - YO YING (ญ) and THO THAN (ฐ) lose their descenders under below
///     vowels; below vowels drop under ฎ and ฏ.
///
/// The firmware renders glyph runs linearly (marks are zero-advance glyphs
/// positioned by their font-native bearings), so shaping is a pure
/// codepoint-substitution pass. Fonts must include the C90 PUA glyphs — see
/// scripts/thai_pua_patch.py. Missing PUA glyphs degrade gracefully: the
/// renderer skips absent glyphs, so shape only when the reader font family
/// provides them (SD-card Thai fonts do).
namespace ThaiShaping {

/// Returns the shaped UTF-8 string. Codepoints outside the Thai block pass
/// through untouched; the output may be longer than the input (SARA AM).
std::string shapeUtf8(std::string in);

}  // namespace ThaiShaping
