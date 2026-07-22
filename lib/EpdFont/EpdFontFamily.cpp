#include "EpdFontFamily.h"

#include <cstddef>

const EpdFont* EpdFontFamily::getFont(const Style style) const {
  // Extract font style bits; render-time overlay bits do not affect font selection.
  const bool hasBold = (style & BOLD) != 0;
  const bool hasItalic = (style & ITALIC) != 0;

  if (hasBold && hasItalic) {
    if (boldItalic) return boldItalic;
    if (bold) return bold;
    if (italic) return italic;
  } else if (hasBold && bold) {
    return bold;
  } else if (hasItalic && italic) {
    return italic;
  }

  return regular;
}

void EpdFontFamily::getTextDimensions(const char* string, int* w, int* h, const Style style) const {
  getFont(style)->getTextDimensions(string, w, h);
}

const EpdFontData* EpdFontFamily::getData(const Style style) const { return getFont(style)->data; }

const EpdGlyph* EpdFontFamily::getGlyph(const uint32_t cp, const Style style) const {
  return getFont(style)->getGlyph(cp);
}

int8_t EpdFontFamily::getKerning(const uint32_t leftCp, const uint32_t rightCp, const Style style) const {
  return getFont(style)->getKerning(leftCp, rightCp);
}

uint32_t EpdFontFamily::applyLigatures(const uint32_t cp, const char*& text, const Style style) const {
  return getFont(style)->applyLigatures(cp, text);
}

namespace {

constexpr bool isThaiOrThaiPua(const uint32_t cp) {
  return (cp >= 0x0E01 && cp <= 0x0E5B) || (cp >= 0xF700 && cp <= 0xF71A);
}

// Builtin Thai-capable fallback families by reader point size. Static storage
// only holds pointers to main.cpp's global font objects — no DRAM beyond the
// table itself.
constexpr uint8_t FALLBACK_SIZES[] = {12, 14, 16, 18};
const EpdFontFamily* thaiFallbacks[4] = {nullptr, nullptr, nullptr, nullptr};

}  // namespace

EpdFontFamily::GlyphSource EpdFontFamily::resolveGlyph(const uint32_t cp, const Style style) const {
  const EpdFont* font = getFont(style);
  if (const EpdGlyph* glyph = font->getGlyphNoReplacement(cp)) {
    return {glyph, font->data};
  }
  if (thaiFallback && isThaiOrThaiPua(cp)) {
    // Fallback family is builtin (never has its own fallback), so this
    // recursion is at most one level deep.
    const GlyphSource fromFallback = thaiFallback->resolveGlyph(cp, style);
    if (fromFallback.glyph) return fromFallback;
  }
  return {font->getGlyph(cp), font->data};  // U+FFFD replacement (or nullptr)
}

void EpdFontFamily::registerBuiltinThaiFallback(const uint8_t pointSize, const EpdFontFamily* family) {
  for (size_t i = 0; i < 4; ++i) {
    if (FALLBACK_SIZES[i] == pointSize) {
      thaiFallbacks[i] = family;
      return;
    }
  }
}

const EpdFontFamily* EpdFontFamily::builtinThaiFallback(const uint8_t pointSize) {
  // Exact size first, then nearest registered size so unusual SD font sizes
  // still get a usable fallback.
  size_t best = 0;
  int bestDelta = 1000;
  for (size_t i = 0; i < 4; ++i) {
    if (!thaiFallbacks[i]) continue;
    const int delta = (pointSize > FALLBACK_SIZES[i]) ? pointSize - FALLBACK_SIZES[i] : FALLBACK_SIZES[i] - pointSize;
    if (delta < bestDelta) {
      bestDelta = delta;
      best = i;
    }
  }
  return (bestDelta == 1000) ? nullptr : thaiFallbacks[best];
}
