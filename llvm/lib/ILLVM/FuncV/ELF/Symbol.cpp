#include "illvm/FuncV/ELF/Symbol.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

void Symbol::writeDataTo(llvm::object::ELF64LE::Sym *sym) const {
  sym->st_name = st_name;
  sym->st_value = st_value;
  sym->st_size = st_size;
  sym->st_info = st_info;
  sym->st_other = st_other;
  sym->st_shndx = st_shndx;
}

void Symbol::dump(std::ostream &oss) const {
  oss << std::setfill(' ');
  oss << "[" << std::setw(5) << idx->getValue() << "] ";

  oss << std::hex;
  oss << std::setfill('0');
  oss << std::setw(16) << st_value << " ";

  oss << std::dec;
  oss << std::setfill(' ');
  oss << std::setw(6) << st_size << " ";

  oss << std::setw(8) << TypeToString::symbolTypeToString(getStType()) << " ";
  oss << std::setw(8) << TypeToString::symbolBindToString(getStBind()) << " ";
  oss << std::setw(8) << TypeToString::symbolVisToString(st_other) << " ";

  if (specialShndx == -1) {
    oss << std::setw(8) << st_shndx << " ";
  } else {
    oss << std::setw(8) << TypeToString::symbolSShndxToString(specialShndx)
        << " ";
  }

  oss << std::setw(20) << name->getValue() << " ";

  oss << "    (value IdxRef: " << value->getValue() << ")";
}

std::string Symbol::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm