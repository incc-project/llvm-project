#ifndef ILLVM_DEBUGADDRSECTION_H
#define ILLVM_DEBUGADDRSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Relocation.h"
#include "illvm/FuncV/ELF/RelocationSection.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugAddrSection final : public Section {
private:
  // Structure representing an address table
  struct AddrTableHeader {
    uint32_t unitLength;
    uint16_t version; // DWARF version
    uint8_t addrSize; // Address size in bytes
    uint8_t segSize;  // Segment selector size
  };

  AddrTableHeader header;
  std::vector<std::shared_ptr<DebugAddrRef>> addresses;

  DebugAddrSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                   llvm::Error &err);

public:
  static llvm::Expected<std::shared_ptr<DebugAddrSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<DebugAddrSection>(
        new DebugAddrSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  // TODO call this function in ObjFile
  llvm::Error
  parseReferences(const std::shared_ptr<RelocationSection> &relaSection) const;

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  std::shared_ptr<DebugAddrRef> getDebugAddrRef(const uint64_t idx) const {
    return addresses[idx];
  }

  // Add an existed addr ref to addresses.
  void push_back(const std::shared_ptr<DebugAddrRef> &addrRef) {
    addresses.push_back(addrRef);
  }

  // Add a new addr ref to addresses.
  std::shared_ptr<DebugAddrRef>
  push_back(std::shared_ptr<Relocation> &relaEntry) {
    const auto idx = std::make_shared<IdxRef>(0);
    const auto addrRef = std::make_shared<DebugAddrRef>(idx, relaEntry);
    addresses.push_back(addrRef);
    return addrRef;
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGADDRSECTION_H
