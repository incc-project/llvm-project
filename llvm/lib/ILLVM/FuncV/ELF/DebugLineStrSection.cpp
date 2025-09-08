#include "illvm/FuncV/ELF/DebugLineStrSection.h"

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

DebugLineStrSection::DebugLineStrSection(
    const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
    : Section(SectionType::DebugLineStr, shdr, _data) {
  const char *start = data;
  const char *end = data + sh_size;
  uint64_t offset = 0;

  while (start < end) {
    const char *s = start;
    size_t len = strlen(s);
    if (len == 0) {
      ++start;
      ++offset;
      continue;
    }
    strings.push_back({offset, std::string(s)});
    start += len + 1;
    offset += len + 1;
  }
}

void DebugLineStrSection::writeDataTo(char *buffer) {
  std::vector<uint8_t> out;

  for (const auto &entry : strings) {
    for (char c : entry.str) {
      out.push_back(static_cast<uint8_t>(c));
    }
    out.push_back(0);
  }

  assert(out.size() <= sh_size && "Rewritten .debug_str larger than original");
  memcpy(buffer, out.data(), out.size());
}

void DebugLineStrSection::dumpData(std::ostream &oss) const {
  oss << ".debug_line_str contents:\n";
  for (const auto &entry : strings) {
    oss << "0x" << DebugConvert::intToHex(entry.offset, 8) << ": \""
        << entry.str << "\"\n";
  }
}

// Get string by offset
std::string DebugLineStrSection::getString(uint32_t offset) const {
  if (offset >= sh_size)
    return "<invalid offset>";
  return std::string(data + offset);
}

} // namespace elf
} // namespace funcv
} // namespace illvm
