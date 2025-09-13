#ifndef ILLVM_DEBUGINFOSECTION_H
#define ILLVM_DEBUGINFOSECTION_H

#include "illvm/FuncV/ELF/DebugAbbrevSection.h"
#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugRnglistSection.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"
#include "illvm/FuncV/ELF/FormValue.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

// Ref 7.5
class DebugInfoSection final : public Section {
private:
  // Structure representing a compile unit header
  struct CompileUnitHeader {
    uint32_t unitLength;           // Unit length
    uint16_t version;              // DWARF version
    uint8_t unitType;              // Unit type
    uint8_t addrSize;              // Address size
    uint32_t abbrevOffset;         // Abbreviation offset
  };

  // TODO: DIE class tree.
  // Structure representing a DIE (Debugging Information Entry)
  struct DIE {
    uint64_t abbrevCode = 0; // Abbreviation code
    const DebugAbbrevSection::AbbreviationDecl *abbrevDecl =
        nullptr; // Abbreviation declaration
    // TODO attr map.
    std::vector<std::pair<uint64_t, std::shared_ptr<FormValue>>>
        attributes;            // Attributes
    std::vector<DIE> children; // Child DIEs
  };

  CompileUnitHeader header; // Compile unit headers
  DIE root;

public:
  DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                   FormValueBaseSections &baseSections);

  static DIE parseDIE(const uint8_t *&p, FormValueBaseSections &baseSections);

  void dumpData(std::ostream &oss) const override;

  static void dumpDIE(std::ostream &oss, const DIE &die);

  void writeDataTo(char *buffer) override;

  static void writeDIE(const DIE &die, uint8_t *&out);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGINFOSECTION_H
