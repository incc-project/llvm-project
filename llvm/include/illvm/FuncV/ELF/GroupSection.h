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

public:
  GroupSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data);

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
