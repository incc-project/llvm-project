//===--- SymbolTableSection.h - Symbol table section ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Symbol table section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_SYMBOLTABLESECTION_H
#define ILLVM_SYMBOLTABLESECTION_H

#include "illvm/FuncV/ELF/Section.h"
#include "illvm/FuncV/ELF/StringTableSection.h"
#include "illvm/FuncV/ELF/Symbol.h"

namespace illvm {
namespace funcv {
namespace elf {

// Updated by SymbolTableSection.
class SymtabShndxSection final : public Section {
private:
  // idx -> symbol shndx.
  std::vector<uint32_t> originalIndexes;
  // Reconstruction by SymbolTableSection.
  std::vector<uint32_t> indexes;

public:
  friend class SymbolTableSection;

  SymtabShndxSection(const llvm::object::ELF64LE::Shdr *shdr,
                     const char *_data);

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  std::string dataToString() const override;
};

class SymbolTableSection final : public Section {
private:
  using Elf_Sym = llvm::object::ELF64LE::Sym;
  std::shared_ptr<SymtabShndxSection> symtabShndx;
  std::vector<std::shared_ptr<Symbol>> symbols;

  // Update after layout.
  uint64_t localSymNum;

public:
  SymbolTableSection(const llvm::object::ELF64LE::Shdr *shdr,
                     const char *_data);

  void parseReferences(const std::vector<std::shared_ptr<Section>> &sections,
                       const std::shared_ptr<StringTableSection> &strTab) const;

  void layout() override;

  // Call after section layout.
  bool needSymtabShNdx() const;

  void fini() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  std::string dataToString() const override;

  size_t getSize() const { return symbols.size(); }

  auto getSymbol(const size_t idx) { return symbols[idx]; }

  void push_back(const std::shared_ptr<Symbol> &symbol) {
    symbols.push_back(symbol);
  }

  const auto &getSymbols() const { return symbols; }

  void setSymtabShndx(const std::shared_ptr<SymtabShndxSection> &_symtabShndx) {
    symtabShndx = _symtabShndx;
  }

  auto getSymtabShndx() { return symtabShndx; }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_SYMBOLTABLESECTION_H
