#ifndef ICLANG_SECTION_HPP
#define ICLANG_SECTION_HPP

#include "iclang/dwarf/config.hpp"
#include "iclang/dwarf/reference.hpp"
#include "iclang/dwarf/relocation.hpp"
#include "iclang/dwarf/symbol.hpp"

namespace iclang {

class Section {
protected:
  SectionType type;

  uint32_t sh_name; // Update while writing, according to name.
  std::shared_ptr<StrRef> name;
  uint32_t sh_type;
  uint64_t sh_flags;
  uint64_t sh_addr; // Always 0.
  uint64_t sh_offset; // Update while writing.
  uint64_t sh_size; // Update while writing.
  uint32_t sh_link; // Update while writing, according to link.
  std::shared_ptr<IdxRef> link; // May be nullptr.
  uint32_t sh_info;
  // Refer to https://docs.oracle.com/cd/E26502_01/html/E26507/chapter6-94076.html#chapter6-47976 .
  std::shared_ptr<IdxRef> infoLink; // May be nullptr.
  uint64_t sh_addralign;
  uint64_t sh_entsize;

  // Section data.
  // Important: Once data is created, it cannot be modified.
  // We will recreate a new buffer of data while writing.
  const char *data;

  // The index of this section in section header table，
  // update while writing.
  std::shared_ptr<IdxRef> idx;

public:
  Section(const SectionType _type, const uint32_t _sh_name, const std::shared_ptr<StrRef>& _name,
  const uint32_t _sh_type, const uint64_t _sh_flags, const uint64_t _sh_addr,
  const uint64_t _sh_offset, const uint64_t _sh_size,
  const uint32_t _sh_link, const std::shared_ptr<IdxRef> &_link,
  const uint32_t _sh_info, const std::shared_ptr<IdxRef> &_info_link, const uint64_t _sh_addralign, const uint64_t _sh_entsize,
  const char *_data, const std::shared_ptr<IdxRef> &_idx) : type(_type), sh_name(_sh_name), name(_name),
  sh_type(_sh_type), sh_flags(_sh_flags), sh_addr(_sh_addr),
  sh_offset(_sh_offset), sh_size(_sh_size),
  sh_link(_sh_link), link(_link),
  sh_info(_sh_info), infoLink(_info_link), sh_addralign(_sh_addralign), sh_entsize(_sh_entsize),
  data(_data), idx(_idx) {}

