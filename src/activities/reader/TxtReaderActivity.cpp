#include "TxtReaderActivity.h"

#include <BidiUtils.h>
#include <FontCacheManager.h>
#include <FsHelpers.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include <MarkdownParser.h>
#include <Serialization.h>
#include <ThaiBrk.h>
#include <ThaiShaping.h>
#include <Utf8.h>

#include <cmath>
#include <limits>

#include "CrossPointSettings.h"
#include "CrossPointState.h"
#include "MappedInputManager.h"
#include "ProgressFile.h"
#include "ReaderUtils.h"
#include "RecentBooksStore.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr size_t CHUNK_SIZE = 8 * 1024;  // 8KB chunk for reading
constexpr size_t NO_TABLE_HEADER = std::numeric_limits<size_t>::max();
constexpr uint32_t NO_TABLE_HEADER_CACHE = std::numeric_limits<uint32_t>::max();
constexpr int MAX_THAI_JUSTIFY_GAP_PX = 1;
// Cache file magic and version
constexpr uint32_t CACHE_MAGIC = 0x54585449;  // "TXTI"
// Increment when cache format changes. Thai fork: offset +100 from upstream
// (Thai-aware wrapping and Markdown transforms change page break offsets), so
// page-index caches never validate across firmware swaps in either direction.
constexpr uint8_t CACHE_VERSION = 106;
}  // namespace

void TxtReaderActivity::onEnter() {
  Activity::onEnter();

  if (!txt) {
    return;
  }

  ReaderUtils::applyOrientation(renderer, SETTINGS.orientation);

  txt->setupCacheDir();

  isMarkdown = FsHelpers::hasMarkdownExtension(txt->getPath());

  // Save current txt as last opened file and add to recent books
  auto filePath = txt->getPath();
  auto fileName = filePath.substr(filePath.rfind('/') + 1);
  APP_STATE.openEpubPath = filePath;
  APP_STATE.saveToFile();
  RECENT_BOOKS.addBook(filePath, fileName, "", "");

  // Trigger first update
  requestUpdate();
}

void TxtReaderActivity::onExit() {
  Activity::onExit();

  // Reset orientation back to portrait for the rest of the UI
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);

  pageOffsets.clear();
  pageTableHeaderOffsets.clear();
  currentPageLines.clear();
  APP_STATE.readerActivityLoadCount = 0;
  APP_STATE.saveToFile();
  txt.reset();
}

void TxtReaderActivity::loop() {
  if (ReaderUtils::handleBackNavigation(mappedInput, activityManager, txt ? txt->getPath().c_str() : "",
                                        {this, [](void* ctx) { static_cast<TxtReaderActivity*>(ctx)->onGoHome(); }})) {
    return;
  }

  const auto touch = ReaderUtils::detectTouchPageTurn(renderer, mappedInput);
  auto [prevTriggered, nextTriggered, fromTilt] = ReaderUtils::detectPageTurn(mappedInput);
  prevTriggered = prevTriggered || touch.prev;
  nextTriggered = nextTriggered || touch.next;
  if (!prevTriggered && !nextTriggered) {
    return;
  }

  if (prevTriggered && currentPage > 0) {
    currentPage--;
    requestUpdate();
  } else if (nextTriggered) {
    if (currentPage < totalPages - 1) {
      currentPage++;
      requestUpdate();
    } else {
      onGoHome();
    }
  }
}

void TxtReaderActivity::initializeReader() {
  if (initialized) {
    return;
  }

  // Store current settings for cache validation
  cachedFontId = SETTINGS.getReaderFontId();
  cachedScreenMargin = SETTINGS.screenMargin;
  cachedParagraphAlignment = SETTINGS.paragraphAlignment;
  cachedLineSpacing = SETTINGS.lineSpacing;
  cachedLineCompression = SETTINGS.getReaderLineCompression();

  // Calculate viewport dimensions
  renderer.getOrientedViewableTRBL(&cachedOrientedMarginTop, &cachedOrientedMarginRight, &cachedOrientedMarginBottom,
                                   &cachedOrientedMarginLeft);
  cachedOrientedMarginTop += cachedScreenMargin;
  cachedOrientedMarginLeft += cachedScreenMargin;
  cachedOrientedMarginRight += cachedScreenMargin;
  cachedOrientedMarginBottom +=
      std::max(cachedScreenMargin, static_cast<uint8_t>(UITheme::getInstance().getStatusBarHeight()));

  viewportWidth = renderer.getScreenWidth() - cachedOrientedMarginLeft - cachedOrientedMarginRight;
  viewportHeight = renderer.getScreenHeight() - cachedOrientedMarginTop - cachedOrientedMarginBottom;
  const int lineHeight =
      std::max(1, static_cast<int>(std::ceil(renderer.getLineHeight(cachedFontId) * cachedLineCompression)));

  linesPerPage = viewportHeight / lineHeight;
  if (linesPerPage < 1) linesPerPage = 1;

  LOG_DBG("TRS", "Viewport: %dx%d, lines per page: %d", viewportWidth, viewportHeight, linesPerPage);

  // Try to load cached page index first
  if (!loadPageIndexCache()) {
    // Cache not found, build page index
    buildPageIndex();
    // Save to cache for next time
    savePageIndexCache();
  }

  // Load saved progress
  loadProgress();

  initialized = true;
}

