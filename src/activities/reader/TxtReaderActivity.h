#pragma once

#include <Txt.h>

#include <vector>

#include "CrossPointSettings.h"
#include "activities/Activity.h"

class TxtReaderActivity final : public Activity {
  struct TextRun {
    std::string text;
    uint8_t style = 0;
    bool thaiBreakAfter = false;
  };

  struct DisplayLine {
    std::string text;
    std::vector<TextRun> runs;
    int fontId = 0;
    uint8_t style = 0;
    uint8_t headingLevel = 0;
    int advanceY = 0;
    uint16_t thaiGapCount = 0;
    bool paragraphEnd = false;
  };

  std::unique_ptr<Txt> txt;

  int currentPage = 0;
  int totalPages = 1;
  int pagesUntilFullRefresh = 0;

  // Streaming text reader - stores file offsets for each page
  std::vector<size_t> pageOffsets;  // File offset for start of each page
  std::vector<DisplayLine> currentPageLines;
  // .md files get block and inline formatting: scaled/bold headings, bullets,
  // bold/italic runs, code emphasis, and links displayed without their URL.
  bool isMarkdown = false;
  int linesPerPage = 0;
  int viewportWidth = 0;
  int viewportHeight = 0;
  bool initialized = false;

  // Cached settings for cache validation (different fonts/margins require re-indexing)
  int cachedFontId = 0;
  uint8_t cachedScreenMargin = 0;
  uint8_t cachedParagraphAlignment = CrossPointSettings::LEFT_ALIGN;
  uint8_t cachedLineSpacing = CrossPointSettings::NORMAL;
  float cachedLineCompression = 1.0f;
  int cachedOrientedMarginTop = 0;
  int cachedOrientedMarginRight = 0;
  int cachedOrientedMarginBottom = 0;
  int cachedOrientedMarginLeft = 0;

  void renderPage();
  void renderStatusBar() const;

  void initializeReader();
  bool loadPageAtOffset(size_t offset, std::vector<DisplayLine>& outLines, size_t& nextOffset);
  void buildPageIndex();
  bool loadPageIndexCache();
  void savePageIndexCache() const;
  void saveProgress() const;
  void loadProgress();

 public:
  explicit TxtReaderActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::unique_ptr<Txt> txt)
      : Activity("TxtReader", renderer, mappedInput), txt(std::move(txt)) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool isReaderActivity() const override { return true; }
  ScreenshotInfo getScreenshotInfo() const override;
};
