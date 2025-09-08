//===--- Type.h - ELF type -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ELF type.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_TYPE_H
#define ILLVM_TYPE_H

#include <string>

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Object/ELFTypes.h"

namespace illvm {
namespace funcv {
namespace elf {

constexpr int WordSize = 8;

enum class ELFKind : uint8_t {
  ELFNoneKind,
  ELF32LEKind,
  ELF32BEKind,
  ELF64LEKind,
  ELF64BEKind
};

enum class SectionType {
  Ordinary,
  StrTab,
  SymTab,
  SymTabShNdx,
  RelaTab,
  Group,
  EhFrame,
};

// Refer to llvm/include/llvm/BinaryFormat/ELF.h
class TypeToString {
public:
  static std::string elfMachineToString(const uint16_t machine);

  static std::string sectionFlagToString(const uint64_t flag);

  static std::string sectionTypeToString(const uint32_t type);

  static std::string symbolTypeToString(const unsigned char type);

  static std::string symbolBindToString(const unsigned char type);

  static std::string symbolVisToString(const unsigned char type);

  static std::string symbolSShndxToString(const int type);

  static std::string relaTypeToString(const uint32_t type);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_TYPE_H
