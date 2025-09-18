//===--- RelocationSection.h - Relocation section ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Relocation section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_RELOCATIONSECTION_H
#define ILLVM_RELOCATIONSECTION_H

#include "illvm/FuncV/ELF/Relocation.h"
#include "illvm/FuncV/ELF/Section.h"
#include "illvm/FuncV/ELF/SymbolTableSection.h"

namespace illvm {
namespace funcv {
namespace elf {

class RelocationSection final : public Section {
private:
  using Elf_Rela = llvm::object::ELF64LE::Rela;
  // index -> relocation entry.
  std::vector<std::shared_ptr<Relocation>> relocations;

  RelocationSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                    llvm::Error &err);

public:
  static llvm::Expected<std::shared_ptr<RelocationSection>>
  Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<RelocationSection>(
        new RelocationSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  void parseReferences(const std::shared_ptr<SymbolTableSection> &symTab) const;

  void layout() override;

  void fini() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  const auto &getRelocations() const { return relocations; }

  void push_back(const std::shared_ptr<Relocation> &relocation) {
    relocations.push_back(relocation);
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_RELOCATIONSECTION_H
