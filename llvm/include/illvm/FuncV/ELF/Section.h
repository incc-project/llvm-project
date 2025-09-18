//===--- Section.h - ELF section -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ELF section.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_SECTION_H
#define ILLVM_SECTION_H

#include <memory>

#include "illvm/FuncV/ELF/Reference.h"
#include "illvm/FuncV/ELF/Type.h"

namespace illvm {
namespace funcv {
namespace elf {

class Section {
protected:
  SectionType type;

  // Update while writing, according to name.
  uint32_t sh_name;
  std::shared_ptr<StrRef> name;
  uint32_t sh_type;
  uint64_t sh_flags;
  // Always 0.
  uint64_t sh_addr;
  // Update while writing.
  uint64_t sh_offset;
  // Update while writing.
  uint64_t sh_size;
  // Update while writing, according to link.
  uint32_t sh_link;
  // May be nullptr.
  std::shared_ptr<IdxRef> link;
  uint32_t sh_info;
  // Refer to
  // https://docs.oracle.com/cd/E26502_01/html/E26507/chapter6-94076.html#chapter6-47976
  // . May be nullptr.
  std::shared_ptr<IdxRef> infoLink;
  uint64_t sh_addralign;
  uint64_t sh_entsize;

  // Section data.
  // Important: Once data is created, it cannot be modified.
  // We will recreate a new buffer of data while writing.
  const char *data;

  // The index of this section in section header table，
  // update while writing.
  std::shared_ptr<IdxRef> idx;

  Section(const SectionType _type, const uint32_t _sh_name,
          const std::shared_ptr<StrRef> &_name, const uint32_t _sh_type,
          const uint64_t _sh_flags, const uint64_t _sh_addr,
          const uint64_t _sh_offset, const uint64_t _sh_size,
          const uint32_t _sh_link, const std::shared_ptr<IdxRef> &_link,
          const uint32_t _sh_info, const std::shared_ptr<IdxRef> &_info_link,
          const uint64_t _sh_addralign, const uint64_t _sh_entsize,
          const char *_data, const std::shared_ptr<IdxRef> &_idx)
      : type(_type), sh_name(_sh_name), name(_name), sh_type(_sh_type),
        sh_flags(_sh_flags), sh_addr(_sh_addr), sh_offset(_sh_offset),
        sh_size(_sh_size), sh_link(_sh_link), link(_link), sh_info(_sh_info),
        infoLink(_info_link), sh_addralign(_sh_addralign),
        sh_entsize(_sh_entsize), data(_data), idx(_idx) {}

  Section(const SectionType _type, const llvm::object::ELF64LE::Shdr *shdr,
          const char *_data)
      : Section(_type, shdr->sh_name, nullptr, shdr->sh_type, shdr->sh_flags,
                shdr->sh_addr, shdr->sh_offset, shdr->sh_size, shdr->sh_link,
                nullptr, shdr->sh_info, nullptr, shdr->sh_addralign,
                shdr->sh_entsize, _data, nullptr) {}

public:
  virtual ~Section() = default;

  SectionType getType() const { return type; }

  uint64_t getShName() const { return sh_name; }

  void setShName(const uint64_t _sh_name) { sh_name = _sh_name; }

  std::shared_ptr<StrRef> getName() const { return name; }

  std::string getNameValue() const { return name->getValue(); }

  void setName(const std::shared_ptr<StrRef> &_name) { name = _name; }

  uint64_t getShType() const { return sh_type; }

  uint64_t getShFlags() const { return sh_flags; }

  uint64_t getShOffset() const { return sh_offset; }

  void setShOffset(const uint64_t offset) { sh_offset = offset; }

  uint64_t getShSize() const { return sh_size; }

  void setShSize(const uint64_t size) { sh_size = size; }

  uint64_t getShLink() const { return sh_link; }

  void setShLink(const uint64_t _sh_link) { sh_link = _sh_link; }

  std::shared_ptr<IdxRef> getLink() const { return link; }

  void setLink(const std::shared_ptr<IdxRef> &_link) { link = _link; }

  uint32_t getShInfo() const { return sh_info; }

  void setShInfo(const uint32_t _sh_info) { sh_info = _sh_info; }

  std::shared_ptr<IdxRef> getInfoLink() const { return infoLink; }

  void setInfoLink(const std::shared_ptr<IdxRef> &_info_link) {
    infoLink = _info_link;
  }

  uint64_t getShAddralign() const { return sh_addralign; }

  uint64_t getEntSize() const { return sh_entsize; }

  const char *getData() const { return data; }

  void setData(const char *_data) { data = _data; }

  std::shared_ptr<IdxRef> getIdx() const { return idx; }

  uint64_t getIdxValue() const { return idx->getValue(); }

  void setIdx(const std::shared_ptr<IdxRef> &_idx) { idx = _idx; }

  // Update order, idx, offset, size.
  virtual void layout() {}

  virtual void fini() {}

  void writeHeaderTo(llvm::object::ELF64LE::Shdr *shdr) const;

  virtual void writeDataTo(char *buffer);

  void dumpHeader(std::ostream &oss) const;

  std::string headerToString() const;

  virtual void dumpData(std::ostream &oss) const;

  std::string dataToString() const;
};

class OrdinarySection final : public Section {
private:
  OrdinarySection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                  llvm::Error &err)
      : Section(SectionType::Ordinary, shdr, _data) {}

public:
  static llvm::Expected<std::shared_ptr<OrdinarySection>>
  Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section =
        std::shared_ptr<OrdinarySection>(new OrdinarySection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_SECTION_H
