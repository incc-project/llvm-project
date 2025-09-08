#include "illvm/FuncV/ELF/Section.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

void Section::writeHeaderTo(llvm::object::ELF64LE::Shdr *shdr) const {
  shdr->sh_name = sh_name;
  shdr->sh_type = sh_type;
  shdr->sh_flags = sh_flags;
  shdr->sh_addr = sh_addr;
  shdr->sh_offset = sh_offset;
  shdr->sh_size = sh_size;
  shdr->sh_link = sh_link;
  shdr->sh_info = sh_info;
  shdr->sh_addralign = sh_addralign;
  shdr->sh_entsize = sh_entsize;
}

void Section::writeDataTo(char *buffer) { memcpy(buffer, data, sh_size); }

void Section::dumpHeader(std::ostream &oss) const {
  oss << std::setfill(' ');
  oss << "[" << std::setw(5) << idx->getValue() << "] ";
  oss << std::setw(20) << name->getValue() << " ";
  oss << std::setw(20) << TypeToString::sectionTypeToString(sh_type) << " ";
  oss << std::hex;
  oss << std::setfill('0');
  oss << std::setw(6) << sh_offset << " ";
  oss << std::setw(6) << sh_size << " ";
  oss << std::setw(4) << sh_entsize << " ";
  oss << std::dec;
  oss << std::setfill(' ');
  oss << std::setw(4) << TypeToString::sectionFlagToString(sh_flags) << " ";
  oss << std::setw(5) << sh_link << " ";
  oss << std::setw(4) << sh_info << " ";
  oss << std::setw(4) << sh_addralign;

  oss << "    " << "(link IdxRef: ";
  if (link == nullptr) {
    oss << "NULL";
  } else {
    oss << link->getValue();
  }
  oss << ", info IdxRef: ";
  if (infoLink == nullptr) {
    oss << "NULL";
  } else {
    oss << infoLink->getValue();
  }
  oss << ")";
}

std::string Section::headerToString() const {
  std::stringstream oss;
  dumpHeader(oss);
  return oss.str();
}

void Section::dumpData(std::ostream &oss) const {
  oss << std::hex;
  oss << std::setfill('0');
  for (uint64_t i = 0; i < sh_size; i += 16) {
    if (i != 0) {
      oss << std::endl;
    }
    // Address.
    oss << "0x" << std::setw(10) << i << ":";
    // Hex data.
    uint64_t j = 0;
    for (; j < 16 && i + j < sh_size; ++j) {
      const uint8_t dataV = data[i + j];
      oss << " " << std::setw(2) << static_cast<unsigned>(dataV);
    }
    // Align.
    for (; j < 16; ++j) {
      oss << "   ";
    }
    oss << " ";
    // String data.
    j = 0;
    for (; j < 16 && i + j < sh_size; ++j) {
      const uint8_t dataV = data[i + j];
      if (std::isprint(dataV)) {
        oss << dataV;
      } else {
        oss << ".";
      }
    }
  }
  oss << std::dec;
  std::setfill(' ');
  oss << std::endl;
}

std::string Section::dataToString() const {
  std::stringstream oss;
  dumpData(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
