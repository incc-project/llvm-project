#ifndef ILLVM_DEBUGRNGLISTSECTION_H
#define ILLVM_DEBUGRNGLISTSECTION_H

#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

#include <map>

namespace illvm {
namespace funcv {
namespace elf {

class DebugRnglistSection final : public Section {
private:
  // Ref : 7.28
  struct RnglistEntry {
    uint8_t kind;
    uint64_t value0;
    uint64_t value1;
  };

  struct RnglistHeader {
    uint32_t unit_length;
    uint16_t version;
    uint8_t addr_size;
    uint8_t seg_size;
    uint32_t offset_entry_count;
  };

  RnglistHeader header{};
  std::vector<uint32_t> offsets;
  std::map<uint32_t, std::vector<RnglistEntry>> rangeLists;
  //  const DebugAddrSection &debugAddr;
  const DebugAddrSection *debugAddr;
  uint64_t baseOffset;
  mutable std::unordered_map<uint32_t, uint64_t> contextMap;

public:
  DebugRnglistSection(const llvm::object::ELF64LE::Shdr *shdr,
                      const char *_data, const DebugAddrSection *addrRef);

  void dumpData(std::ostream &oss) const override;

  void registerAddrBase(uint32_t index, uint64_t addrBaseOffset) const;

  void writeDataTo(char *buffer) override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGRNGLISTSECTION_H
