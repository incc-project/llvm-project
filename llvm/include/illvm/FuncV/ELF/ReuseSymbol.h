//===--- ReuseSymbol.h - Reuse symbol ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Reuse symbol (include .iclang.reusev).
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_REUSESYMBOL_H
#define ILLVM_REUSESYMBOL_H

#include "illvm/FuncV/ELF/BDG.h"

namespace illvm {
namespace funcv {
namespace elf {

class ReuseSymbol {
private:
  static void replaceNewSymbol(const std::shared_ptr<Section> &newSection,
                               const std::shared_ptr<Symbol> &newSymbol,
                               const std::shared_ptr<Symbol> &oldSymbol);

  static std::shared_ptr<Symbol>
  createNewSymbol(ObjFile &newObjFile,
                  const std::shared_ptr<Section> &newSection,
                  const std::shared_ptr<Symbol> &oldSymbol);

  static void handleReuseVersionSymbol(ObjFile &newObjFile,
                                       const uint64_t oldReuseVersion);

public:
  static void run(ObjFile &newObjFile, const BDG &bdg);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_REUSESYMBOL_H
