//===--- Reuse.hpp - FuncV reuse ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Workflow:
// * Build BDG.
// * Reuse sections.
// * Reuse symbols.
// * Reuse eh_frame.
// * Reuse relocations.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_REUSE_H
#define ILLVM_REUSE_H

#include "illvm/FuncV/ELF/BDG.h"

namespace illvm {
namespace funcv {
namespace elf {

class Reuse {
  ObjFile &oldObjFile;
  ObjFile &newObjFile;
  const std::unordered_set<std::string> &funcXSet;

public:
  Reuse(ObjFile &_oldObjFile, ObjFile &_newObjFile,
        const std::unordered_set<std::string> &_funcXSet)
      : oldObjFile(_oldObjFile), newObjFile(_newObjFile), funcXSet(_funcXSet) {}

  void run();
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_REUSE_H
