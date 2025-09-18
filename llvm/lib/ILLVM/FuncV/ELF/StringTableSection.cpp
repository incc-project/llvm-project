#include "illvm/FuncV/ELF/StringTableSection.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

StringTableSection::StringTableSection(const llvm::object::ELF64LE::Shdr *shdr,
                                       const char *_data, llvm::Error &err)
    : Section(SectionType::StrTab, shdr, _data) {
  const auto firstStrRef = std::make_shared<StrRef>(data, 0);
  originalIndexes[0] = firstStrRef;
  strs.push_back(firstStrRef);
  for (uint64_t i = 0; i < sh_size; i++) {
    if (data[i] == '\0' && i + 1 < sh_size) {
      const auto strRef = std::make_shared<StrRef>(data + i + 1, i + 1);
      originalIndexes[i + 1] = strRef;
      strs.push_back(strRef);
    }
  }
}

std::shared_ptr<StrRef>
StringTableSection::parseOriginalIndex(uint64_t strOff) {
  const auto it = originalIndexes.find(strOff);
  if (it != originalIndexes.end()) {
    return it->second;
  }
  // Handle string overlap (compression).
  const auto strRef = std::make_shared<StrRef>(data + strOff, strOff);
  originalIndexes[strOff] = strRef;
  strs.push_back(strRef);
  return strRef;
}

void StringTableSection::layout() {
  uint64_t strOff = 0;
  sh_size = 0;
  for (const auto &strRef : strs) {
    strRef->setOffset(strOff);
    strOff += strRef->getLength() + 1;
    sh_size += strRef->getLength() + 1;
  }
}

void StringTableSection::writeDataTo(char *buffer) {
  uint64_t strOff = 0;
  for (const auto &strRef : strs) {
    const uint64_t length = strRef->getLength() + 1;
    memcpy(buffer + strOff, strRef->getValue().data(), length);
    strOff += length;
  }
}

void StringTableSection::dumpData(std::ostream &oss) const {
  for (size_t i = 0; i < strs.size(); i++) {
    oss << "[" << i << "] \"";
    strs[i]->dump(oss);
    oss << "\"" << std::endl;
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
