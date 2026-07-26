# crosspointTH changelog

## 1.5.0-th.1.1 — 2026-07-26

- แก้ปัญหาหน่วยความจำไม่เพียงพอเมื่อทำ index ไฟล์ Markdown ที่มีบรรทัดภาษาไทยและอังกฤษยาวมาก
- ลดการใช้หน่วยความจำซ้ำระหว่างการจัดหน้า Markdown เบื้องหลัง และเลื่อนการประมวลผลภาพตัวอักษร
  แบบละเอียดออกไปจนกว่าการทำ index จะเสร็จ
- ลดขนาดงานจัดหน้า Markdown ต่อรอบเพื่อให้คืนการควบคุมแก่ปุ่มเปลี่ยนหน้าเร็วขึ้นเมื่อพบบรรทัดยาว
- เพิ่มกรณีทดสอบสำหรับบรรทัดภาษาไทยและอังกฤษขนาดใหญ่
- เพิ่มคำแนะนำให้รอ Indexing ถึง 100% และลิงก์ XTEINK EPUB Optimizer สำหรับทั้งแปลง Markdown
  เป็น EPUB และ optimize ไฟล์ EPUB ที่มีอยู่แล้ว

## 1.5.0-th.1 — 2026-07-26

- Ported the supplied CrossPoint 1.5 reader changes onto the Thai edition while preserving Thai
  word breaking, contextual shaping, built-in glyph coverage, SD-font fallback, Markdown tables,
  EPUB table headings, and reading statistics.
- Added incremental EPUB section building, resumable partial indexes, background build heap gates,
  next-page glyph prewarming, and delayed indexing feedback for faster perceived book opening and
  page turns without violating the ESP32-C3 memory floor.
- Reworked TXT/Markdown pagination to open after the first page, continue indexing one page at a
  time in the background, show byte-based percentage progress and an estimated page count, preserve
  saved-page resume after cache invalidation, and avoid redundant full-line measurements.
- Added lazy EPUB image extraction and header probing so indexing no longer expands every image up
  front; image pixel-cache memory is released after each rendered page.
- Added the consolidated Text Settings screen with a live preview and Thai labels.
- Added full SD-font coverage indexes and CJK-aware UI fallback while retaining Thai fallback to
  matching built-in Noto families.
- Adopted the supplied TLS, Wi-Fi, font-loading, image-rendering, and heap-reclamation fixes.
- Advanced the Thai EPUB section-cache format to 133 so older Thai caches rebuild safely for the
  new lazy-image record layout.
- Hardened SD firmware installation with immediate flash read-back verification before changing
  `otadata`, and enabled the FreeInk Back+Up early-boot escape hatch for a known-good `ota_0` image.

## 1.4.1-th.3 — 2026-07-23

- Added GitHub-style Markdown pipe-table parsing for `.md` books.
- Rendered table headers and cells as stacked, labeled fields that fit the narrow Xteink display.
- Preserved escaped pipes and pipes inside inline code, and kept expanded rows together across page breaks.
- Replaced EPUB `Tab Row …, Cell …` labels with semantic `<th>` headings, with a concise
  `Column N` fallback when an EPUB provides no headings.
- Bumped the EPUB layout cache so existing books rebuild with the improved table labels.
- Updated the boot-screen edition label to `1.4.1-th.3`.

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