void TxtReaderActivity::buildPageIndex() {
  pageOffsets.clear();
  pageTableHeaderOffsets.clear();
  pageOffsets.push_back(0);  // First page starts at offset 0
  pageTableHeaderOffsets.push_back(NO_TABLE_HEADER);

  size_t offset = 0;
  size_t tableHeaderOffset = NO_TABLE_HEADER;
  const size_t fileSize = txt->getFileSize();

  LOG_DBG("TRS", "Building page index for %zu bytes...", fileSize);

  GUI.drawPopup(renderer, tr(STR_INDEXING));

  while (offset < fileSize) {
    std::vector<DisplayLine> tempLines;
    size_t nextOffset = offset;
    size_t nextTableHeaderOffset = tableHeaderOffset;

    if (!loadPageAtOffset(offset, tempLines, nextOffset, tableHeaderOffset, nextTableHeaderOffset)) {
      break;
    }

    if (nextOffset <= offset) {
      // No progress made, avoid infinite loop
      break;
    }

    offset = nextOffset;
    tableHeaderOffset = nextTableHeaderOffset;
    if (offset < fileSize) {
      pageOffsets.push_back(offset);
      pageTableHeaderOffsets.push_back(tableHeaderOffset);
    }

    // Yield to other tasks periodically
    if (pageOffsets.size() % 20 == 0) {
      vTaskDelay(1);
    }
  }

  totalPages = pageOffsets.size();
  LOG_DBG("TRS", "Built page index: %d pages", totalPages);
}

namespace {

int builtinReaderFontId(const uint8_t size) {
  static constexpr int serif[] = {NOTOSERIF_12_FONT_ID, NOTOSERIF_14_FONT_ID, NOTOSERIF_16_FONT_ID,
                                  NOTOSERIF_18_FONT_ID};
  static constexpr int sans[] = {NOTOSANS_12_FONT_ID, NOTOSANS_14_FONT_ID, NOTOSANS_16_FONT_ID, NOTOSANS_18_FONT_ID};
  const uint8_t clamped = std::min<uint8_t>(size, 3);
  return SETTINGS.fontFamily == CrossPointSettings::NOTOSANS ? sans[clamped] : serif[clamped];
}

int mdHeadingFontId(const uint8_t level, const int bodyFontId) {
  if (level == 0) return bodyFontId;
  const uint8_t base = std::min<uint8_t>(SETTINGS.fontSize, 3);
  const uint8_t bump = level == 1 ? 2 : (level <= 3 ? 1 : 0);
  return builtinReaderFontId(std::min<uint8_t>(base + bump, 3));
}

}  // namespace

bool TxtReaderActivity::readMarkdownTableHeaders(const size_t offset, std::vector<std::string>& headers) {
  headers.clear();
  const size_t fileSize = txt->getFileSize();
  if (offset == NO_TABLE_HEADER || offset >= fileSize) return false;

  const size_t readSize = std::min(CHUNK_SIZE, fileSize - offset);
  std::vector<uint8_t> buffer(readSize + 1);
  if (!txt->readContent(buffer.data(), offset, readSize)) return false;
  buffer[readSize] = '\0';

  size_t lineEnd = 0;
  while (lineEnd < readSize && buffer[lineEnd] != '\n') ++lineEnd;
  if (lineEnd == readSize && offset + lineEnd < fileSize) return false;
  if (lineEnd > 0 && buffer[lineEnd - 1] == '\r') --lineEnd;

  const std::string line(reinterpret_cast<const char*>(buffer.data()), lineEnd);
  MarkdownParser::TableRow row;
  if (!MarkdownParser::parseTableRow(line, row) || MarkdownParser::isTableDelimiter(row)) return false;
  headers = std::move(row.cells);
  return !headers.empty();
}

