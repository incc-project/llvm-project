#ifndef ILLVM_DEBUGARANGESECTION_H
#define ILLVM_DEBUGARANGESECTION_H

#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugArangeSection final : public Section {
private:
  struct ArangeEntry {
    uint64_t address;
    uint64_t length;
  };

  struct ArangeSet {
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_info_offset;
    uint8_t address_size;
    uint8_t segment_size;
    std::vector<ArangeEntry> ranges;
  };

  std::vector<ArangeSet> aranges;

  DebugArangeSection(const llvm::object::ELF64LE::Shdr *shdr,
                       const char *_data, llvm::Error &err);

public:
  static llvm::Expected<std::shared_ptr<DebugArangeSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<DebugArangeSection>(
        new DebugArangeSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGARANGESECTION_H
