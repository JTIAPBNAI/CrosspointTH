# crosspointTH changelog

## Unreleased

- Added GitHub-style Markdown pipe-table parsing for `.md` books.
- Rendered table headers and cells as stacked, labeled fields that fit the narrow Xteink display.
- Preserved escaped pipes and pipes inside inline code, and kept expanded rows together across page breaks.

## 1.4.1-th.2 — 2026-07-22

- Added Thai glyphs to the 8-point status-bar font so Thai book and file titles no longer render as replacement diamonds.
- Calibrated C90 high-tone variants downward by font scale so tone marks above upper vowels remain clear without floating an extra tier.
- Capped Thai justification expansion at one pixel per dictionary boundary to preserve natural Sarabun spacing.
- Replaced the byte-at-a-time Markdown inline parser with a span-based parser and yielded during very long paragraph wrapping.
- Added regression coverage for long Thai Markdown paragraphs and the high-tone vertical adjustment.
- Added lightweight EPUB reading statistics with per-book and all-book totals.
- Debounced reading-stat writes to reader exit and ignored idle page dwell over ten minutes.
- Added a Thai-only, text-only project README with direct pre-release download links.
- Documented the CrossInk feature review and the features intentionally excluded for flash/RAM/performance safety.
- Recorded a successful physical rollback from crosspointTH to official CrossPoint firmware.

## 1.4.1-th.1 — 2026-07-22

- Added Thai UI translation and built-in Thai glyph coverage.
- Added fallback from SD-card fonts without Thai glyphs to compatible built-in Noto fonts.
- Added Thai dictionary word breaking for EPUB, TXT, and Markdown.
- Added Thai shaping for upper vowels, tone marks, and stacked-mark combinations.
- Reduced Thai TXT/Markdown indexing work by reusing per-line segmentation results.
- Added Markdown headings, bold, italic, inline code, lists, block quotes, and link text.
- Applied the existing reader line-spacing and paragraph-alignment settings to TXT/Markdown.
- Added Thai-aware justification at dictionary word boundaries without splitting glyph clusters.
- Added host tests for Thai shaping and cluster behavior.
- Branded user-visible edition screens as `crosspointTH`, maintained by `JTIAPBN.Ai`.