TxtReaderActivity::TableRowResult TxtReaderActivity::appendMarkdownTableFields(const std::vector<std::string>& fields,
                                                                               std::vector<DisplayLine>& outLines,
                                                                               int& usedHeight) {
  const int lineAdvance =
      std::max(1, static_cast<int>(std::ceil(renderer.getLineHeight(cachedFontId) * cachedLineCompression)));
  std::vector<DisplayLine> pending;

  const auto makeDisplayLine = [&](const std::string& visual, const bool paragraphEnd) {
    const auto parsed = MarkdownParser::parseInline(visual, EpdFontFamily::REGULAR);
    const bool hasThai = ThaiBrk::containsThai(visual);

    DisplayLine display;
    display.fontId = cachedFontId;
    display.style = EpdFontFamily::REGULAR;
    display.advanceY = lineAdvance;
    display.paragraphEnd = paragraphEnd;
    display.runs.reserve(parsed.runs.size());
    for (const auto& run : parsed.runs) {
      if (!hasThai) {
        display.text += run.text;
        display.runs.push_back({run.text, static_cast<uint8_t>(run.style), false});
        continue;
      }

      const auto breakOffsets = ThaiBrk::wordBreakByteOffsets(run.text);
      size_t tokenStart = 0;
      for (const size_t breakOffset : breakOffsets) {
        if (breakOffset <= tokenStart || breakOffset > run.text.size()) continue;
        std::string token = ThaiShaping::shapeUtf8(run.text.substr(tokenStart, breakOffset - tokenStart));
        display.text += token;
        display.runs.push_back({std::move(token), static_cast<uint8_t>(run.style), true});
        ++display.thaiGapCount;
        tokenStart = breakOffset;
      }
      if (tokenStart < run.text.size()) {
        std::string token = ThaiShaping::shapeUtf8(run.text.substr(tokenStart));
        display.text += token;
        display.runs.push_back({std::move(token), static_cast<uint8_t>(run.style), false});
      }
    }
    return display;
  };

  const auto measure = [&](const std::string& visual) {
    const auto parsed = MarkdownParser::parseInline(visual, EpdFontFamily::REGULAR);
    const bool hasThai = ThaiBrk::containsThai(visual);
    int width = 0;
    for (const auto& run : parsed.runs) {
      const std::string shaped = hasThai ? ThaiShaping::shapeUtf8(run.text) : run.text;
      width += renderer.getTextAdvanceX(cachedFontId, shaped.c_str(), run.style);
    }
    return width;
  };

  for (const auto& field : fields) {
    if (field.empty()) continue;

    const bool hasThai = ThaiBrk::containsThai(field);
    const auto thaiBreaks = hasThai ? ThaiBrk::wordBreakByteOffsets(field) : std::vector<size_t>{};
    std::vector<size_t> charEnds;
    charEnds.reserve(field.size() / (hasThai ? 3 : 1));
    const auto* charPtr = reinterpret_cast<const unsigned char*>(field.c_str());
    const auto* const fieldStart = charPtr;
    while (*charPtr) {
      utf8NextCodepoint(&charPtr);
      charEnds.push_back(static_cast<size_t>(charPtr - fieldStart));
    }

    size_t fieldBytePos = 0;
    std::string remaining = field;
    const size_t fieldFirstLine = pending.size();
    while (!remaining.empty()) {
      if (measure(remaining) <= viewportWidth) {
        pending.push_back(makeDisplayLine(remaining, true));
        fieldBytePos = field.size();
        remaining.clear();
        break;
      }

      const auto firstEnd = std::upper_bound(charEnds.begin(), charEnds.end(), fieldBytePos);
      size_t low = static_cast<size_t>(firstEnd - charEnds.begin());
      size_t high = charEnds.size();
      size_t fittingEnd = fieldBytePos;
      while (low < high) {
        const size_t mid = low + (high - low) / 2;
        const size_t candidateEnd = charEnds[mid];
        if (measure(remaining.substr(0, candidateEnd - fieldBytePos)) <= viewportWidth) {
          fittingEnd = candidateEnd;
          low = mid + 1;
        } else {
          high = mid;
        }
      }

      size_t fittingBytes = fittingEnd > fieldBytePos ? fittingEnd - fieldBytePos : 0;
      if (fittingBytes == 0 && firstEnd != charEnds.end()) fittingBytes = *firstEnd - fieldBytePos;

      size_t breakPos = 0;
      if (fittingBytes > 0) {
        const size_t spacePos = remaining.rfind(' ', fittingBytes - 1);
        if (spacePos != std::string::npos && spacePos > 0) breakPos = spacePos;
      }
      if (hasThai) {
        for (const size_t absoluteBreak : thaiBreaks) {
          if (absoluteBreak <= fieldBytePos) continue;
          const size_t relativeBreak = absoluteBreak - fieldBytePos;
          if (relativeBreak > fittingBytes) break;
          if (relativeBreak > breakPos) breakPos = relativeBreak;
        }
      }
      if (breakPos == 0) breakPos = fittingBytes;
      if (breakPos == 0) return TableRowResult::TOO_TALL;

      pending.push_back(makeDisplayLine(remaining.substr(0, breakPos), false));
      size_t skipBytes = breakPos;
      if (breakPos < remaining.size() && remaining[breakPos] == ' ') ++skipBytes;
      fieldBytePos += skipBytes;
      remaining = remaining.substr(skipBytes);
    }
    if (pending.size() > fieldFirstLine) pending.back().paragraphEnd = true;
  }

  if (pending.empty()) return TableRowResult::ADDED;
  const int rowHeight = static_cast<int>(pending.size()) * lineAdvance;
  if (rowHeight > viewportHeight) return TableRowResult::TOO_TALL;
  if (!outLines.empty() && usedHeight + rowHeight > viewportHeight) return TableRowResult::DEFERRED;

  outLines.insert(outLines.end(), std::make_move_iterator(pending.begin()), std::make_move_iterator(pending.end()));
  usedHeight += rowHeight;
  return TableRowResult::ADDED;
}

