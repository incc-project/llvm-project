//===--- ReuseEhFrame.h - Reuse eh_frame ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Reuse eh_frame (the reuse of related relocations is the responsibility of
// ReuseRelocation.h).
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_REUSEEHFRAME_H
#define ILLVM_REUSEEHFRAME_H

#include "illvm/FuncV/ELF/BDG.h"

namespace illvm {
namespace funcv {
namespace elf {

class ReuseEhFrame {
private:
  static std::shared_ptr<CFI>
  createNewCFI(ObjFile &newObjFile, const std::shared_ptr<CFI> &oldCFI);

  static std::shared_ptr<FDE>
  createNewFDE(const std::shared_ptr<CFI> &newCFI,
               const std::shared_ptr<FDE> &oldFDE);

public:
  static void run(ObjFile &newObjFile, const BDG &bdg);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_REUSEEHFRAME_H
