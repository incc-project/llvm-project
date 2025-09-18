//===--- GroupSection.h - Group section ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Group section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_GROUPSECTION_H
#define ILLVM_GROUPSECTION_H

#include <memory>
#include <vector>

#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class GroupSection final : public Section {
private:
  std::vector<std::shared_ptr<Section>> sections;

  GroupSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data, llvm::Error &err);

public:

  static llvm::Expected<std::shared_ptr<GroupSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section =
        std::shared_ptr<GroupSection>(new GroupSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  void
  parseReferences(const std::vector<std::shared_ptr<Section>> &allSections);

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  const auto &getSections() { return sections; }

  void push_back(const std::shared_ptr<Section> &section) {
    sections.push_back(section);
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_GROUPSECTION_H
