#include "illvm/FuncV/ELF/Relocation.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

void Relocation::writeDataTo(llvm::object::ELF64LE::Rela *rela) const {
  rela->r_offset = r_offset;
  rela->r_info = r_info;
  rela->r_addend = r_addend;
}

void Relocation::dump(std::ostream &oss) const {
  oss << std::hex;
  oss << std::setfill('0');
  oss << std::setw(16) << r_offset << " ";

  oss << std::setw(16) << r_info << " ";

  oss << std::setfill(' ');
  oss << std::setw(20) << TypeToString::relaTypeToString(getTypeInfo())
      << " ";

  oss << std::setfill('0');
  oss << std::setw(16) << sym->getStValue() << " ";

  oss << std::setfill(' ');
  oss << std::setw(20) << sym->getName()->getValue() << " ";

  if (r_addend < 0) {
    oss << "- " << -r_addend;
  } else {
    oss << "+ " << r_addend;
  }

  oss << std::dec;

  oss << "    (offset IdxRef: " << offset->getValue() << ")";
}

std::string Relocation::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
