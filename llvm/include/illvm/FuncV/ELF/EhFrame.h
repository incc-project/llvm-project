//===--- EhFrame.h - ELF exception frame ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ELF exception frame.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_EHFRAME_H
#define ILLVM_EHFRAME_H

#include <memory>
#include <string>
#include <vector>

#include "illvm/FuncV/ELF/Relocation.h"

namespace illvm {
namespace funcv {
namespace elf {

// Frame Description Entry.
class FDE {
  uint32_t length;
  uint64_t extLength;
  // Offset (do not include length and extLength), update while writing.
  uint32_t ciePointer;
  const char *otherData;
  // Update rela.r_offset when writing.
  // For convenience, we will set the r_offset relative to the FDE itself
  // during parseReference, and update it to the global offset during layout.
  std::shared_ptr<Relocation> pcBeginRelaEntry;
  // Update rela.r_offset when writing.
  // For convenience, we will set the r_offset relative to the FDE itself
  // during parseReference, and update it to the global offset during layout.
  std::vector<std::shared_ptr<Relocation>> otherRelaEntries;

public:
  FDE(const uint32_t _length, const uint64_t _extLength,
      const uint32_t _ciePointer, const char *_otherData)
      : length(_length), extLength(_extLength), ciePointer(_ciePointer),
        otherData(_otherData) {}

  uint32_t getLength() const { return length; }

  uint64_t getExtLength() const { return extLength; }

  uint64_t getSizeExcludeLength() const {
    if (length == 0xffffffff) {
      return extLength;
    }
    return length;
  }

  uint32_t getCIEPointerPreFilling() const {
    if (length == 0xffffffff) {
      return sizeof(uint32_t) + sizeof(uint64_t);
    }
    return sizeof(uint32_t);
  }

  uint64_t getSize() const {
    if (length == 0xffffffff) {
      return extLength + sizeof(uint32_t) + sizeof(uint64_t);
    }
    return length + sizeof(uint32_t);
  }

  void setCIEPointer(const uint32_t _ciePointer) { ciePointer = _ciePointer; }

  uint32_t getCIEPointer() const { return ciePointer; }

  const char *getOtherData() const { return otherData; }

  uint64_t getOtherDataSize() const {
    return getSizeExcludeLength() - sizeof(uint32_t);
  }

  uint32_t getPCBeginPreFilling() const {
    return getCIEPointerPreFilling() + sizeof(uint32_t);
  }

  void setPCBeginRelaEntry(const std::shared_ptr<Relocation> &entry) {
    pcBeginRelaEntry = entry;
  }

  std::shared_ptr<Relocation> getPCBeginRelaEntry() const {
    return pcBeginRelaEntry;
  }

  void addOtherRelaEntry(const std::shared_ptr<Relocation> &entry) {
    otherRelaEntries.push_back(entry);
  }

  auto &getOtherRelaEntries() const { return otherRelaEntries; }

  void writeDataTo(char *buffer) const;

  void dump(std::ostream &oss) const;

  std::string toString() const;
};

// Common Information Entry.
class CIE {
  uint32_t length;
  uint64_t extLength;
  uint32_t cieID;
  const char *otherData;

  // Update rela.r_offset when writing.
  // For convenience, we will set the r_offset relative to the FDE itself
  // during parseReference, and update it to the global offset during layout.
  std::vector<std::shared_ptr<Relocation>> relaEntries;

public:
  CIE(const uint32_t _length, const uint64_t _extLength, const uint32_t _cieID,
      const char *_otherData)
      : length(_length), extLength(_extLength), cieID(_cieID),
        otherData(_otherData) {}

  uint32_t getLength() const { return length; }

  uint64_t getExtLength() const { return extLength; }

  uint64_t getSizeExcludeLength() const {
    if (length == 0xffffffff) {
      return extLength;
    }
    return length;
  }

  uint64_t getSize() const {
    if (length == 0xffffffff) {
      return extLength + sizeof(uint32_t) + sizeof(uint64_t);
    }
    return length + sizeof(uint32_t);
  }

  uint32_t getCIEID() const { return cieID; }

  const char *getOtherData() const { return otherData; }

  uint64_t getOtherDataSize() const {
    return getSizeExcludeLength() - sizeof(uint32_t);
  }

  void addRelaEntry(const std::shared_ptr<Relocation> &entry) {
    relaEntries.push_back(entry);
  }

  auto &getRelaEntries() const { return relaEntries; }

  void writeDataTo(char *buffer) const;

  void dump(std::ostream &oss) const;

  std::string toString() const;
};

// Call Frame Information Format.
class CFI {
  std::shared_ptr<CIE> cie;
  std::vector<std::shared_ptr<FDE>> fdes;

public:
  explicit CFI(const std::shared_ptr<CIE> &_cie) : cie(_cie) {}

  std::shared_ptr<CIE> getCIE() const { return cie; }

  void addFDE(const std::shared_ptr<FDE> &_fde) { fdes.push_back(_fde); }

  auto &getFDEs() { return fdes; }

  uint64_t writeDataTo(char *buffer) const;

  void dump(std::ostream &oss) const;

  std::string toString() const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_EHFRAME_H