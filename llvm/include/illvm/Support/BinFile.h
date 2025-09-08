//===--- BinFile.h - Binary file -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Binary file.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_BINFILE_HPP
#define ILLVM_BINFILE_HPP

#include <fstream>
#include <string>

namespace illvm {

class BinFile {
private:
  std::string filePath;

  void *fileData;

  std::streamsize fileSize;

public:
  explicit BinFile(const std::string &path);

  ~BinFile() {
    if (fileData) {
      free(fileData);
    }
  }

  std::streamsize getFileSize() const { return fileSize; }

  const std::string &getFilePath() const { return filePath; }

  const char *readBytes(const int offset) const;
};

} // namespace illvm

#endif //ILLVM_BINFILE_HPP
