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

  DebugLoclistsSection(const llvm::object::ELF64LE::Shdr *shdr,
                       const char *_data, llvm::Error &err);

public:

  static llvm::Expected<std::shared_ptr<DebugLoclistsSection>>
    Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<DebugLoclistsSection>(
        new DebugLoclistsSection(shdr, _data, err));
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

#endif // ILLVM_DEBUGLOCLISTSSECTION_H
