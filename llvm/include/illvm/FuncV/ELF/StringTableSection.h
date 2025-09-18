//===--- StringTableSection.h - String table section ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// String table section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_STRINGTABLESECTION_H
#define ILLVM_STRINGTABLESECTION_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class StringTableSection final : public Section {
private:
  // offset -> strRef.
  // It can only work during parsing.
  std::unordered_map<uint64_t, std::shared_ptr<StrRef>> originalIndexes;
  // The first string should be "".
  std::vector<std::shared_ptr<StrRef>> strs;

  StringTableSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                     llvm::Error &err);

public:

  static llvm::Expected<std::shared_ptr<StringTableSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<StringTableSection>(
        new StringTableSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  std::shared_ptr<StrRef> parseOriginalIndex(uint64_t strOff);

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  // Add an existed str ref to strtab.
  void push_back(const std::shared_ptr<StrRef> &str) { strs.push_back(str); }

  // Add a new str to strtab.
  std::shared_ptr<StrRef> push_back(const std::string &str) {
    const auto res = std::make_shared<StrRef>(str, 0);
    strs.push_back(res);
    return res;
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_STRINGTABLESECTION_H
