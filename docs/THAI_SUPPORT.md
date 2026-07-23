# Thai support in crosspointTH

This document records the implementation, limitations, validation, and release safety policy for
the Thai community edition maintained by **JTIAPBN.Ai**.

## Rendering design

Thai text is treated as grapheme-like clusters. A base consonant may be followed by a lower or
upper vowel and one or more marks. The shaping layer maps stacked combinations to prepared glyphs
so tone marks do not float too high or collide with upper vowels. Line wrapping and justification
must only operate between clusters, never inside them.

When a selected SD-card font has no glyph for a Thai code point, `EpdFontFamily` resolves that glyph
from the size- and style-compatible built-in Noto family. Missing-glyph widths are not cached under
the requested Thai code point, so a replacement glyph cannot hide a valid fallback.

Thai word breaking uses an embedded dictionary. TXT/Markdown indexing segments each source line
once and reuses the resulting boundaries while fitting display lines. This avoids the repeated
prefix segmentation that made large Thai files appear frozen.

Development builds after `v1.4.1-th.2` recognize GitHub-style Markdown pipe tables. Because a rigid
grid is not readable on the narrow Xteink display, each row is rendered as stacked fields with bold
column labels. Alignment markers are accepted, the separator row is hidden, escaped pipes and pipes
inside inline code remain part of their cell, and an expanded row is kept together when a page has
enough room. An unusually large row falls back to ordinary Markdown rendering so pagination can
continue without losing source text.

## Reader settings

`crosspointTH` intentionally reuses the upstream **Reader Line Spacing** and **Reader Paragraph
Alignment** settings rather than adding Thai-only persistent fields. This keeps settings compatible
with official firmware and gives EPUB, TXT, and Markdown a consistent control surface.

For justified Thai paragraphs, spare width may add at most one pixel at each dictionary word boundary.
This avoids the large visible holes produced by full-width expansion while keeping natural Sarabun
spacing close to upstream. No tracking or letter spacing is added inside a Thai cluster, and
paragraph-final lines are not justified.

## Automated coverage

Host tests in `test/thai_shaping/` cover representative combinations including:

- a single upper vowel: `อึ`
- upper vowel plus tone: `อึ่`, `ปึ้`
- multiple upper marks: `อื้อ`
- tone plus following vowel: `อ่ำ`

Release candidates must also be checked on a physical X3 or X4 with a Thai corpus containing long
paragraphs, mixed Thai/Latin text, Markdown headings/lists, and an SD font that lacks Thai glyphs.

## Known limitations

- Dictionary breaking can be ambiguous for names, new words, and domain-specific compounds.
- Font fallback preserves readable Thai but may visually differ from the selected Latin font.
- Markdown support, including pipe tables, is a lightweight reader subset rather than a complete
  CommonMark or GitHub Flavored Markdown implementation.
- Automated host tests validate shaping decisions but cannot fully replace inspection on an e-ink panel.

## Physical validation log

- 2026-07-22: Thai EPUB content rendered and the device successfully flashed back from the
  crosspointTH pre-release to official CrossPoint firmware. This validates the documented rollback
  path for the tested unit; every new firmware artifact still requires its own safety gate and
  device check.
- 2026-07-22: Testing identified missing Thai glyphs in the upstream 8-point status font, excessive
  C90 high-tone height, wide Thai justification gaps, and slow Markdown indexing. These findings are
  addressed in `v1.4.1-th.2` and remain pending a second on-device visual/performance check.

## Release safety checklist

Before every production firmware build:

1. Confirm the upstream base commit and FreeInk SDK submodule commit.
2. Confirm there are no changes to `partitions.csv`, bootloader/platform code, HAL, board configuration,
   power management, display drivers, OTA boot switching, or firmware flashing code.
3. Run all host tests and require every test to pass.
4. Build the unchanged upstream `gh_release` environment.
5. Verify the output is an ESP32-C3 DIO app image with a valid checksum/hash.
6. Verify its size is below the app partition size in `partitions.csv` with a documented margin.
7. Copy the exact verified image to `crosspointTH-firmware.bin` and publish its byte size and SHA-256.
8. Keep the official upstream recovery link and rollback instructions in the release notes.

Run `bash scripts/check_release_safety.sh` before a local production build. GitHub release and
release-candidate workflows run the same gate automatically. The recorded baselines are stored in
`UPSTREAM_BASE_COMMIT` and `FREEINK_SDK_BASE_COMMIT`; changing either requires an explicit upstream
review, not a routine version bump.

The tag workflow follows this policy: it publishes `crosspointTH-firmware.bin` plus its SHA-256 and
debug symbols, but does not package bootloader or partition-table binaries for end users.

If any hardware-sensitive file differs, do not describe the image as hardware-safe until the change
has received separate code review and physical-device recovery testing.
