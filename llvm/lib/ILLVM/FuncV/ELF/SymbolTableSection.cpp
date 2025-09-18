#include "illvm/FuncV/ELF/SymbolTableSection.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

SymtabShndxSection::SymtabShndxSection(const llvm::object::ELF64LE::Shdr *shdr,
                                       const char *_data, llvm::Error &err)
    : Section(SectionType::SymTabShNdx, shdr, _data) {
  // Refer to llvm/include/llvm/Object/ELF.h::ELFFile::getSHNDXTable
  ILLVM_FCHECK(shdr->sh_entsize == sizeof(llvm::object::ELF64LE::Word), "");
  ILLVM_FCHECK(shdr->sh_size % shdr->sh_entsize == 0, "");
  const size_t symNum = shdr->sh_size / shdr->sh_entsize;
  originalIndexes.reserve(symNum);
  for (size_t i = 0; i < symNum; i++) {
    const auto *shndx = reinterpret_cast<const llvm::object::ELF64LE::Word *>(
        _data + i * shdr->sh_entsize);
    originalIndexes.push_back(*shndx);
  }
}

void SymtabShndxSection::writeDataTo(char *buffer) {
  for (size_t i = 0; i < indexes.size(); i++) {
    auto *shndx = reinterpret_cast<llvm::object::ELF64LE::Word *>(
        buffer + i * sh_entsize);
    *shndx = indexes[i];
  }
}

void SymtabShndxSection::dumpData(std::ostream &oss) const {
  for (size_t i = 0; i < indexes.size(); i++) {
    oss << "[" << i << "] " << indexes[i] << std::endl;
  }
}

SymbolTableSection::SymbolTableSection(const llvm::object::ELF64LE::Shdr *shdr,
                                       const char *_data, llvm::Error &err)
    : Section(SectionType::SymTab, shdr, _data), symtabShndx(nullptr),
      localSymNum(0) {
  ILLVM_FCHECK(shdr->sh_entsize == sizeof(Elf_Sym), "");
  ILLVM_FCHECK(shdr->sh_size % shdr->sh_entsize == 0, "");

  const size_t symNum = shdr->sh_size / shdr->sh_entsize;
  symbols.reserve(symNum);

  for (size_t i = 0; i < symNum; i++) {
    const auto *sym =
        reinterpret_cast<const Elf_Sym *>(_data + i * shdr->sh_entsize);
    const auto symbol = std::make_shared<Symbol>(sym);
    symbols.push_back(symbol);
  }
}

void SymbolTableSection::parseReferences(
    const std::vector<std::shared_ptr<Section>> &sections,
    const std::shared_ptr<StringTableSection> &strTab) const {
  for (size_t i = 0; i < symbols.size(); i++) {
    const auto symbol = symbols[i];
    // Name offset -> strRef.
    symbol->setName(strTab->parseOriginalIndex(symbol->getStName()));
    // Value -> idxRef
    symbol->setValue(std::make_shared<IdxRef>(symbol->getStValue()));

    // Handle shndx.
    const uint64_t shndx = symbol->getStShndx();
    if (shndx == llvm::ELF::SHN_UNDEF ||
        (llvm::ELF::SHN_LORESERVE <= shndx && shndx < llvm::ELF::SHN_XINDEX)) {
      symbol->setSpecialShndx(shndx);
    } else {
      uint64_t secIdx = 0;
      if (shndx == llvm::ELF::SHN_XINDEX) {
        ILLVM_FCHECK(symtabShndx != nullptr, "");
        secIdx = symtabShndx->originalIndexes[i];
      } else {
        secIdx = shndx;
      }
      const auto &section = sections[secIdx];
      // Handle section symbol name.
      if (symbol->getStType() == llvm::ELF::STT_SECTION) {
        symbol->setName(section->getName());
      }
      // shndx -> sec idx ref.
      symbol->setSecIdx(section->getIdx());
    }

    // idx -> idxRef.
    symbol->setIdx(std::make_shared<IdxRef>(i));
  }
}

void SymbolTableSection::layout() {
  // Local symbols need to be placed before global symbols.
  std::vector<std::shared_ptr<Symbol>> temp;
  temp.reserve(symbols.size());
  // Local
  for (const auto &symbol : symbols) {
    if (symbol->getStBind() != llvm::ELF::STB_LOCAL) {
      continue;
    }
    temp.push_back(symbol);
  }
  localSymNum = temp.size();
  // Other
  for (const auto &symbol : symbols) {
    if (symbol->getStBind() == llvm::ELF::STB_LOCAL) {
      continue;
    }
    temp.push_back(symbol);
  }
  symbols = temp;

  // Update idx, size (symtab + shndxtab).
  for (size_t i = 0; i < symbols.size(); i++) {
    const auto &symbol = symbols[i];
    symbol->getIdx()->setValue(i);
  }
  sh_size = symbols.size() * sh_entsize;
  if (symtabShndx != nullptr) {
    symtabShndx->sh_size = symbols.size() * symtabShndx->sh_entsize;
  }
}

// Call after section layout.
bool SymbolTableSection::needSymtabShNdx() const {
  for (const auto &symbol : symbols) {
    const int specialShndx = symbol->getSpecialShndx();
    if (specialShndx != -1) {
      continue;
    }
    const uint64_t secIdx = symbol->getSecIdxValue();
    if (secIdx >= llvm::ELF::SHN_LORESERVE) {
      return true;
    }
  }
  return false;
}

void SymbolTableSection::fini() {
  // Reconstruction by SymbolTableSection.
  if (symtabShndx != nullptr) {
    symtabShndx->indexes.resize(symbols.size(), 0);
  }

  for (size_t i = 0; i < symbols.size(); i++) {
    const auto &symbol = symbols[i];
    // Update name.
    if (symbol->getStType() == llvm::ELF::STT_SECTION) {
      symbol->setStName(0);
    } else {
      symbol->setStName(symbol->getName()->getOffset());
    }
    // Update value.
    symbol->setStValue(symbol->getValueValue());
    // Update st_shndx.
    const int specialShndx = symbol->getSpecialShndx();
    if (specialShndx != -1) {
      symbol->setStShndx(specialShndx);
    } else {
      const uint64_t secIdx = symbol->getSecIdxValue();
      if (secIdx >= llvm::ELF::SHN_LORESERVE) {
        symbol->setStShndx(llvm::ELF::SHN_XINDEX);
        // Update SymtabShndxSection.
        symtabShndx->indexes[i] = secIdx;
      } else {
        symbol->setStShndx(secIdx);
      }
    }
  }

  // Update info.
  sh_info = localSymNum;
}

void SymbolTableSection::writeDataTo(char *buffer) {
  for (size_t i = 0; i < symbols.size(); i++) {
    auto *sym = reinterpret_cast<Elf_Sym *>(buffer + i * sh_entsize);
    symbols[i]->writeDataTo(sym);
  }
}

void SymbolTableSection::dumpData(std::ostream &oss) const {
  oss << std::setfill(' ');
  oss << "[" << std::setw(5) << "Nr" << "] ";
  oss << std::setw(16) << "Value" << " ";
  oss << std::setw(6) << "Size" << " ";
  oss << std::setw(8) << "Type" << " ";
  oss << std::setw(8) << "Bind" << " ";
  oss << std::setw(8) << "Vis" << " ";
  oss << std::setw(8) << "Ndx" << " ";
  oss << std::setw(20) << "Name" << " ";
  oss << std::endl;

  for (const auto &symbol : symbols) {
    symbol->dump(oss);
    oss << std::endl;
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
