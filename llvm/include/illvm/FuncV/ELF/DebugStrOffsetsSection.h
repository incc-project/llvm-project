#ifndef ILLVM_DEBUGSTROFFSETSSECTION_H
#define ILLVM_DEBUGSTROFFSETSSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"
#include "illvm/FuncV/ELF/Relocation.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugStrOffsetsSection final : public Section {
private:
  const DebugStrSection &debugStr;

  // TODO: block struct -> rela. Note: debug_info -> block ref.
  // Structure representing a string offsets table
  struct StringOffsetsTable {
    uint32_t unitLength;
    uint16_t version;
    uint16_t padding; // always 0
  };

  std::vector<StringOffsetsTable> tables;
  std::vector<uint64_t> offsetBases;
  std::vector<uint64_t> sizes;
  std::vector<uint64_t> headerSizes;
  std::vector<std::vector<uint32_t>> allOffsets;

public:
  DebugStrOffsetsSection(const llvm::object::ELF64LE::Shdr *shdr,
                         const char *_data, const DebugStrSection &strRef);

  // Apply relocations to string offsets
  void applyRelocations(const std::vector<std::shared_ptr<Relocation>> &relocs);

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  // Get table index by base offset
  int getTableIndex(uint64_t baseOffset) const;

  // Get string offset by table index and string index
  uint32_t getStringOffset(int tableIndex, uint32_t strxIndex) const;

  // Get string by table index and string index
  std::string getStringFromStrx(int tableIndex, uint32_t strxIndex) const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGSTROFFSETSSECTION_H
