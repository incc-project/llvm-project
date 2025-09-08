#ifndef ILLVM_DEBUGLINESTRSECTION_H
#define ILLVM_DEBUGLINESTRSECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugLineStrSection final : public Section {
private:
  // Structure representing a string entry in .debug_str section
  struct StringEntry {
    uint64_t offset; // Offset within the section
    std::string str; // The string content
  };

  std::vector<StringEntry> strings; // List of string entries
public:
  DebugLineStrSection(const llvm::object::ELF64LE::Shdr *shdr,
                      const char *_data);

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  // Get string by offset
  std::string getString(uint32_t offset) const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGLINESTRSECTION_H
