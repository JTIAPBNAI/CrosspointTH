#pragma once

#include <string>

#include "ReadingStatsCodec.h"
#include "activities/Activity.h"

class ReadingStatsActivity final : public Activity {
  std::string bookTitle;
  ReadingStatsSnapshot bookStats;
  ReadingStatsSnapshot globalStats;

 public:
  ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string title,
                       const ReadingStatsSnapshot& book, const ReadingStatsSnapshot& global);

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