bool TxtReaderActivity::loadPageAtOffset(size_t offset, std::vector<DisplayLine>& outLines, size_t& nextOffset,
                                         size_t tableHeaderOffset, size_t& nextTableHeaderOffset) {
  outLines.clear();
  const size_t fileSize = txt->getFileSize();

  if (offset >= fileSize) {
    return false;
  }

  // Read a chunk from file
  size_t chunkSize = std::min(CHUNK_SIZE, fileSize - offset);
  auto* buffer = static_cast<uint8_t*>(malloc(chunkSize + 1));
  if (!buffer) {
    LOG_ERR("TRS", "Failed to allocate %zu bytes", chunkSize);
    return false;
  }

  if (!txt->readContent(buffer, offset, chunkSize)) {
    free(buffer);
    return false;
  }
  buffer[chunkSize] = '\0';

  // Prime the SD card font's advance table with this chunk's codepoints.
  // Without this, every getTextAdvanceX() call in the wrap loop below triggers
  // on-demand glyph loads through the 8-slot overflow ring buffer, which
  // thrashes for any text with more than 8 unique chars (i.e. all English),
  // floods the heap with short-lived bitmap allocations, and eventually
  // corrupts FreeRTOS state. The advance table persists across calls per
  // font, so the cost amortizes to ~ASCII-size after the first chunk.
  if (renderer.isSdCardFont(cachedFontId)) {
    // Markdown headers render bold, so prewarm the bold style too.
    renderer.ensureSdCardFontReady(cachedFontId, reinterpret_cast<const char*>(buffer),
                                   /*styleMask=*/isMarkdown ? 0x03 : 0x01);
  }

  // Parse lines from buffer
  size_t pos = 0;
  int usedHeight = 0;
  bool pageFull = false;
  size_t activeTableHeaderOffset = tableHeaderOffset;
  std::vector<std::string> markdownTableHeaders;
  bool inMarkdownTable = isMarkdown && readMarkdownTableHeaders(activeTableHeaderOffset, markdownTableHeaders);
  if (!inMarkdownTable) activeTableHeaderOffset = NO_TABLE_HEADER;

  while (pos < chunkSize && !pageFull) {
    // Find end of line
    size_t lineEnd = pos;
    while (lineEnd < chunkSize && buffer[lineEnd] != '\n') {
      lineEnd++;
    }

    // Check if we have a complete line
    bool lineComplete = (lineEnd < chunkSize) || (offset + lineEnd >= fileSize);

    if (!lineComplete && static_cast<int>(outLines.size()) > 0) {
      // Incomplete line and we already have some lines, stop here
      break;
    }

    // Calculate the actual length of line content in the buffer (excluding newline)
    size_t lineContentLen = lineEnd - pos;

    // Check for carriage return
    bool hasCR = (lineContentLen > 0 && buffer[pos + lineContentLen - 1] == '\r');
    size_t displayLen = hasCR ? lineContentLen - 1 : lineContentLen;

    // Extract line content for display (without CR/LF)
    std::string line(reinterpret_cast<char*>(buffer + pos), displayLen);

    MarkdownParser::TableRow tableRow;
    const bool parsedTableRow = isMarkdown && lineComplete && MarkdownParser::parseTableRow(line, tableRow);
    const bool delimiterRow = parsedTableRow && MarkdownParser::isTableDelimiter(tableRow);
    bool headerRow = false;
    if (parsedTableRow && !delimiterRow && lineEnd < chunkSize) {
      const size_t nextStart = lineEnd + 1;
      size_t nextEnd = nextStart;
      while (nextEnd < chunkSize && buffer[nextEnd] != '\n') ++nextEnd;
      const bool nextComplete = nextEnd < chunkSize || offset + nextEnd >= fileSize;
      if (nextComplete) {
        size_t nextLength = nextEnd - nextStart;
        if (nextLength > 0 && buffer[nextStart + nextLength - 1] == '\r') --nextLength;
        const std::string nextLine(reinterpret_cast<char*>(buffer + nextStart), nextLength);
        MarkdownParser::TableRow nextRow;
        headerRow = MarkdownParser::parseTableRow(nextLine, nextRow) && MarkdownParser::isTableDelimiter(nextRow) &&
                    nextRow.cells.size() == tableRow.cells.size();
      }
    }

    const bool renderTableRow =
        parsedTableRow && !delimiterRow && (headerRow || inMarkdownTable || tableRow.hasOuterPipes);
    if (delimiterRow && (inMarkdownTable || tableRow.hasOuterPipes)) {
      inMarkdownTable = true;
      pos = lineEnd + 1;
      continue;
    }
    if (renderTableRow) {
      if (headerRow) {
        markdownTableHeaders = tableRow.cells;
        inMarkdownTable = true;
        activeTableHeaderOffset = offset + pos;
      }
      const auto fields = MarkdownParser::formatTableRow(tableRow, markdownTableHeaders, headerRow);
      const TableRowResult result = appendMarkdownTableFields(fields, outLines, usedHeight);
      if (result == TableRowResult::DEFERRED) {
        pageFull = true;
        break;
      }
      if (result == TableRowResult::ADDED) {
        pos = lineEnd + 1;
        continue;
      }
      // An unusually large row cannot be kept atomic. Fall through to the
      // ordinary Markdown path so it remains readable and paginates safely.
    } else if (!delimiterRow) {
      inMarkdownTable = false;
      markdownTableHeaders.clear();
      activeTableHeaderOffset = NO_TABLE_HEADER;
    }

    // Decode/break the source line once. The old wrap loop repeated Thai
    // dictionary segmentation and then re-measured successively shorter UTF-8
    // prefixes, making indexing roughly quadratic for long space-less Thai
    // paragraphs. These offsets stay in source-byte coordinates while `line`
    // is sliced below.
    const bool sourceLineHasThai = ThaiBrk::containsThai(line);
    const std::vector<size_t> sourceThaiBreaks =
        sourceLineHasThai ? ThaiBrk::wordBreakByteOffsets(line) : std::vector<size_t>{};
    std::vector<size_t> sourceCharEnds;
    sourceCharEnds.reserve(line.size() / (sourceLineHasThai ? 3 : 1));
    const auto* charPtr = reinterpret_cast<const unsigned char*>(line.c_str());
    const auto* const lineStart = charPtr;
    while (*charPtr) {
      utf8NextCodepoint(&charPtr);
      sourceCharEnds.push_back(static_cast<size_t>(charPtr - lineStart));
    }

    // Track position within this source line (in bytes from pos)
    size_t lineBytePos = 0;

    // Shaping runs only on display copies, keeping the source byte offsets used
    // by pagination and the one-time dictionary break table valid.
    const bool lineHasThai = sourceLineHasThai;
    // Markdown state for this source line: header style applies to every
    // wrapped visual line; leading markers are stripped from the first
    // segment only.
    const uint8_t headingLevel = isMarkdown ? MarkdownParser::headingLevel(line) : 0;
    const auto mdStyle = headingLevel != 0 ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR;
    const int lineFontId = mdHeadingFontId(headingLevel, cachedFontId);
    const int lineAdvance =
        std::max(1, static_cast<int>(std::ceil(renderer.getLineHeight(lineFontId) * cachedLineCompression))) +
        (headingLevel != 0 ? 3 : 0);
    bool mdFirstSegment = true;
    const auto makeDisplayLine = [&](std::string visual, const bool paragraphEnd) {
      if (isMarkdown) visual = MarkdownParser::transformBlock(visual, mdFirstSegment);
      MarkdownParser::Inline parsed;
      if (isMarkdown) {
        parsed = MarkdownParser::parseInline(visual, mdStyle);
      } else {
        parsed.runs.push_back({visual, EpdFontFamily::REGULAR});
      }

      DisplayLine display;
      display.fontId = lineFontId;
      display.style = mdStyle;
      display.headingLevel = headingLevel;
      display.advanceY = lineAdvance;
      display.paragraphEnd = paragraphEnd;
      display.runs.reserve(parsed.runs.size());
      for (auto& run : parsed.runs) {
        if (!lineHasThai) {
          display.text += run.text;
          display.runs.push_back({std::move(run.text), static_cast<uint8_t>(run.style), false});
          continue;
        }

        // Preserve dictionary word boundaries as render-run metadata. Natural
        // Thai text keeps zero added space at these boundaries; Justify uses
        // them as the only stretch points, never splitting a consonant/vowel/
        // tone-mark cluster.
        const auto breakOffsets = ThaiBrk::wordBreakByteOffsets(run.text);
        size_t tokenStart = 0;
        for (const size_t breakOffset : breakOffsets) {
          if (breakOffset <= tokenStart || breakOffset > run.text.size()) continue;
          std::string token = ThaiShaping::shapeUtf8(run.text.substr(tokenStart, breakOffset - tokenStart));
          display.text += token;
          display.runs.push_back({std::move(token), static_cast<uint8_t>(run.style), true});
          ++display.thaiGapCount;
          tokenStart = breakOffset;
        }
        if (tokenStart < run.text.size()) {
          std::string token = ThaiShaping::shapeUtf8(run.text.substr(tokenStart));
          display.text += token;
          display.runs.push_back({std::move(token), static_cast<uint8_t>(run.style), false});
        }
      }
      return display;
    };
    const auto pushDisplayLine = [&](std::string visual, const bool paragraphEnd = false) {
      if (usedHeight + lineAdvance > viewportHeight && !outLines.empty()) return false;
      outLines.push_back(makeDisplayLine(std::move(visual), paragraphEnd));
      usedHeight += lineAdvance;
      mdFirstSegment = false;
      // A single Markdown source line can contain thousands of Thai
      // codepoints. Yield while wrapping it so the watchdog and input tasks
      // keep running even before a complete page has been indexed.
      if ((outLines.size() % 12) == 0) vTaskDelay(1);
      return true;
    };

    // Emit at least one visual line for each source line (including blank lines),
    // then continue with wrapping when needed.
    do {
      if (line.empty()) {
        if (!pushDisplayLine({}, true)) pageFull = true;
        break;
      }

      // Measure what will actually be drawn: the Markdown-transformed, shaped
      // runs in their real font/style (bold text and headings are wider).
      const auto measure = [&](const std::string& s) {
        std::string t = isMarkdown ? MarkdownParser::transformBlock(s, mdFirstSegment) : s;
        const auto parsed = isMarkdown ? MarkdownParser::parseInline(t, mdStyle)
                                       : MarkdownParser::Inline{{{t, EpdFontFamily::REGULAR}}};
        int width = 0;
        for (const auto& run : parsed.runs) {
          std::string shaped = lineHasThai ? ThaiShaping::shapeUtf8(run.text) : run.text;
          width += renderer.getTextAdvanceX(lineFontId, shaped.c_str(), run.style);
        }
        return width;
      };

      int lineWidth = measure(line);

      if (lineWidth <= viewportWidth) {
        if (!pushDisplayLine(line, true)) {
          pageFull = true;
          break;
        }
        lineBytePos = displayLen;  // Consumed entire display content
        line.clear();
        break;
      }

      // Binary-search the largest UTF-8 prefix that fits. Measurement shapes
      // the candidate, so reducing O(codepoints) trials to O(log codepoints)
      // is the main indexing-speed win for Thai files.
      auto firstEnd = std::upper_bound(sourceCharEnds.begin(), sourceCharEnds.end(), lineBytePos);
      size_t low = static_cast<size_t>(firstEnd - sourceCharEnds.begin());
      size_t high = sourceCharEnds.size();
      size_t fittingEnd = lineBytePos;
      while (low < high) {
        const size_t mid = low + (high - low) / 2;
        const size_t candidateEnd = sourceCharEnds[mid];
        if (measure(line.substr(0, candidateEnd - lineBytePos)) <= viewportWidth) {
          fittingEnd = candidateEnd;
          low = mid + 1;
        } else {
          high = mid;
        }
      }
      size_t fittingBytes = fittingEnd > lineBytePos ? fittingEnd - lineBytePos : 0;
      if (fittingBytes == 0 && firstEnd != sourceCharEnds.end()) {
        fittingBytes = *firstEnd - lineBytePos;  // always consume one complete codepoint
      }

      // Prefer the latest natural boundary that fits: an ASCII space or Thai
      // dictionary word boundary. Fall back to the UTF-8 character boundary
      // only for a single overlong word/OOV cluster.
      size_t breakPos = 0;
      if (fittingBytes > 0) {
        const size_t spacePos = line.rfind(' ', fittingBytes - 1);
        if (spacePos != std::string::npos && spacePos > 0) breakPos = spacePos;
      }
      if (lineHasThai) {
        for (const size_t absoluteBreak : sourceThaiBreaks) {
          if (absoluteBreak <= lineBytePos) continue;
          const size_t relativeBreak = absoluteBreak - lineBytePos;
          if (relativeBreak > fittingBytes) break;
          if (relativeBreak > breakPos) breakPos = relativeBreak;
        }
      }
      if (breakPos == 0) breakPos = fittingBytes;

      if (!pushDisplayLine(line.substr(0, breakPos))) {
        pageFull = true;
        break;
      }

      // Skip space at break point
      size_t skipChars = breakPos;
      if (breakPos < line.length() && line[breakPos] == ' ') {
        skipChars++;
      }
      lineBytePos += skipChars;
      line = line.substr(skipChars);
    } while (!line.empty() && !pageFull);

    // Determine how much of the source buffer we consumed
    if (line.empty()) {
      // Fully consumed this source line, move past the newline
      pos = lineEnd + 1;
    } else {
      // Partially consumed - page is full mid-line
      // Move pos to where we stopped in the line (NOT past the line)
      pos = pos + lineBytePos;
      break;
    }
  }

  // Ensure we make progress even if calculations go wrong
  if (pos == 0 && !outLines.empty()) {
    // Fallback: at minimum, consume something to avoid infinite loop
    pos = 1;
  }

  nextOffset = offset + pos;

  // Make sure we don't go past the file
  if (nextOffset > fileSize) {
    nextOffset = fileSize;
  }
  nextTableHeaderOffset = inMarkdownTable ? activeTableHeaderOffset : NO_TABLE_HEADER;

  free(buffer);

  return !outLines.empty();
}

