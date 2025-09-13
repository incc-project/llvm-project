//===--- EHFrameSection.h - EHFrame section -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// EHFrame section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_EHFRAMESECTION_H
#define ILLVM_EHFRAMESECTION_H

#include "illvm/FuncV/ELF/EhFrame.h"
#include "illvm/FuncV/ELF/RelocationSection.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class EhFrameSection final : public Section {
private:
  std::vector<std::shared_ptr<CFI>> cfis;

  using RelaPair = std::pair<uint64_t, std::shared_ptr<Relocation>>;

  // Return false means termination.
  bool loadEntry(uint64_t &offset, uint32_t &length, uint64_t &extLength,
                 uint32_t &entryFlag, const char *&otherData) const;

  void linkRelaToCIEFDE(const std::vector<RelaPair> &rOffsetToRela) const;

  void adjustRelativeROffset() const;

public:
  EhFrameSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data);

  void
  parseReferences(const std::shared_ptr<RelocationSection> &relaEhFrame) const;

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  void addCFI(const std::shared_ptr<CFI> &newCFI) { cfis.push_back(newCFI); }

  auto &getCFIs() const { return cfis; }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_EHFRAMESECTION_H
