#pragma once

#include <string>

#include "ReadingStatsCodec.h"

namespace ReadingStatsStore {

ReadingStatsSnapshot loadBook(const std::string& cachePath);
ReadingStatsSnapshot loadGlobal();
bool saveBook(const std::string& cachePath, const ReadingStatsSnapshot& stats);
bool saveGlobal(const ReadingStatsSnapshot& stats);
void formatDuration(uint32_t seconds, char* buffer, size_t bufferSize);

}  // namespace ReadingStatsStore