  Section(const SectionType _type, const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
  : Section(_type, shdr->sh_name, nullptr,
    shdr->sh_type, shdr->sh_flags, shdr->sh_addr,
    shdr->sh_offset, shdr->sh_size,
    shdr->sh_link, nullptr,
    shdr->sh_info, nullptr, shdr->sh_addralign, shdr->sh_entsize,
    _data, nullptr) {}

  virtual ~Section() = default;

  SectionType getType() const { return type; }

  uint64_t getShName() const { return sh_name; }

  void setShName(const uint64_t _sh_name) { sh_name = _sh_name; }

  std::shared_ptr<StrRef> getName() const { return name; }

  const char *getNameValue() const { return name->getValue(); }

  void setName(const std::shared_ptr<StrRef> &_name) { name = _name; }

  uint64_t getShType() const { return sh_type; }

  uint64_t getShFlags() const { return sh_flags; }

  uint64_t getShOffset() const { return sh_offset; }

  void setShOffset(const uint64_t offset) { sh_offset = offset; }

  uint64_t getShSize() const { return sh_size; }

  void setShSize(const uint64_t size) { sh_size = size; }

  uint64_t getShLink() const { return sh_link; }

  void setShLink(const uint64_t _sh_link) { sh_link = _sh_link; }

  std::shared_ptr<IdxRef> getLink() const { return link; }

  void setLink(const std::shared_ptr<IdxRef> &_link) { link = _link; }

  uint32_t getShInfo() const { return sh_info; }

  void setShInfo(const uint32_t _sh_info) { sh_info = _sh_info; }

  std::shared_ptr<IdxRef> getInfoLink() const { return infoLink; }

  void setInfoLink(const std::shared_ptr<IdxRef> &_info_link) { infoLink = _info_link; }

  uint64_t getShAddralign() const { return sh_addralign; }

  uint64_t getEntSize() const { return sh_entsize; }

  const char *getData() const { return data; }

  void setData(const char * _data) { data = _data; }

  std::shared_ptr<IdxRef> getIdx() const { return idx; }

  uint64_t getIdxValue() const { return idx->getValue(); }

  void setIdx(const std::shared_ptr<IdxRef> &_idx) { idx = _idx; }

  // Cal order, idx, offset, size.
  virtual void layout() {}

  virtual void fini() {}

  void writeHeaderTo(llvm::object::ELF64LE::Shdr *shdr) {
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

  virtual void writeDataTo(char *buffer) {
    memcpy(buffer, data, sh_size);
  }

  void dumpHeader(std::ostream &oss) const {
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
    if (link == nullptr) {
      oss << std::setw(5) << 0 << " ";
    } else {
      oss << std::setw(5) << link->getValue() << " ";
    }
    oss << std::setw(4) << sh_info << " ";
    oss << std::setw(4) << sh_addralign;
  }

  virtual void dumpData(std::ostream &oss) const {
    oss << std::hex;
    oss << std::setfill('0');
    for (uint64_t i = 0; i < sh_size; i += 16) {
      if (i != 0) {
        oss << "\n";
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
    oss << '\n';
  }
};

class OrdinarySection final : public Section {
private:

public:
  OrdinarySection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
  : Section(SectionType::Ordinary, shdr, _data) {}
};

class StringTableSection final : public Section {
private:
  // offset -> strRef.
  // It can only work during parsing.
  std::map<uint64_t, std::shared_ptr<StrRef>> originalIndexes;
  // The first string should be "".
  std::vector<std::shared_ptr<StrRef>> strs;
  std::vector<char> newStrPool;

public:
  StringTableSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
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

  std::shared_ptr<StrRef> parseOriginalIndex(uint64_t strOff) {
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

  void layout() override {
    uint64_t strOff = 0;
    sh_size = 0;
    for (const auto &strRef : strs) {
      strRef->setOffset(strOff);
      strOff += strRef->getLength() + 1;
      sh_size += strRef->getLength() + 1;
    }
  }

  void writeDataTo(char *buffer) override {
    uint64_t strOff = 0;
    for (const auto &strRef : strs) {
      const uint64_t length = strRef->getLength() + 1;
      memcpy(buffer + strOff, strRef->getValue(), length);
      strOff += length;
    }
  }

  void dumpData(std::ostream &oss) const override {
    for (size_t i = 0; i < strs.size(); i++) {
      oss << "[" << i << "] \"";
      strs[i]->dump(oss);
      oss << "\"\n";
    }
  }

  // Add an existed str ref to strtab.
  void push_back(const std::shared_ptr<StrRef>& str) {
    strs.push_back(str);
  }

  // Add a new str to strtab.
  std::shared_ptr<StrRef> push_back(const std::string &str) {
    for (const auto &c : str) {
      newStrPool.push_back(c);
    }
    newStrPool.push_back('\0');
    uint64_t offset = newStrPool.size() - str.length() - 1;
    auto res = std::make_shared<StrRef>(newStrPool.data() + offset, offset);
    strs.push_back(res);
    return res;
  }
};

// Updated by SymbolTableSection.
class SymtabShndxSection final : public Section {
private:
  // idx -> symbol shndx.
  std::vector<uint32_t> originalIndexes;
  // Reconstruction by SymbolTableSection.
  std::vector<uint32_t> indexes;

public:
  friend class SymbolTableSection;

  SymtabShndxSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
  : Section(SectionType::SymTabShNdx, shdr, _data) {
    auto &logger = Logger::getInstance();

    // Refer to llvm/include/llvm/Object/ELF.h::ELFFile::getSHNDXTable
    if (shdr->sh_entsize != sizeof(llvm::object::ELF64LE::Word)) {
      logger.fatal("Invalid symtab shndx: shdr->sh_entsize != sizeof(Word)");
    }
    if (shdr->sh_size % shdr->sh_entsize != 0) {
      logger.fatal("Invalid symtab shndx: shdr->sh_size % shdr->sh_entsize != 0");
    }
    const size_t symNum = shdr->sh_size / shdr->sh_entsize;
    originalIndexes.reserve(symNum);
    for (size_t i = 0; i < symNum; i++) {
      const auto *shndx = reinterpret_cast<const llvm::object::ELF64LE::Word *>(_data + i * shdr->sh_entsize);
      originalIndexes.push_back(*shndx);
    }
  }

  void writeDataTo(char *buffer) override {
    for (size_t i = 0; i < indexes.size(); i++) {
      auto *shndx = reinterpret_cast<llvm::object::ELF64LE::Word *>(buffer + i * sh_entsize);
      *shndx = indexes[i];
    }
  }

  void dumpData(std::ostream &oss) const override {
    for (size_t i = 0; i < indexes.size(); i++) {
      oss << "[" << i << "] " << indexes[i] << "\n";
    }
  }
};

class SymbolTableSection final : public Section {
private:
  using Elf_Sym = llvm::object::ELF64LE::Sym;
  std::shared_ptr<SymtabShndxSection> symtabShndx;
  std::vector<std::shared_ptr<Symbol>> symbols;

  // Update after layout.
  uint64_t localSymNum;

public:
  SymbolTableSection(const typename llvm::object::ELF64LE::Shdr *shdr, const char *_data)
  : Section(SectionType::SymTab, shdr, _data), symtabShndx(nullptr), localSymNum(0) {
    auto &logger = Logger::getInstance();

    if (shdr->sh_entsize != sizeof(Elf_Sym)) {
      logger.fatal("Invalid symbol table: shdr->sh_entsize != sizeof(Elf_Sym)");
    }
    if (shdr->sh_size % shdr->sh_entsize != 0) {
      logger.fatal("Invalid symbol table: shdr->sh_size % shdr->sh_entsize != 0");
    }

    const size_t symNum = shdr->sh_size / shdr->sh_entsize;
    symbols.reserve(symNum);

    for (size_t i = 0; i < symNum; i++) {
      const auto* sym = reinterpret_cast<const Elf_Sym*>(_data + i * shdr->sh_entsize);
      std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>(sym);
      symbols.push_back(symbol);
    }
  }

  int getSize() const { return symbols.size(); }

  std::shared_ptr<Symbol> getSymbol(const size_t idx) { return symbols[idx]; }

  void push_back(const std::shared_ptr<Symbol>& symbol) {
    symbols.push_back(symbol);
  }

  // _symtabShndx can be nullptr, and we will check whether _symtabShndx is needed.
  void parseShNdx(const std::shared_ptr<SymtabShndxSection> &_symtabShndx) {
    auto &logger = Logger::getInstance();

    // Check whether _symtabShndx is needed.
    bool isNeeded = false;
    for (const auto & symbol : symbols) {
      if (symbol->getStShndx() == llvm::ELF::SHN_XINDEX) {
        isNeeded = true;
        break;
      }
    }
    if (isNeeded && _symtabShndx == nullptr) {
      logger.fatal("Missing symtab shndx");
    }

    symtabShndx = _symtabShndx;

    for (size_t i = 0; i < symbols.size(); i++) {
      const auto &symbol = symbols[i];
      const uint64_t shndx = symbol->getStShndx();
      if (shndx == llvm::ELF::SHN_UNDEF ||
        (llvm::ELF::SHN_LORESERVE <= shndx && shndx < llvm::ELF::SHN_XINDEX)) {
        symbol->setSpecialShndx(shndx);
      } else if (shndx == llvm::ELF::SHN_XINDEX) {
        symbol->setStShndx(symtabShndx->originalIndexes[i]);
      }
    }
  }

  void parseReferences(const std::vector<std::shared_ptr<Section>> &sections,
    const std::shared_ptr<StringTableSection> &strTab) const {
    for (size_t i = 0; i < symbols.size(); i++) {
      const auto symbol = symbols[i];
      // Name offset - > strRef.
      symbol->setName(strTab->parseOriginalIndex(symbol->getStName()));
      if (symbol->getSpecialShndx() == -1) {
        const auto& section = sections[symbol->getStShndx()];

        // Handle section symbol name.
        if (symbol->getStType() == llvm::ELF::STT_SECTION) {
          symbol->setName(section->getName());
        }
        // st_shndx -> target section idx.
        symbol->setSecIdx(section->getIdx());
      }
      // idx -> idxRef.
      symbol->setIdx(std::make_shared<IdxRef>(i));
    }
  }

  void layout() override {
    // Local symbols need to be placed before global symbols.
    std::vector<std::shared_ptr<Symbol>> temp;
    temp.reserve(symbols.size());
    // Local
    for (const auto & symbol : symbols) {
      if (symbol->getStBind() != llvm::ELF::STB_LOCAL) {
        continue;
      }
      temp.push_back(symbol);
    }
    localSymNum = temp.size();
    // Other
    for (const auto & symbol : symbols) {
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

  void fini() override {
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
      // Update st_shndx.
      const int specialShndx = symbol->getSpecialShndx();
      if (specialShndx != -1) {
        symbol->setStShndx(specialShndx);
      } else {
        const int shndx = symbol->getSecIdxValue();
        if (shndx >= llvm::ELF::SHN_LORESERVE) {
          symbol->setStShndx(llvm::ELF::SHN_XINDEX); 
          // Update SymtabShndxSection.
          symtabShndx->indexes[i] = shndx;
        } else {
          symbol->setStShndx(shndx);
        }
      }
    }

    // Update info.
    sh_info = localSymNum;
  }

  void writeDataTo(char *buffer) override {
    for (size_t i = 0; i < symbols.size(); i++) {
      auto* sym = reinterpret_cast<Elf_Sym*>(buffer + i * sh_entsize);
      symbols[i]->writeDataTo(sym);
    }
  }

  void dumpData(std::ostream &oss) const override {
    oss << std::setfill(' ');
    oss << "[" << std::setw(5) << "Nr" << "] ";
    oss << std::setw(16) << "Value" << " ";
    oss << std::setw(6) << "Size" << " ";
    oss << std::setw(8) << "Type" << " ";
    oss << std::setw(8) << "Bind" << " ";
    oss << std::setw(8) << "Vis" << " ";
    oss << std::setw(8) << "Ndx" << " ";
    oss << std::setw(20) << "Name" << " ";
    oss << "\n";

    for (const auto &symbol : symbols) {
      symbol->dump(oss);
      oss << "\n";
    }
  }

  const std::vector<std::shared_ptr<Symbol>> &getSymbols() {
    return symbols;
  }
};

class RelocationSection final : public Section {
private:
  using Elf_Rela = typename llvm::object::ELF64LE::Rela;
  // index -> relocation entry.
  std::vector<std::shared_ptr<Relocation>> relocations;

public:
  RelocationSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
  : Section(SectionType::RelaTab, shdr, _data) {
    auto &logger = Logger::getInstance();

    if (shdr->sh_entsize != sizeof(Elf_Rela)) {
      logger.fatal("Invalid symbol table: shdr->sh_entsize != sizeof(Elf_Rela)");
    }
    if (shdr->sh_size % shdr->sh_entsize != 0) {
      logger.fatal("Invalid symbol table: shdr->sh_size % shdr->sh_entsize != 0");
    }

    const size_t relocationNum = shdr->sh_size / shdr->sh_entsize;
    relocations.reserve(relocationNum);

    for (size_t i = 0; i < relocationNum; i++) {
      const auto* rela = reinterpret_cast<const Elf_Rela*>(_data + i * shdr->sh_entsize);
      std::shared_ptr<Relocation> relocation = std::make_shared<Relocation>(rela);
      relocations.push_back(relocation);
    }
  }

  void parseReferences(const std::shared_ptr<SymbolTableSection> &symTab) {
    for (const auto &relocation : relocations) {
      auto sym = symTab->getSymbol(relocation->getSymbolInfo());
      relocation->setSym(sym);
    }
  }

  void layout() override {
    sh_size = relocations.size() * sh_entsize;
  }

  void fini() override {
    for (const auto & relocation : relocations) {
      // Update symbol info.
      const auto idx = relocation->getSym()->getIdxValue();
      relocation->setSymbolInfo(idx);
    }
  }

  void writeDataTo(char *buffer) override {
    for (size_t i = 0; i < relocations.size(); i++) {
      auto* rela = reinterpret_cast<Elf_Rela*>(buffer + i * sh_entsize);
      relocations[i]->writeDataTo(rela);
    }
  }

  void dumpData(std::ostream &oss) const override {
    oss << std::setfill(' ');
    oss << std::setw(16) << "Offset" << " ";
    oss << std::setw(16) << "Info" << " ";
    oss << std::setw(20) << "Type" << " ";
    oss << std::setw(16) << "SymbolValue" << " ";
    oss << std::setw(20) << "SymbolName" << " ";
    oss << "Addend" << " ";
    oss << "\n";

    for (const auto &relocation : relocations) {
      relocation->dump(oss);
      oss << "\n";
    }
  }

  const std::vector<std::shared_ptr<Relocation>> &getRelocations() {
    return relocations;
  }

  void push_back(const std::shared_ptr<Relocation>& relocation) {
    relocations.push_back(relocation);
  }
};

class DebugInfoSection final : public Section {
public:
  DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
      : Section(SectionType::DebugInfo, shdr, _data) {
    // TODO parse debug info structure
  }

  void dumpData(std::ostream &oss) const override {
    // TODO dump debug info structure
    oss << "...\n";
  }
};

} // namespace iclang

#endif // ICLANG_SECTION_HPP
