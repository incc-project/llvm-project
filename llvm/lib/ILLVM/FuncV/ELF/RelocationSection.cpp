#include "illvm/FuncV/ELF/RelocationSection.h"

#include "illvm/Support/Diagnostics.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Logger.h"

#include "llvm/Support/FormatVariadic.h"

namespace illvm {
namespace funcv {
namespace elf {

RelocationSection::RelocationSection(const llvm::object::ELF64LE::Shdr *shdr,
                                     const char *_data, llvm::Error &err)
    : Section(SectionType::RelaTab, shdr, _data) {
  ILLVM_FCHECK(shdr->sh_entsize == sizeof(Elf_Rela), "");
  ILLVM_FCHECK(shdr->sh_size % shdr->sh_entsize == 0, "");

  const size_t relocationNum = shdr->sh_size / shdr->sh_entsize;
  relocations.reserve(relocationNum);

  for (size_t i = 0; i < relocationNum; i++) {
    const auto *rela =
        reinterpret_cast<const Elf_Rela *>(_data + i * shdr->sh_entsize);
    const auto relocation = std::make_shared<Relocation>(rela);
    relocations.push_back(relocation);
  }
}

void RelocationSection::parseReferences(
    const std::shared_ptr<SymbolTableSection> &symTab) const {
  for (const auto &relocation : relocations) {
    // Handle offset idxRef
    relocation->setOffset(std::make_shared<IdxRef>(relocation->getROffset()));
    // Handle symbol ref.
    const auto sym = symTab->getSymbol(relocation->getSymbolInfo());
    relocation->setSym(sym);
  }
}

void RelocationSection::layout() { sh_size = relocations.size() * sh_entsize; }

void RelocationSection::fini() {
  for (const auto &relocation : relocations) {
    // Update r_offset.
    relocation->setROffset(relocation->getOffsetValue());
    // Update symbol info.
    const auto idx = relocation->getSym()->getIdxValue();
    relocation->setSymbolInfo(idx);
  }
}

void RelocationSection::writeDataTo(char *buffer) {
  for (size_t i = 0; i < relocations.size(); i++) {
    auto *rela = reinterpret_cast<Elf_Rela *>(buffer + i * sh_entsize);
    relocations[i]->writeDataTo(rela);
  }
}

void RelocationSection::dumpData(std::ostream &oss) const {
  oss << std::setfill(' ');
  oss << std::setw(16) << "Offset" << " ";
  oss << std::setw(16) << "Info" << " ";
  oss << std::setw(20) << "Type" << " ";
  oss << std::setw(16) << "SymbolValue" << " ";
  oss << std::setw(20) << "SymbolName" << " ";
  oss << "Addend" << " ";
  oss << std::endl;

  for (const auto &relocation : relocations) {
    relocation->dump(oss);
    oss << std::endl;
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
