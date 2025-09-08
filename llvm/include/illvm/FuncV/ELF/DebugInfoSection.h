#ifndef ILLVM_DEBUGINFOSECTION_H
#define ILLVM_DEBUGINFOSECTION_H

#include "illvm/FuncV/ELF/DebugAbbrevSection.h"
#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugRnglistSection.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

// Ref 7.5
class DebugInfoSection final : public Section {
private:
  // Structure representing a compile unit header
  struct CompileUnitHeader {
    uint64_t offset;               // Offset in section
    uint32_t unitLength;           // Unit length
    uint16_t version;              // DWARF version
    uint8_t unitType;              // Unit type
    uint8_t addrSize;              // Address size
    uint32_t abbrevOffset;         // Abbreviation offset
    std::optional<uint64_t> dwoId; // Optional DWO ID
  };

  // Structure representing a DIE (Debugging Information Entry)
  struct DIE {
    uint64_t offset;     // Offset in section
    uint64_t abbrevCode; // Abbreviation code
    const DebugAbbrevSection::AbbreviationDecl
        *abbrevDecl; // Abbreviation declaration
    std::vector<std::pair<uint64_t, FormValueRaw>> attributes; // Attributes
    std::vector<DIE> children;                                 // Child DIEs
  };

  std::vector<CompileUnitHeader> cuHeaders; // Compile unit headers
  std::vector<std::vector<DIE>> cuDIEs; // Array of top-level DIEs for each CU
  const DebugAbbrevSection &abbrev;
  const DebugStrSection *debugStr;
  const DebugStrOffsetsSection *debugStrOffset;
  const DebugAddrSection *debugAddr;
  const DebugRnglistSection *debugRnglist;

public:
  DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                   const DebugAbbrevSection &abbrevRef,
                   const DebugStrSection *strRef,
                   const DebugStrOffsetsSection *strOffsetRef,
                   const DebugAddrSection *addrRef,
                   const DebugRnglistSection *rnglistRef);

  DIE parseDIE(const uint8_t *&p, const uint8_t *start, const uint8_t *end,
               uint32_t abbrevOffset, int strOffsetsTableIndex = -1,
               uint64_t addrBaseOffset = 0);

  void dumpData(std::ostream &oss) const override;

  void dumpDIE(std::ostream &oss, const DIE &die) const;

  void writeDataTo(char *buffer) override;

  void writeDIE(const DIE &die, std::vector<uint8_t> &out) const;

  FormValueRaw parseFormValue(
      uint64_t form, const uint8_t *&p, const uint8_t *end,
      const DebugStrOffsetsSection *strOffsets = nullptr,
      const DebugStrSection *strSection = nullptr,
      const DebugAddrSection *addrSection = nullptr, uint64_t dieOffset = 0,
      int strOffsetsTableIndex = -1, uint64_t addrBaseOffset = 0,
      std::optional<int64_t> implicitConst = std::nullopt);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGINFOSECTION_H
