//===--- FuncV.h - FuncV -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Workflow:
// * Init old/new obj file.
// * Reuse binary code form old obj file to new obj file according to funcXSet.
// * Fini new obj file.
// * Write new obj file.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_FUNCV_H
#define ILLVM_FUNCV_H

#include <string>
#include <unordered_set>

#include "illvm/FuncV/ELF/ObjFile.h"
#include "illvm/FuncV/ELF/Reuse.h"

namespace illvm {
namespace funcv {
namespace elf {

class FuncV {
private:
  static bool startsWith(const char *str, const char *prefix);

  static std::pair<unsigned char, unsigned char>
  getElfArchType(const BinFile &binFile);

  static ELFKind getELFKind(const BinFile &binFile);

public:
  // Load symbol table for FuncX.
  static std::unordered_set<std::string>
  onlyLoadSymbolTable(const std::string &objPath);

  static void run(const std::string &oldObjPath, const std::string &newObjPath,
                  const std::string &outputPath,
                  const std::unordered_set<std::string> &funcXSet);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_FUNCV_H