void TxtReaderActivity::render(RenderLock&&) {
  if (!txt) {
    return;
  }

  // Initialize reader if not done
  if (!initialized) {
    initializeReader();
  }

  if (pageOffsets.empty()) {
    renderer.clearScreen();
    renderer.drawCenteredText(UI_12_FONT_ID, 300, tr(STR_EMPTY_FILE), true, EpdFontFamily::BOLD);
    renderer.displayBuffer();
    return;
  }

  // Bounds check
  if (currentPage < 0) currentPage = 0;
  if (currentPage >= totalPages) currentPage = totalPages - 1;

  // Load current page content
  size_t offset = pageOffsets[currentPage];
  size_t nextOffset;
  const size_t tableHeaderOffset = currentPage < static_cast<int>(pageTableHeaderOffsets.size())
                                       ? pageTableHeaderOffsets[currentPage]
                                       : NO_TABLE_HEADER;
  size_t nextTableHeaderOffset = tableHeaderOffset;
  currentPageLines.clear();
  loadPageAtOffset(offset, currentPageLines, nextOffset, tableHeaderOffset, nextTableHeaderOffset);

  renderer.clearScreen();
  renderPage();

  // Save progress
  saveProgress();
}

void TxtReaderActivity::renderPage() {
  const int contentWidth = viewportWidth;

  // Render text lines with alignment
  auto renderLines = [&]() {
    int y = cachedOrientedMarginTop;
    for (size_t lineIdx = 0; lineIdx < currentPageLines.size(); ++lineIdx) {
      const auto& line = currentPageLines[lineIdx];
      if (!line.text.empty()) {
        int x = cachedOrientedMarginLeft;
        const bool lineIsRtl = BidiUtils::startsWithRtl(line.text.c_str(), BidiUtils::RTL_PARAGRAPH_PROBE_DEPTH);
        uint8_t effectiveAlignment = cachedParagraphAlignment;
        if (lineIsRtl && (effectiveAlignment == CrossPointSettings::LEFT_ALIGN ||
                          effectiveAlignment == CrossPointSettings::JUSTIFIED)) {
          effectiveAlignment = CrossPointSettings::RIGHT_ALIGN;
        }
        int textWidth = 0;
        for (const auto& run : line.runs) {
          textWidth +=
              renderer.getTextAdvanceX(line.fontId, run.text.c_str(), static_cast<EpdFontFamily::Style>(run.style));
        }
        const bool justifyThai = effectiveAlignment == CrossPointSettings::JUSTIFIED && !line.paragraphEnd &&
                                 line.thaiGapCount > 0 && textWidth < contentWidth;
        // Thai has no visible inter-word spaces. Stretching every dictionary
        // boundary to fill the full width produced the large holes seen on
        // device, especially when a line has only a few detected words. Keep
        // at most a subtle one-pixel adjustment and otherwise retain natural
        // Sarabun spacing like the upstream reader.
        const int thaiGapExtra =
            justifyThai ? std::min(MAX_THAI_JUSTIFY_GAP_PX, (contentWidth - textWidth) / line.thaiGapCount) : 0;

        // Apply text alignment
        switch (effectiveAlignment) {
          case CrossPointSettings::LEFT_ALIGN:
          default:
            // x already set to left margin
            break;
          case CrossPointSettings::CENTER_ALIGN: {
            x = cachedOrientedMarginLeft + (contentWidth - textWidth) / 2;
            break;
          }
          case CrossPointSettings::RIGHT_ALIGN: {
            x = cachedOrientedMarginLeft + contentWidth - textWidth;
            break;
          }
          case CrossPointSettings::JUSTIFIED:
            // For plain text, justified is treated as left-aligned
            // (true justification would require word spacing adjustments)
            break;
        }

        for (const auto& run : line.runs) {
          const auto style = static_cast<EpdFontFamily::Style>(run.style);
          renderer.drawText(line.fontId, x, y, run.text.c_str(), true, style);
          x += renderer.getTextAdvanceX(line.fontId, run.text.c_str(), style);
          if (run.thaiBreakAfter) x += thaiGapExtra;
        }
      }
      y += line.advanceY > 0 ? line.advanceY : renderer.getLineHeight(cachedFontId);
    }
  };

  // Font prewarm: scan pass accumulates text, then prewarm, then real render
  auto* fcm = renderer.getFontCacheManager();
  auto scope = fcm->createPrewarmScope();
  renderLines();  // scan pass — text accumulated, no drawing
  scope.endScanAndPrewarm();

  // BW rendering
  renderLines();
  renderStatusBar();

  ReaderUtils::displayWithRefreshCycle(renderer, pagesUntilFullRefresh);

  if (SETTINGS.textAntiAliasing) {
    ReaderUtils::renderAntiAliased(renderer, [&renderLines]() { renderLines(); });
  }
  // scope destructor clears font cache via FontCacheManager
}

