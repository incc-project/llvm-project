#include "illvm/Support/BinFile.h"

#include "illvm/Support/Diagnostics.h"

namespace illvm {

BinFile::BinFile(const std::string &path)
    : filePath(path), fileData(nullptr), fileSize(0) {
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  ILLVM_FCHECK(file.is_open(), "failed to open file: " + filePath);

  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  fileData = malloc(fileSize);
  ILLVM_FCHECK(fileData != nullptr,
               "memory allocation failed for file: " + filePath);

  if (!file.read(static_cast<char *>(fileData), fileSize)) {
    free(fileData);
    ILLVM_UNREACHABLE("error reading file: " + filePath);
  }

  file.close();
}

const char *BinFile::readBytes(const int offset) const {
  ILLVM_FCHECK(0 <= offset && offset < fileSize,
               "offset out of range in file: " + filePath);
  return static_cast<char *>(fileData) + offset;
}

} // namespace illvm
