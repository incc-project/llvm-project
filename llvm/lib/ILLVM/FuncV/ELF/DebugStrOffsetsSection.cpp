#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

DebugStrOffsetsSection::DebugStrOffsetsSection(
    const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
    const std::shared_ptr<DebugStrSection> &debugStrSection, llvm::Error &err)
    : Section(SectionType::DebugStrOffsets, shdr, _data) {
  // Ref 7.26
  // Read header.
  ILLVM_FCHECK(sizeof(StringOffsetsHeader) <= sh_size, "");

  const auto *hdr = reinterpret_cast<const StringOffsetsHeader *>(data);
  ILLVM_FCHECK(hdr->unitLength == sh_size - sizeof(uint32_t), "");

  header.unitLength = hdr->unitLength;
  header.version = hdr->version;
  header.padding = hdr->padding;

  ILLVM_FCHECK(
      (sh_size - sizeof(StringOffsetsHeader)) % sizeof(OffsetESType) == 0, "");

  // Read offsets.
  for (uint64_t i = sizeof(StringOffsetsHeader), idx = 0; i < sh_size;
       i += sizeof(OffsetESType), idx += 1) {
    const auto offsetES = reinterpret_cast<const OffsetESType *>(data + i);
    offsets.emplace_back(std::make_shared<DebugStrOffsetRef>(
        std::make_shared<IdxRef>(idx),
        debugStrSection->parseOriginalIndex(*offsetES)));
  }
}

std::shared_ptr<DebugStrOffsetRef>
DebugStrOffsetsSection::getStrOffsetRef(const uint32_t idx) {
  return offsets[idx];
}

void DebugStrOffsetsSection::layout() {
  sh_size = sizeof(StringOffsetsHeader) + offsets.size() * sizeof(OffsetESType);
  header.unitLength = sh_size - sizeof(uint32_t);
  for (size_t i = 0; i < offsets.size(); i += 1) {
    offsets[i]->idx->setValue(i);
  }
}

void DebugStrOffsetsSection::writeDataTo(char *buffer) {
  uint64_t bufOff = 0;

  auto *hdr = reinterpret_cast<StringOffsetsHeader *>(buffer);
  hdr->unitLength = header.unitLength;
  hdr->version = header.version;
  hdr->padding = header.padding;

  bufOff += sizeof(StringOffsetsHeader);

  for (const auto &offset : offsets) {
    OffsetESType v = offset->strRef->offset->getValue();
    memcpy(buffer + bufOff, &v, sizeof(OffsetESType));
    bufOff += sizeof(OffsetESType);
  }
}

void DebugStrOffsetsSection::dumpData(std::ostream &oss) const {
  oss << ".debug_str_offsets contents:\n";
  oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << 0
      << ": Contribution size = " << std::dec << header.unitLength
      << ", Format = DWARF32"
      << ", Version = " << header.version << "\n";

  for (size_t i = 0; i < offsets.size(); ++i) {
    uint64_t absOffset = sizeof(StringOffsetsHeader) + i * sizeof(OffsetESType);
    uint32_t strOffset = offsets[i]->strRef->offset->getValue();
    std::string str = offsets[i]->strRef->str;

    oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << absOffset
        << ": " << std::setw(8) << std::setfill('0') << strOffset << " \""
        << str << "\"\n";
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
