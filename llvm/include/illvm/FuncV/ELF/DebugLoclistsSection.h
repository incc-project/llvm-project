#ifndef ILLVM_DEBUGLOCLISTSSECTION_H
#define ILLVM_DEBUGLOCLISTSSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugLoclistsSection final : public Section {
private:
  struct Header {
    uint32_t length;
    uint16_t version;
    uint8_t addrSize;
    uint8_t segSize;
    uint32_t offsetEntryCount;
  } header;

  std::vector<uint64_t> offsets;

  struct LocEntry {
    uint64_t offset;
    uint8_t kind;
    std::vector<uint64_t> values;
    std::vector<uint8_t> expr;
  };

  std::vector<LocEntry> entries;

public:
  DebugLoclistsSection(const llvm::object::ELF64LE::Shdr *shdr,
                       const char *_data);

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGLOCLISTSSECTION_H