void TxtReaderActivity::renderStatusBar() const {
  const float progress = totalPages > 0 ? (currentPage + 1) * 100.0f / totalPages : 0;
  std::string title;
  if (SETTINGS.statusBarSpec().showsTitle()) {
    title = txt->getTitle();
  }
  GUI.drawStatusBar(renderer, progress, currentPage + 1, totalPages, title);
}

void TxtReaderActivity::saveProgress() const {
  uint8_t data[4];
  data[0] = currentPage & 0xFF;
  data[1] = (currentPage >> 8) & 0xFF;
  data[2] = 0;
  data[3] = 0;
  if (!ProgressFile::writeAtomic(txt->getCachePath(), data, sizeof(data))) {
    LOG_ERR("TRS", "Failed to save progress: page %d", currentPage);
  }
}

void TxtReaderActivity::loadProgress() {
  HalFile f;
  if (Storage.openFileForRead("TRS", txt->getCachePath() + "/progress.bin", f)) {
    uint8_t data[4];
    if (f.read(data, 4) == 4) {
      currentPage = data[0] + (data[1] << 8);
      if (currentPage >= totalPages) {
        currentPage = totalPages - 1;
      }
      if (currentPage < 0) {
        currentPage = 0;
      }
      LOG_DBG("TRS", "Loaded progress: page %d/%d", currentPage, totalPages);
    }
  }
}

