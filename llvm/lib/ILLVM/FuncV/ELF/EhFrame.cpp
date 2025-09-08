#include "illvm/FuncV/ELF/EhFrame.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

void FDE::writeDataTo(char *buffer) const {
  uint64_t offset = 0;
  memcpy(buffer + offset, &length, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  if (length == 0xffffffff) {
    memcpy(buffer + offset, &extLength, sizeof(uint64_t));
    offset += sizeof(uint64_t);
  }
  memcpy(buffer + offset, &ciePointer, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  memcpy(buffer + offset, otherData, getOtherDataSize());
}

void FDE::dump(std::ostream &oss) const {
  oss << length << "(" << extLength << ") " << ciePointer << ": ";
  if (pcBeginRelaEntry != nullptr) {
    oss << pcBeginRelaEntry->toString();
  } else {
    oss << "NULL";
  }
  oss << std::endl << "[otherRelaEntries]:" << std::endl;
  for (const auto &entry : otherRelaEntries) {
    entry->dump(oss);
    oss << std::endl;
  }
}

std::string FDE::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

void CIE::writeDataTo(char *buffer) const {
  uint64_t offset = 0;
  memcpy(buffer + offset, &length, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  if (length == 0xffffffff) {
    memcpy(buffer + offset, &extLength, sizeof(uint64_t));
    offset += sizeof(uint64_t);
  }
  memcpy(buffer + offset, &cieID, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  memcpy(buffer + offset, otherData, getOtherDataSize());
}

void CIE::dump(std::ostream &oss) const {
  oss << length << "(" << extLength << ")";
  oss << std::endl << "[relaEntries]:" << std::endl;
  for (const auto &entry : relaEntries) {
    entry->dump(oss);
    oss << std::endl;
  }
}

std::string CIE::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

uint64_t CFI::writeDataTo(char *buffer) const {
  uint64_t offset = 0;
  cie->writeDataTo(buffer + offset);
  offset += cie->getSize();
  for (const auto &fde : fdes) {
    fde->writeDataTo(buffer + offset);
    offset += fde->getSize();
  }
  return offset;
}

void CFI::dump(std::ostream &oss) const {
  oss << "CIE: " << cie->toString() << std::endl;
  for (size_t i = 0; i < fdes.size(); i++) {
    auto &fde = fdes[i];
    oss << "FDE" << i << ": " << fde->toString() << std::endl;
  }
}

std::string CFI::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm