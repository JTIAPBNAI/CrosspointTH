#include "ThaiShaping.h"

#include <Utf8.h>

namespace ThaiShaping {
namespace {

constexpr uint32_t SARA_AM = 0x0E33;
constexpr uint32_t NIKHAHIT = 0x0E4D;
constexpr uint32_t SARA_AA = 0x0E32;

// Consonants whose ascender stem collides with upper marks: ป ฝ ฟ ฬ.
constexpr bool isAscenderBase(const uint32_t cp) {
  return cp == 0x0E1B || cp == 0x0E1D || cp == 0x0E1F || cp == 0x0E2C;
}

// Consonants whose descender collides with below vowels: ฎ ฏ.
constexpr bool isDescenderBase(const uint32_t cp) { return cp == 0x0E0E || cp == 0x0E0F; }

constexpr bool isBaseConsonant(const uint32_t cp) { return cp >= 0x0E01 && cp <= 0x0E2E; }

// Upper vowels + NIKHAHIT + MAITAIKHU (everything that occupies the first
// above-base tier).
constexpr bool isUpperVowel(const uint32_t cp) {
  return cp == 0x0E31 || (cp >= 0x0E34 && cp <= 0x0E37) || cp == 0x0E47 || cp == NIKHAHIT;
}

constexpr bool isBelowVowel(const uint32_t cp) { return cp >= 0x0E38 && cp <= 0x0E3A; }

// Tone marks + THANTHAKHAT: MAI EK..THANTHAKHAT (U+0E48-0E4C).
constexpr bool isToneMark(const uint32_t cp) { return cp >= 0x0E48 && cp <= 0x0E4C; }

// C90 PUA variant selectors. Return the input unchanged when no variant applies.

// Upper vowel shifted left for ascender bases.
constexpr uint32_t upperLeftVariant(const uint32_t cp) {
  if (cp >= 0x0E34 && cp <= 0x0E37) return 0xF701 + (cp - 0x0E34);
  if (cp == 0x0E31) return 0xF710;
  if (cp == NIKHAHIT) return 0xF711;
  if (cp == 0x0E47) return 0xF712;
  return cp;
}

// Tone mark variants. In Sarabun's OpenType shaping, the .small forms occupy
// the second-above-base tier (above SARA I/II/UE/UEE), while the normal forms
// sit directly above the consonant. C90 names these high variants differently
// across fonts, so keep the mapping explicit here.
constexpr uint32_t toneHighVariant(const uint32_t cp) { return 0xF70A + (cp - 0x0E48); }
constexpr uint32_t toneHighLeftVariant(const uint32_t cp) { return 0xF705 + (cp - 0x0E48); }
constexpr uint32_t toneLeftVariant(const uint32_t cp) { return 0xF713 + (cp - 0x0E48); }

// Below vowel lowered for descender bases (ฎ ฏ).
constexpr uint32_t belowLowVariant(const uint32_t cp) { return 0xF718 + (cp - 0x0E38); }

// Descender-less base substitutes for ญ ฐ.
constexpr uint32_t descenderlessBase(const uint32_t cp) {
  if (cp == 0x0E0D) return 0xF70F;  // ญ
  if (cp == 0x0E10) return 0xF700;  // ฐ
  return cp;
}

}  // namespace

std::string shapeUtf8(std::string in) {
  std::string out;
  out.reserve(in.size() + 8);

  // Cluster state, reset at each base consonant.
  uint32_t base = 0;
  bool upperVowelSeen = false;
  // Byte position in `out` where the pending tone mark of the current cluster
  // starts, for SARA AM reordering. npos when none.
  size_t toneStartInOut = std::string::npos;
  uint32_t toneOriginalCp = 0;  // unshaped tone mark of the current cluster

  // Position of the current cluster's base consonant in `out`, so a later
  // below vowel can rewrite ญ/ฐ to the descender-less form in place. The
  // substituted PUA glyph is 3 UTF-8 bytes, same as the original, so an
  // in-place overwrite is safe.
  size_t baseStartInOut = std::string::npos;

  const auto* ptr = reinterpret_cast<const unsigned char*>(in.c_str());
  uint32_t cp;
  while ((cp = utf8NextCodepoint(&ptr))) {
    if (isBaseConsonant(cp)) {
      base = cp;
      upperVowelSeen = false;
      toneStartInOut = std::string::npos;
      baseStartInOut = out.size();
      utf8AppendCodepoint(cp, out);
      continue;
    }

    if (cp == SARA_AM) {
      // Decompose; NIKHAHIT moves before any pending tone mark of this cluster.
      const uint32_t nikhahit = isAscenderBase(base) ? upperLeftVariant(NIKHAHIT) : NIKHAHIT;
      if (toneStartInOut != std::string::npos) {
        // The tone was shaped for the "no upper vowel" case, but it now rides
        // above the NIKHAHIT — rewrite it back to the high form first. All
        // involved codepoints encode to 3 UTF-8 bytes, so in-place replace.
        const uint32_t highTone =
            isAscenderBase(base) ? toneHighLeftVariant(toneOriginalCp) : toneHighVariant(toneOriginalCp);
        std::string repl;
        utf8AppendCodepoint(highTone, repl);
        out.replace(toneStartInOut, repl.size(), repl);
        std::string nik;
        utf8AppendCodepoint(nikhahit, nik);
        out.insert(toneStartInOut, nik);
      } else {
        utf8AppendCodepoint(nikhahit, out);
      }
      utf8AppendCodepoint(SARA_AA, out);
      // SARA AA ends the syllable's mark stack.
      upperVowelSeen = false;
      toneStartInOut = std::string::npos;
      base = 0;
      continue;
    }

    if (isUpperVowel(cp)) {
      upperVowelSeen = true;
      if (isAscenderBase(base)) cp = upperLeftVariant(cp);
      utf8AppendCodepoint(cp, out);
      continue;
    }

    if (isBelowVowel(cp)) {
      if (base == 0x0E0D || base == 0x0E10) {
        // Rewrite ญ/ฐ already emitted at baseStartInOut to its descender-less
        // variant (same UTF-8 length, verified 3 bytes for both ranges).
        std::string repl;
        utf8AppendCodepoint(descenderlessBase(base), repl);
        if (baseStartInOut != std::string::npos && baseStartInOut + repl.size() <= out.size()) {
          out.replace(baseStartInOut, repl.size(), repl);
        }
      } else if (isDescenderBase(base)) {
        cp = belowLowVariant(cp);
      }
      utf8AppendCodepoint(cp, out);
      continue;
    }

    if (isToneMark(cp)) {
      uint32_t shaped = cp;
      if (upperVowelSeen) {
        // Rides on the second tier above the upper vowel. The regular tone
        // glyph belongs to the first tier and visibly collides/floats against
        // SARA UE/UEE when used here.
        shaped = isAscenderBase(base) ? toneHighLeftVariant(cp) : toneHighVariant(cp);
      } else {
        // With no upper vowel the normal tone height is correct; only move it
        // left for tall ascender bases (ป ฝ ฟ ฬ).
        shaped = isAscenderBase(base) ? toneLeftVariant(cp) : cp;
      }
      toneStartInOut = out.size();
      toneOriginalCp = cp;
      utf8AppendCodepoint(shaped, out);
      continue;
    }

    // Leading vowels, SARA A/AA, digits, punctuation, non-Thai: pass through.
    // SARA AA also terminates the mark cluster.
    if (cp == SARA_AA || cp == 0x0E30) {
      base = 0;
      upperVowelSeen = false;
      toneStartInOut = std::string::npos;
    }
    utf8AppendCodepoint(cp, out);
  }

  return out;
}

}  // namespace ThaiShaping