bool TxtReaderActivity::loadPageIndexCache() {
  // Cache file format (using serialization module):
  // - uint32_t: magic "TXTI"
  // - uint8_t: cache version
  // - uint32_t: file size (to validate cache)
  // - int32_t: viewport width
  // - int32_t: lines per page
  // - int32_t: font ID (to invalidate cache on font change)
  // - int32_t: screen margin (to invalidate cache on margin change)
  // - uint8_t: paragraph alignment (to invalidate cache on alignment change)
  // - uint8_t: line spacing (to invalidate cache on vertical rhythm change)
  // - uint32_t: total pages count
  // - N * uint32_t: page offsets
  // - N * uint32_t: active Markdown table-header offsets (UINT32_MAX for none)

  std::string cachePath = txt->getCachePath() + "/index.bin";
  HalFile f;
  if (!Storage.openFileForRead("TRS", cachePath, f)) {
    LOG_DBG("TRS", "No page index cache found");
    return false;
  }

  // Read and validate header using serialization module
  uint32_t magic;
  serialization::readPod(f, magic);
  if (magic != CACHE_MAGIC) {
    LOG_DBG("TRS", "Cache magic mismatch, rebuilding");
    return false;
  }

  uint8_t version;
  serialization::readPod(f, version);
  if (version != CACHE_VERSION) {
    LOG_DBG("TRS", "Cache version mismatch (%d != %d), rebuilding", version, CACHE_VERSION);
    return false;
  }

  uint32_t fileSize;
  serialization::readPod(f, fileSize);
  if (fileSize != txt->getFileSize()) {
    LOG_DBG("TRS", "Cache file size mismatch, rebuilding");
    return false;
  }

  int32_t cachedWidth;
  serialization::readPod(f, cachedWidth);
  if (cachedWidth != viewportWidth) {
    LOG_DBG("TRS", "Cache viewport width mismatch, rebuilding");
    return false;
  }

  int32_t cachedLines;
  serialization::readPod(f, cachedLines);
  if (cachedLines != linesPerPage) {
    LOG_DBG("TRS", "Cache lines per page mismatch, rebuilding");
    return false;
  }

  int32_t fontId;
  serialization::readPod(f, fontId);
  if (fontId != cachedFontId) {
    LOG_DBG("TRS", "Cache font ID mismatch (%d != %d), rebuilding", fontId, cachedFontId);
    return false;
  }

  int32_t margin;
  serialization::readPod(f, margin);
  if (margin != cachedScreenMargin) {
    LOG_DBG("TRS", "Cache screen margin mismatch, rebuilding");
    return false;
  }

  uint8_t alignment;
  serialization::readPod(f, alignment);
  if (alignment != cachedParagraphAlignment) {
    LOG_DBG("TRS", "Cache paragraph alignment mismatch, rebuilding");
    return false;
  }

  uint8_t lineSpacing;
  serialization::readPod(f, lineSpacing);
  if (lineSpacing != cachedLineSpacing) {
    LOG_DBG("TRS", "Cache line spacing mismatch (%d != %d), rebuilding", lineSpacing, cachedLineSpacing);
    return false;
  }

  uint32_t numPages;
  serialization::readPod(f, numPages);

  // Read page offsets
  pageOffsets.clear();
  pageOffsets.reserve(numPages);

  for (uint32_t i = 0; i < numPages; i++) {
    uint32_t offset;
    serialization::readPod(f, offset);
    pageOffsets.push_back(offset);
  }
  pageTableHeaderOffsets.clear();
  pageTableHeaderOffsets.reserve(numPages);
  for (uint32_t i = 0; i < numPages; i++) {
    uint32_t offset;
    serialization::readPod(f, offset);
    pageTableHeaderOffsets.push_back(offset == NO_TABLE_HEADER_CACHE ? NO_TABLE_HEADER : static_cast<size_t>(offset));
  }

  totalPages = pageOffsets.size();
  LOG_DBG("TRS", "Loaded page index cache: %d pages", totalPages);
  return true;
}

