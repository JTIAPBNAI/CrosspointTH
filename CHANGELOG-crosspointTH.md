# crosspointTH changelog

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
