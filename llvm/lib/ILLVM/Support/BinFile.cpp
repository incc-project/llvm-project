#include "illvm/Support/BinFile.h"

#include "illvm/Support/Diagnostics.h"

namespace illvm {

BinFile::BinFile(const std::string &path)
    : filePath(path), fileData(nullptr), fileSize(0) {
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  ILLVM_FCHECK(file.is_open(), "Failed to open file: " + filePath);

  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  fileData = malloc(fileSize);
  ILLVM_FCHECK(fileData != nullptr,
               "Memory allocation failed for file: " + filePath);

  if (!file.read(static_cast<char *>(fileData), fileSize)) {
    free(fileData);
    ILLVM_UNREACHABLE("Error reading file: " + filePath);
  }

  file.close();
}

const char *BinFile::readBytes(const int offset) const {
  const auto &logger = Logger::getInstance();
  if (offset < 0 || offset >= fileSize) {
    logger.fatal("Offset out of range in file: " + filePath);
  }
  return static_cast<char *>(fileData) + offset;
}

} // namespace illvm