void TxtReaderActivity::savePageIndexCache() const {
  std::string cachePath = txt->getCachePath() + "/index.bin";
  HalFile f;
  if (!Storage.openFileForWrite("TRS", cachePath, f)) {
    LOG_ERR("TRS", "Failed to save page index cache");
    return;
  }

  // Write header using serialization module
  serialization::writePod(f, CACHE_MAGIC);
  serialization::writePod(f, CACHE_VERSION);
  serialization::writePod(f, static_cast<uint32_t>(txt->getFileSize()));
  serialization::writePod(f, static_cast<int32_t>(viewportWidth));
  serialization::writePod(f, static_cast<int32_t>(linesPerPage));
  serialization::writePod(f, static_cast<int32_t>(cachedFontId));
  serialization::writePod(f, static_cast<int32_t>(cachedScreenMargin));
  serialization::writePod(f, cachedParagraphAlignment);
  serialization::writePod(f, cachedLineSpacing);
  serialization::writePod(f, static_cast<uint32_t>(pageOffsets.size()));

  // Write page offsets
  for (size_t offset : pageOffsets) {
    serialization::writePod(f, static_cast<uint32_t>(offset));
  }
  for (size_t offset : pageTableHeaderOffsets) {
    serialization::writePod(f, offset == NO_TABLE_HEADER ? NO_TABLE_HEADER_CACHE : static_cast<uint32_t>(offset));
  }

  LOG_DBG("TRS", "Saved page index cache: %d pages", totalPages);
}

ScreenshotInfo TxtReaderActivity::getScreenshotInfo() const {
  ScreenshotInfo info;
  info.readerType = ScreenshotInfo::ReaderType::Txt;
  if (txt) {
    const std::string t = txt->getTitle();
    snprintf(info.title, sizeof(info.title), "%s", t.c_str());
  }
  info.currentPage = currentPage + 1;
  info.totalPages = totalPages;
  info.progressPercent = totalPages > 0 ? static_cast<int>((currentPage + 1) * 100.0f / totalPages + 0.5f) : 0;
  if (info.progressPercent > 100) info.progressPercent = 100;
  return info;
}
