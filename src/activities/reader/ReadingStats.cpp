#include "ReadingStats.h"

#include <HalStorage.h>
#include <I18n.h>
#include <Logging.h>

#include <array>

namespace {

constexpr char BOOK_STATS_FILE[] = "crosspointTH_stats_v1.bin";
constexpr char GLOBAL_STATS_PATH[] = "/.crosspoint/crosspointTH_global_stats_v1.bin";

template <size_t N>
bool readStatsFile(const std::string& path, std::array<uint8_t, N>& data) {
  HalFile file;
  if (!Storage.openFileForRead("THSTAT", path, file)) return false;
  const int bytesRead = file.read(data.data(), data.size());
  file.close();
  return bytesRead == static_cast<int>(data.size());
}

template <size_t N>
bool writeStatsFileAtomic(const std::string& path, const std::array<uint8_t, N>& data) {
  const std::string temporaryPath = path + ".tmp";
  {
    HalFile file;
    if (!Storage.openFileForWrite("THSTAT", temporaryPath, file)) {
      LOG_ERR("THSTAT", "Could not open stats temp file: %s", temporaryPath.c_str());
      return false;
    }
    if (file.write(data.data(), data.size()) != data.size()) {
      LOG_ERR("THSTAT", "Short write for stats temp file: %s", temporaryPath.c_str());
      file.close();
      Storage.remove(temporaryPath.c_str());
      return false;
    }
    file.flush();
    file.close();
  }

  if (Storage.exists(path.c_str()) && !Storage.remove(path.c_str())) {
    LOG_ERR("THSTAT", "Could not replace stats file: %s", path.c_str());
    Storage.remove(temporaryPath.c_str());
    return false;
  }
  if (!Storage.rename(temporaryPath.c_str(), path.c_str())) {
    LOG_ERR("THSTAT", "Could not publish stats file: %s", path.c_str());
    Storage.remove(temporaryPath.c_str());
    return false;
  }
  return true;
}

}  // namespace

ReadingStatsSnapshot ReadingStatsStore::loadBook(const std::string& cachePath) {
  ReadingStatsSnapshot stats;
  std::array<uint8_t, ReadingStatsCodec::BOOK_FILE_SIZE> data{};
  if (!readStatsFile(cachePath + "/" + BOOK_STATS_FILE, data)) return stats;
  if (!ReadingStatsCodec::decodeBook(data.data(), data.size(), stats)) {
    LOG_ERR("THSTAT", "Ignoring incompatible book stats file");
    return {};
  }
  return stats;
}

ReadingStatsSnapshot ReadingStatsStore::loadGlobal() {
  ReadingStatsSnapshot stats;
  std::array<uint8_t, ReadingStatsCodec::GLOBAL_FILE_SIZE> data{};
  if (!readStatsFile(GLOBAL_STATS_PATH, data)) return stats;
  if (!ReadingStatsCodec::decodeGlobal(data.data(), data.size(), stats)) {
    LOG_ERR("THSTAT", "Ignoring incompatible global stats file");
    return {};
  }
  return stats;
}

bool ReadingStatsStore::saveBook(const std::string& cachePath, const ReadingStatsSnapshot& stats) {
  return writeStatsFileAtomic(cachePath + "/" + BOOK_STATS_FILE, ReadingStatsCodec::encodeBook(stats));
}

bool ReadingStatsStore::saveGlobal(const ReadingStatsSnapshot& stats) {
  return writeStatsFileAtomic(GLOBAL_STATS_PATH, ReadingStatsCodec::encodeGlobal(stats));
}

void ReadingStatsStore::formatDuration(const uint32_t seconds, char* buffer, const size_t bufferSize) {
  if (seconds < 60) {
    snprintf(buffer, bufferSize, "%s", tr(STR_STATS_LESS_THAN_MIN));
    return;
  }
  const uint32_t hours = seconds / 3600;
  const uint32_t minutes = (seconds % 3600) / 60;
  if (hours == 0) {
    snprintf(buffer, bufferSize, "%lu min", static_cast<unsigned long>(minutes));
  } else {
    snprintf(buffer, bufferSize, "%luh %lu min", static_cast<unsigned long>(hours),
             static_cast<unsigned long>(minutes));
  }
}
