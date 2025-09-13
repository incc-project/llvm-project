#ifndef ILLVM_DEBUGABBREVSECTION_H
#define ILLVM_DEBUGABBREVSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

#include <map>

namespace illvm {
namespace funcv {
namespace elf {

class DebugAbbrevSection final : public Section {
public:
  // Structure representing an attribute-form pair in an abbreviation
  // declaration
  struct AttributeForm {
    uint64_t attr = 0;                        // DWARF attribute code
    uint64_t form = 0;                        // DWARF form code
    int64_t implicitConst = 0;
  };

  // Structure representing an abbreviation declaration
  struct AbbreviationDecl {
    uint64_t code = 0;                        // Abbreviation code
    uint64_t tag = 0;                         // DWARF tag
    bool hasChildren = false;                     // Whether this DIE has children
    std::vector<AttributeForm> attrForms; // List of attribute-form pairs
  };

  // Map of abbreviation table
  std::vector<AbbreviationDecl> abbrevTable;

  DebugAbbrevSection(const llvm::object::ELF64LE::Shdr *shdr,
                     const char *_data);

  void dumpData(std::ostream &oss) const override;

  void writeDataTo(char *buffer) override;

  // Get abbreviation declaration by code
  const AbbreviationDecl &getAbbreviationDecl(uint64_t code) const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGABBREVSECTION_H
