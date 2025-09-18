//===--- ObjFile.h - ELF obj file ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Workflow:
// * Parse ELF header.
// * Parse section header table (Note: e_shnum, e_shstrndx).
// * Parse string table.
// * Parse .symtab_shndx, symbol table.
// * Parse other sections.
// * Parse references.
// * Create necessary sections.
// * Update section layout.
// * Update section header and content (Note: e_shnum and e_shstrndx).
// * Write.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_OBJFILE_H
#define ILLVM_OBJFILE_H

#include <memory>
#include <string>
#include <vector>

#include "illvm/FuncV/ELF/Sections.h"
#include "illvm/Support/BinFile.h"

namespace illvm {
namespace funcv {
namespace elf {

class ObjFile {
private:
  const BinFile &binFile;

  unsigned char e_ident[llvm::ELF::EI_NIDENT] = {};
  uint16_t e_type = 0;
  uint16_t e_machine = 0;
  uint32_t e_version = 0;
  uint64_t e_entry = 0;
  uint64_t e_phoff = 0;
  // Update while writing.
  uint64_t e_shoff = 0;
  uint32_t e_flags = 0;
  uint16_t e_ehsize = 0;
  uint16_t e_phentsize = 0;
  uint64_t e_phnum = 0;
  uint16_t e_shentsize = 0;
  // Update while writing.
  uint64_t e_shnum = 0;
  uint64_t e_shstrndx = 0;
  // e_shstrndx.
  std::shared_ptr<StringTableSection> shstrTab = nullptr;

  std::vector<std::shared_ptr<Section>> sections;

  std::shared_ptr<StringTableSection> strTab = nullptr;
  std::shared_ptr<SymbolTableSection> symTab = nullptr;
  std::shared_ptr<EhFrameSection> ehFrame = nullptr;
  std::shared_ptr<RelocationSection> relaEhFrame = nullptr;

  std::shared_ptr<DebugAbbrevSection> debugAbbrev;
  std::shared_ptr<DebugStrSection> debugStr;
  std::shared_ptr<DebugLineStrSection> debugLineStr;
  std::shared_ptr<DebugInfoSection> debugInfoSection;
  std::shared_ptr<DebugStrOffsetsSection> debugStrOff;
  std::shared_ptr<DebugAddrSection> debugAddr;
  std::shared_ptr<DebugRnglistSection> debugRnglist;
  std::shared_ptr<RelocationSection> relaDebugStrOffsets;
  std::shared_ptr<RelocationSection> relaDebugAddr;

  static bool getIsRela(const uint16_t m);

  llvm::Error parseHeader();

  llvm::Error parseStrSymTable(
      const char *object,
      const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs);

  llvm::Error parseOtherSections(
      const char *object,
      const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs);

  void parseReferences();

  llvm::Error parseSections();

  llvm::Expected<std::shared_ptr<SymtabShndxSection>> createSymtabShndx();

  static std::size_t alignOffset(const std::uint64_t offset,
                                 const std::uint64_t sh_addralign);

  void layout();

public:
  explicit ObjFile(const BinFile &_binFile) : binFile(_binFile) {}

  llvm::Error init();

  llvm::Error fini();

  void save(const std::string &outputPath) const;

  void dump(std::ostream &oss) const;

  std::string toString() const;

  std::shared_ptr<StringTableSection> &getStrTab() { return strTab; }

  std::shared_ptr<StringTableSection> &getShstrTab() { return shstrTab; }

  std::shared_ptr<SymbolTableSection> &getSymTab() { return symTab; }

  std::shared_ptr<EhFrameSection> &getEhFrame() { return ehFrame; }

  std::shared_ptr<RelocationSection> &getRelaEhFrame() { return relaEhFrame; }

  std::vector<std::shared_ptr<Section>> &getSections() { return sections; }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_OBJFILE_H
