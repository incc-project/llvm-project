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
  using OffsetESType = uint32_t;

  // Structure representing a string offsets table
  struct StringOffsetsHeader {
    uint32_t unitLength;
    uint16_t version;
    uint16_t padding; // always 0
  };

  StringOffsetsHeader header;
  std::vector<std::shared_ptr<DebugStrOffsetRef>> offsets;

  DebugStrOffsetsSection(
      const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
      const std::shared_ptr<DebugStrSection> &debugStrSection, llvm::Error &err);

public:


  static llvm::Expected<std::shared_ptr<DebugStrOffsetsSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data, const std::shared_ptr<DebugStrSection> &debugStrSection) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<DebugStrOffsetsSection>(
        new DebugStrOffsetsSection(shdr, _data, debugStrSection, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  std::shared_ptr<DebugStrOffsetRef> getStrOffsetRef(uint32_t idx);

  void layout() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;

  // Add an existed str offset ref to offsets.
  void push_back(const std::shared_ptr<DebugStrOffsetRef> &strOffsetRef) {
    offsets.push_back(strOffsetRef);
  }

  // Add a new str offset ref to offsets.
  std::shared_ptr<DebugStrOffsetRef> push_back(const std::string &str) {
    const auto idx = std::make_shared<IdxRef>(0);
    const auto offset = std::make_shared<IdxRef>(0);
    const auto strRef = std::make_shared<DebugStrRef>(offset, str);
    const auto strOffRef = std::make_shared<DebugStrOffsetRef>(idx, strRef);
    offsets.push_back(strOffRef);
    return strOffRef;
  }
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGSTROFFSETSSECTION_H
