//===--- ReuseSection.h - Reuse section -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Reuse text/data section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_REUSESECTION_H
#define ILLVM_REUSESECTION_H

#include "illvm/FuncV/ELF/BDG.h"

namespace illvm {
namespace funcv {
namespace elf {

class ReuseSection {
  static llvm::Expected<std::shared_ptr<Section>>
  createNewSection(ObjFile &newObjFile,
                   const std::shared_ptr<Section> &oldSection);

public:
  static llvm::Error run(ObjFile &newObjFile, const BDG &bdg);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_REUSESECTION_H
