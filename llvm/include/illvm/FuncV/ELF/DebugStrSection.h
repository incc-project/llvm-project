#ifndef ILLVM_DEBUGSTRSECTION_H
#define ILLVM_DEBUGSTRSECTION_H

#include <unordered_map>
#include <memory>

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugStrSection final : public Section {
private:
  std::vector<std::shared_ptr<DebugStrRef>> strs; // List of string entries

  // offset -> strRef.
  // It can only work during parsing.
  std::unordered_map<uint64_t, std::shared_ptr<DebugStrRef>> originalIndexes;

  DebugStrSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data, llvm::Error &err);

public:

  static llvm::Expected<std::shared_ptr<DebugStrSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section =
        std::shared_ptr<DebugStrSection>(new DebugStrSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  // TODO: use this function!
  std::shared_ptr<DebugStrRef> parseOriginalIndex(uint64_t strOff);

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  // Add an existed str ref to strtab.
  void push_back(const std::shared_ptr<DebugStrRef> &strRef) { strs.push_back(strRef); }

  // Add a new str to strtab.
  std::shared_ptr<DebugStrRef> push_back(const std::string &str) {
    const auto offset = std::make_shared<IdxRef>(0);
    const auto strRef = std::make_shared<DebugStrRef>(offset, str);
    strs.push_back(strRef);
    return strRef;
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGSTRSECTION_H
