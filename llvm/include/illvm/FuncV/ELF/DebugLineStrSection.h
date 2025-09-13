#ifndef ILLVM_DEBUGLINESTRSECTION_H
#define ILLVM_DEBUGLINESTRSECTION_H

#include <unordered_map>

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

// Immutable
class DebugLineStrSection final : public Section {
private:
  std::vector<std::shared_ptr<DebugStrRef>> strs; // List of string entries

  // offset -> strRef.
  // It can only work during parsing.
  std::unordered_map<uint64_t, std::shared_ptr<DebugStrRef>> originalIndexes;

public:
  DebugLineStrSection(const llvm::object::ELF64LE::Shdr *shdr,
                      const char *_data);

  // TODO: use this function!
  std::shared_ptr<DebugStrRef> parseOriginalIndex(uint64_t strOff);

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGLINESTRSECTION_H
