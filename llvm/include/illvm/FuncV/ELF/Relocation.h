//===--- Relocation.h - ELF relocation -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ELF relocation.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_RELOCATION_H
#define ILLVM_RELOCATION_H

#include "illvm/FuncV/ELF/Reference.h"
#include "illvm/FuncV/ELF/Symbol.h"
#include "illvm/FuncV/ELF/Type.h"

namespace illvm {
namespace funcv {
namespace elf {

class Relocation {
private:
  // Update while writing, according to offset.
  uint64_t r_offset;
  std::shared_ptr<IdxRef> offset;
  // Update while writing, according to sym.
  uint64_t r_info;
  std::shared_ptr<Symbol> sym;
  int64_t r_addend;

public:
  Relocation(const uint64_t r_offset, const std::shared_ptr<IdxRef> &_offset,
             const uint64_t r_info, const std::shared_ptr<Symbol> &sym,
             const int64_t r_addend)
      : r_offset(r_offset), offset(_offset), r_info(r_info), sym(sym),
        r_addend(r_addend) {}

  explicit Relocation(const llvm::object::ELF64LE::Rela *elf_rela)
      : Relocation(elf_rela->r_offset, nullptr, elf_rela->r_info, nullptr,
                   elf_rela->r_addend) {}

  uint64_t getROffset() const { return r_offset; }

  void setROffset(const uint64_t _r_offset) { r_offset = _r_offset; }

  std::shared_ptr<IdxRef> getOffset() const { return offset; }

  uint64_t getOffsetValue() const { return offset->getValue(); }

  void setOffset(const std::shared_ptr<IdxRef> &_offset) { offset = _offset; }

  uint64_t getRInfo() const { return r_info; }

  void setRInfo(const uint64_t _r_info) { r_info = _r_info; }

  uint64_t getSymbolInfo() const { return r_info >> 32; }

  void setSymbolInfo(const uint64_t symbolInfo) {
    r_info &= 0xffffffff;
    r_info |= (symbolInfo << 32);
  }

  uint64_t getTypeInfo() const { return r_info & 0x0ff; }

  std::shared_ptr<Symbol> getSym() const { return sym; }

  void setSym(const std::shared_ptr<Symbol> &_sym) { sym = _sym; }

  uint64_t getRAddend() const { return r_addend; }

  void setRAddend(const int64_t _addend) { r_addend = _addend; }

  void writeDataTo(llvm::object::ELF64LE::Rela *rela) const;

  void dump(std::ostream &oss) const;

  std::string toString() const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_RELOCATION_H
