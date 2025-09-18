#include "illvm/FuncV/ELF/DebugStrSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace illvm {
namespace funcv {
namespace elf {

DebugStrSection::DebugStrSection(const llvm::object::ELF64LE::Shdr *shdr,
                                 const char *_data, llvm::Error &err)
    : Section(SectionType::DebugStr, shdr, _data) {
  const char *start = data;
  const char *end = data + sh_size;
  uint64_t offset = 0;

  while (start < end) {
    const char *s = start;
    const size_t len = strlen(s);
    const auto offsetRef = std::make_shared<IdxRef>(offset);
    const auto strRef =
        std::make_shared<DebugStrRef>(offsetRef, std::string(s));
    strs.push_back(strRef);
    originalIndexes[offset] = strRef;
    start += len + 1;
    offset += len + 1;
  }
}

std::shared_ptr<DebugStrRef>
DebugStrSection::parseOriginalIndex(uint64_t strOff) {
  const auto it = originalIndexes.find(strOff);
  if (it != originalIndexes.end()) {
    return it->second;
  }
  // Handle string overlap (compression).
  const auto offsetRef = std::make_shared<IdxRef>(strOff);
  const auto strRef =
      std::make_shared<DebugStrRef>(offsetRef, std::string(data + strOff));
  strs.push_back(strRef);
  originalIndexes[strOff] = strRef;
  return strRef;
}

void DebugStrSection::layout() {
  uint64_t strOff = 0;
  sh_size = 0;
  for (const auto &strRef : strs) {
    strRef->offset->setValue(strOff);
    strOff += strRef->str.length() + 1;
    sh_size += strRef->str.length() + 1;
  }
}

void DebugStrSection::writeDataTo(char *buffer) {
  uint64_t strOff = 0;
  for (const auto &strRef : strs) {
    const uint64_t length = strRef->str.length() + 1;
    std::string temp;
    memcpy(buffer + strOff, strRef->str.data(), length);
    strOff += length;
  }
}

void DebugStrSection::dumpData(std::ostream &oss) const {
  oss << ".debug_str contents:\n";
  for (const auto &entry : strs) {
    oss << "0x" << DebugConvert::intToHex(entry->offset->getValue(), 8)
        << ": \"" << entry->str << "\"\n";
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
