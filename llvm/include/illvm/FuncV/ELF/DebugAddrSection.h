#ifndef ILLVM_DEBUGADDRSECTION_H
#define ILLVM_DEBUGADDRSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Relocation.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugAddrSection final : public Section {
private:
  // Structure representing an address table
  struct AddrTable {
    uint32_t unitLength;
    uint16_t version; // DWARF version
    uint8_t addrSize; // Address size in bytes
    uint8_t segSize;  // Segment selector size
  };

  std::vector<AddrTable> tables;     // List of address tables
  std::vector<uint64_t> offsetBases; // 每个表的 offsetBase
  std::vector<uint64_t> sizes;       // 每个表的 size（unitLength + 4）
  std::vector<uint64_t> headerSizes; // 每个表头部长度
  std::vector<std::vector<uint64_t>> allAddresses; // 每个表的地址列表

public:
  DebugAddrSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data);

  // Apply relocations to address entries
  void applyRelocations(const std::vector<std::shared_ptr<Relocation>> &relocs);

  // Get address by table base offset and index
  uint64_t getAddressByIndex(uint64_t baseOffset, uint64_t index) const;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGADDRSECTION_H
