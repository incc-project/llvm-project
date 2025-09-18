//===--- ReuseRelocation.h - Reuse relocation -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Reuse relocation sections, adjust relocation entries (include .rela.eh_frame,
// relocations in CIE/FDE).
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_REUSERELOCATION_H
#define ILLVM_REUSERELOCATION_H

#include "illvm/FuncV/ELF/BDG.h"

namespace illvm {
namespace funcv {
namespace elf {

class ReuseRelocation {
private:
  static llvm::Expected<std::shared_ptr<Relocation>> createNewRelaEntry(
      const std::shared_ptr<Relocation> &oldRelaEntry,
      const std::unordered_map<std::string, std::weak_ptr<ReuseNode>>
          &dependencies);

  static llvm::Expected<std::shared_ptr<RelocationSection>>
  createNewRelaSection(
      const std::shared_ptr<ReuseNode> &reuseNode, ObjFile &newObjFile,
      const std::shared_ptr<Section> &newSection,
      const std::shared_ptr<RelocationSection> &oldRelocationSection);

  static llvm::Error
  reuseCIERelaEntries(const std::shared_ptr<ReuseNode> &reuseNode,
                      ObjFile &newObjFile, const std::shared_ptr<CFI> &newCFI,
                      const std::shared_ptr<CFI> &oldCFI);

  static llvm::Error
  reuseFDERelaEntries(const std::shared_ptr<ReuseNode> &reuseNode,
                      ObjFile &newObjFile, const std::shared_ptr<FDE> &newFDE,
                      const std::shared_ptr<FDE> &oldFDE);

public:
  static llvm::Error run(ObjFile &newObjFile, const BDG &bdg);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_REUSERELOCATION_H
