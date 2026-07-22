#pragma once
#include "EpdFont.h"

class EpdFontFamily {
 public:
  // Bitmask of text style flags carried per-word through layout and serialized in page cache.
  // Bits 0-1 select the font variant (BOLD/ITALIC); bits 2-5 are decoration/positioning overlays
  // applied at render time without changing the underlying font. getFont() ignores all bits
  // above bit 1 so decorations compose freely with bold/italic (e.g. BOLD | UNDERLINE | SUP).
  enum Style : uint8_t {
    REGULAR = 0,
    BOLD = 1,
    ITALIC = 2,
    BOLD_ITALIC = 3,
    UNDERLINE = 4,      // drawn as a line below baseline by TextBlock::render()
    STRIKETHROUGH = 8,  // drawn as a line through midline by TextBlock::render()
    SUP = 16,           // superscript: glyph scaled 50%, raised ~40% of ascender
    SUB = 32,           // subscript: glyph scaled 50%, lowered ~25% of ascender
  };
  static constexpr uint8_t TEXT_DECORATION_MASK = static_cast<uint8_t>(UNDERLINE | STRIKETHROUGH);

  explicit EpdFontFamily(const EpdFont* regular, const EpdFont* bold = nullptr, const EpdFont* italic = nullptr,
                         const EpdFont* boldItalic = nullptr)
      : regular(regular), bold(bold), italic(italic), boldItalic(boldItalic) {}
  ~EpdFontFamily() = default;
  void getTextDimensions(const char* string, int* w, int* h, Style style = REGULAR) const;
  const EpdFontData* getData(Style style = REGULAR) const;
  const EpdGlyph* getGlyph(uint32_t cp, Style style = REGULAR) const;

  /// Glyph together with the EpdFontData that owns its bitmap. The two must
  /// travel as a pair: GfxRenderer::getGlyphBitmap() resolves the bitmap
  /// relative to the owning font's data, so mixing a fallback glyph with this
  /// family's data would read garbage.
  struct GlyphSource {
    const EpdGlyph* glyph;
    const EpdFontData* data;
  };

  /// Resolves cp in this family first; when the codepoint is a Thai letter,
  /// mark, or C90 PUA variant that this family lacks (typical for SD-card
  /// fonts converted without Thai coverage), falls back to the registered
  /// builtin Thai-capable family of the same point size. Returns the U+FFFD
  /// replacement from this family when nothing covers cp.
  GlyphSource resolveGlyph(uint32_t cp, Style style = REGULAR) const;

  /// Per-instance fallback used by resolveGlyph (set for SD-card families).
  void setThaiFallback(const EpdFontFamily* family) { thaiFallback = family; }

  /// Registry of builtin Thai-capable families by point size (12/14/16/18),
  /// populated once at boot from main.cpp. Lookup returns the exact size or
  /// the nearest registered one; nullptr when none registered (OMIT_FONTS).
  static void registerBuiltinThaiFallback(uint8_t pointSize, const EpdFontFamily* family);
  static const EpdFontFamily* builtinThaiFallback(uint8_t pointSize);
  int8_t getKerning(uint32_t leftCp, uint32_t rightCp, Style style = REGULAR) const;
  uint32_t applyLigatures(uint32_t cp, const char*& text, Style style = REGULAR) const;
  static constexpr bool hasTextDecoration(const Style style) {
    return (static_cast<uint8_t>(style) & TEXT_DECORATION_MASK) != 0;
  }

 private:
  const EpdFont* regular;
  const EpdFont* bold;
  const EpdFont* italic;
  const EpdFont* boldItalic;
  const EpdFontFamily* thaiFallback = nullptr;

  const EpdFont* getFont(Style style) const;
};
