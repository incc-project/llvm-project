#ifndef ICLANG_SECTION_HPP
#define ICLANG_SECTION_HPP

#include "iclang/dwarf/config.hpp"
#include "iclang/dwarf/reference.hpp"
#include "iclang/dwarf/relocation.hpp"
#include "iclang/dwarf/symbol.hpp"
#include "iclang/dwarf/tools.hpp"
#include "llvm/Support/Endian.h"
using namespace llvm::support;

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

class DebugAbbrevSection final : public Section {
public:
  // Structure representing an attribute-form pair in an abbreviation declaration
  struct AttributeForm {
    uint64_t attr;                  // DWARF attribute code
    uint64_t form;                  // DWARF form code
    std::optional<int64_t> implicitConst; // Optional implicit constant value
  };

  // Structure representing an abbreviation declaration
  struct AbbreviationDecl {
    uint64_t code;                  // Abbreviation code
    uint64_t tag;                   // DWARF tag
    bool hasChildren;               // Whether this DIE has children
    std::vector<AttributeForm> attrForms; // List of attribute-form pairs
  };

  // Map of abbreviation tables (keyed by offset)
  std::map<uint64_t, std::map<uint64_t, AbbreviationDecl>> abbrevTables;

  DebugAbbrevSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
    : Section(SectionType::DebugAbbrev, shdr, _data) {
    // TODO parse debug abbrev structure
    const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
    const uint8_t *end = start + sh_size;
    const uint8_t *p = start;

    while (p < end) {
      uint64_t abbrevOffset = p - start;
      std::map<uint64_t, AbbreviationDecl> declMap;

      while (p < end) {
        unsigned bytesRead;
        uint64_t code = decodeULEB128(p, &bytesRead, end);
        p += bytesRead;
        if (code == 0)
          break;

        uint64_t tag = decodeULEB128(p, &bytesRead, end);
        p += bytesRead;
        uint8_t hasChildrenByte = *p++;

        AbbreviationDecl decl;
        decl.code = code;
        decl.tag = tag;
        decl.hasChildren = (hasChildrenByte == 1); // DW_CHILDREN_yes

        while (p < end) {
          uint64_t attr = decodeULEB128(p, &bytesRead, end);
          p += bytesRead;
          uint64_t form = decodeULEB128(p, &bytesRead, end);
          p += bytesRead;
          if (attr == 0 && form == 0)
            break;

          AttributeForm af = {attr, form};
          if (form == 0x21 /* DW_FORM_implicit_const */) {
            af.implicitConst = decodeSLEB128(p, end);
          }

          decl.attrForms.push_back(af);
        }

        declMap[code] = std::move(decl);
      }

      if (!declMap.empty())
        abbrevTables[abbrevOffset] = std::move(declMap);
    }
  }

  void dumpData(std::ostream &oss) const override {
    // TODO dump debug abbrev structure
    oss << ".debug_abbrev contents:\n";
    for (const auto &[offset, decls] : abbrevTables) {
      oss << "Abbrev table for offset: 0x" << std::setw(8) << std::setfill('0')
          << std::hex << offset << "\n";
      for (const auto &[code, decl] : decls) {
        oss << std::dec << code << ". "
            << getTagName(decl.tag) << "\t"
            << (decl.hasChildren ? "DW_CHILDREN_yes" : "DW_CHILDREN_no") << "\n";

        for (const auto &af : decl.attrForms) {
          oss << "\t" << getAttrName(af.attr) << "\t" << getFormName(af.form);
          if (af.form == 0x21 && af.implicitConst.has_value()) {
            oss << " " << af.implicitConst.value();
          }
          oss << "\n";
        }
        oss << "\n";
      }
    }
  }

  void writeDataTo(char *buffer) override {
    std::vector<uint8_t> out;

    for (const auto &[offset, decls] : abbrevTables) {
      (void)offset;

      for (const auto &[code, decl] : decls) {
        encodeULEB128(code, out);
        encodeULEB128(decl.tag, out);
        out.push_back(decl.hasChildren ? 1 : 0);

        for (const auto &af : decl.attrForms) {
          encodeULEB128(af.attr, out);
          encodeULEB128(af.form, out);
          if (af.form == 0x21 && af.implicitConst.has_value()) {
            encodeSLEB128(af.implicitConst.value(), out);
          }
        }

        // Write attribute-form terminator (0, 0)
        encodeULEB128(0, out);
        encodeULEB128(0, out);
      }
      encodeULEB128(0, out);
    }

    // Copy to target buffer
    assert(out.size() <= sh_size && "Rewritten .debug_abbrev exceeds original section size");
    memcpy(buffer, out.data(), out.size());
  }

  // Get abbreviation declaration by offset and code
  const AbbreviationDecl* getAbbreviationDecl(uint64_t abbrevOffset, uint64_t code) const {
    auto abbrevTableIt = abbrevTables.find(abbrevOffset);
    if (abbrevTableIt == abbrevTables.end()) return nullptr;

    const auto& decls = abbrevTableIt->second;
    auto declIt = decls.find(code);
    if (declIt == decls.end()) return nullptr;

    return &declIt->second;
  }
};

class DebugStrSection final : public Section {
private:
  // Structure representing a string entry in .debug_str section
  struct StringEntry {
    uint64_t offset;    // Offset within the section
    std::string str;    // The string content
  };

  std::vector<StringEntry> strings; // List of string entries

public:
  DebugStrSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
      : Section(SectionType::DebugStr, shdr, _data) {
    // TODO parse debug str structure
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

  void writeDataTo(char *buffer) override {
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

  void dumpData(std::ostream &oss) const override {
    // TODO dump debug str structure
    oss << ".debug_str contents:\n";
    for (const auto &entry : strings) {
      oss << "0x" << intToHex(entry.offset, 8) << ": \"" << entry.str << "\"\n";
    }
  }

  // Get string by offset
  std::string getString(uint32_t offset) const {
    if (offset >= sh_size) return "<invalid offset>";
    return std::string(data + offset);
  }

};

class DebugAddrSection final : public Section {
private:
  // Structure representing an address table
  struct AddrTable {
    uint64_t offsetBase;   // Table start offset (relative to section)
    uint64_t size;         // Total table size (including header and addresses)
    uint16_t version;      // DWARF version
    uint8_t addrSize;      // Address size in bytes
    uint8_t segSize;      // Segment selector size
    uint64_t headerSize;   // Header size
    std::vector<uint64_t> addresses; // List of addresses
  };

  std::vector<AddrTable> tables; // List of address tables

public:
  DebugAddrSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
      : Section(SectionType::DebugAddr, shdr, _data) {
    // TODO parse debug addr structure
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data);
    const uint8_t *end = ptr + shdr->sh_size;

    while (ptr < end) {
      const uint8_t *tableStart = ptr;
      if (end - ptr < 8)
        break; // Invalid header

      uint32_t unitLength = *reinterpret_cast<const uint32_t *>(ptr);
      ptr += 4;

      const uint8_t *tableEnd = ptr + unitLength;
      if (tableEnd > end) break;

      if (tableEnd - ptr < 4) break;

      AddrTable table;
      table.offsetBase = tableStart - reinterpret_cast<const uint8_t *>(data);
      table.size = unitLength + 4;

      table.version = *reinterpret_cast<const uint16_t *>(ptr);
      ptr += 2;

      table.addrSize = *ptr++;
      table.segSize = *ptr++;
      table.headerSize = 4 + 2 + 1 + 1;

      while (ptr + table.addrSize <= tableEnd) {
        uint64_t addr = 0;
        memcpy(&addr, ptr, table.addrSize);
        ptr += table.addrSize;
        table.addresses.push_back(addr);
      }

      tables.push_back(std::move(table));
    }
  }

  // Apply relocations to address entries
  void applyRelocations(const std::vector<std::shared_ptr<Relocation>> &relocs) {
    for (const auto &rel : relocs) {
      uint64_t r_offset = rel->getROffset();

      for (auto &table : tables) {
        uint64_t tableStart = table.offsetBase + table.headerSize;
        uint64_t tableEnd = table.offsetBase + table.size;

        if (r_offset < tableStart || r_offset >= tableEnd)
          continue;

        uint64_t offsetInTable = r_offset - tableStart;
        if (offsetInTable % table.addrSize != 0)
          continue;

        size_t entryIndex = offsetInTable / table.addrSize;
        if (entryIndex >= table.addresses.size())
          continue;

        uint64_t relocated = static_cast<uint64_t>(rel->getRAddend());
        table.addresses[entryIndex] = relocated;
      }
    }
  }

  // Get address by table base offset and index
  uint64_t getAddressByIndex(uint64_t baseOffset, uint64_t index) const {
    for (const auto &table : tables) {
      if (table.offsetBase == baseOffset) {
        if (index < table.addresses.size())
          return table.addresses[index];
      }
    }
    return -1;
  }

  void writeDataTo(char *buffer) override {
    std::vector<uint8_t> out;

    for (const auto &table : tables) {
      size_t startOffset = out.size();

      // 1. Reserve space for unit_length (4 bytes)
      out.resize(out.size() + 4);

      // 2. Write header: version (2 bytes), addr_size (1), seg_size (1)
      out.push_back(table.version & 0xff);
      out.push_back((table.version >> 8) & 0xff);
      out.push_back(table.addrSize);
      out.push_back(table.segSize);

      // 3. Write address entries
      for (uint64_t addr : table.addresses) {
        if (table.addrSize == 4) {
          out.push_back(addr & 0xff);
          out.push_back((addr >> 8) & 0xff);
          out.push_back((addr >> 16) & 0xff);
          out.push_back((addr >> 24) & 0xff);
        } else if (table.addrSize == 8) {
          for (int i = 0; i < 8; ++i)
            out.push_back((addr >> (i * 8)) & 0xff);
        } else {
          assert(false && "Unsupported addr_size");
        }
      }

      // 4. Fill in unit_length (total size minus length field)
      uint32_t length = static_cast<uint32_t>(out.size() - startOffset - 4);
      out[startOffset + 0] = (length & 0xff);
      out[startOffset + 1] = (length >> 8) & 0xff;
      out[startOffset + 2] = (length >> 16) & 0xff;
      out[startOffset + 3] = (length >> 24) & 0xff;
    }

    // 5. Copy to target buffer
    assert(out.size() <= sh_size && "Rewritten .debug_addr larger than original");
    memcpy(buffer, out.data(), out.size());
  }

  void dumpData(std::ostream &oss) const override {
    // TODO dump debug addr structure
    for (const auto &table : tables) {
      oss << ".debug_addr contents:\n";

      oss << "Address table header: length = 0x"
          << std::hex << std::setw(8) << std::setfill('0') << table.size - 4
          << ", format = DWARF32, version = 0x"
          << std::setw(4) << table.version
          << ", addr_size = 0x"
          << std::setw(2) << static_cast<int>(table.addrSize)
          << ", seg_size = 0x"
          << std::setw(2) << static_cast<int>(table.segSize)
          << std::dec << "\n";

      oss << "Addrs: [\n";
      for (uint64_t addr : table.addresses) {
        if (table.addrSize == 4) {
          oss << "0x" << std::hex << std::setw(8) << std::setfill('0')
              << static_cast<uint32_t>(addr) << std::dec << "\n";
        } else if (table.addrSize == 8) {
          oss << "0x" << std::hex << std::setw(16) << std::setfill('0')
              << addr << std::dec << "\n";
        } else {
          oss << "# unsupported addr_size: " << static_cast<int>(table.addrSize) << "\n";
        }
      }
      oss << "]\n";
    }
  }
};

class DebugStrOffsetsSection final : public Section {
private:
  const DebugStrSection &debugStr;

  // Structure representing a string offsets table
  struct StringOffsetsTable {
    uint64_t offsetBase;  // Table start offset (relative to section)
    uint64_t size;        // Contribution size
    uint16_t version;    // DWARF version
    uint64_t headerSize;  // Header size
    uint64_t entrySize;   // Entry size (bytes)
    std::vector<uint32_t> offsets; // List of string offsets
  };

  std::vector<StringOffsetsTable> tables; // List of string offset tables

public:
  DebugStrOffsetsSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data, const DebugStrSection &strRef)
      : Section(SectionType::DebugStrOffsets, shdr, _data), debugStr(strRef) {
    // TODO parse debug str offset structure
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data);
    const uint8_t *end = ptr + sh_size;

    while (ptr < end) {
      const uint8_t *tableStart = ptr;
      if (end - ptr < 4) break;

      uint32_t unitLength = *reinterpret_cast<const uint32_t *>(ptr);
      ptr += 4;

      const uint8_t *tableEnd = ptr + unitLength;
      if (tableEnd > end) break;

      if (tableEnd - ptr < 4) break;
      uint16_t version = *reinterpret_cast<const uint16_t *>(ptr);
      ptr += 2;
      ptr += 2; // padding/reserved

      StringOffsetsTable table;
      table.offsetBase = tableStart - reinterpret_cast<const uint8_t *>(data);
      table.size = unitLength + 4;
      table.version = version;
      table.headerSize = 8; // unit_length + version + padding = 2 + 2 + 4
      table.entrySize = 4; // DWARF v5 string offset entries are 4 bytes

      while (ptr + 4 <= tableEnd) {
//        uint32_t offset = *reinterpret_cast<const uint32_t *>(ptr);
        uint32_t offset = endian::read32le(ptr);
        table.offsets.push_back(offset);
        ptr += 4;
      }

      tables.push_back(std::move(table));
      ptr = tableEnd;
    }
  }

  // Apply relocations to string offsets
  void applyRelocations(const std::vector<std::shared_ptr<Relocation>> &relocs) {
    for (const auto &rel : relocs) {
      uint64_t r_offset = rel->getROffset();

      for (auto &table : tables) {
        // Check if relocation falls within this table
        uint64_t tableStart = table.offsetBase + table.headerSize;
        uint64_t tableEnd = table.offsetBase + table.size;

        if (r_offset < tableStart || r_offset >= tableEnd)
          continue;

        // Find which entry it falls on
        uint64_t offsetInTable = r_offset - tableStart;
        if (offsetInTable % table.entrySize != 0)
          continue;

        size_t entryIndex = offsetInTable / table.entrySize;
        if (entryIndex >= table.offsets.size())
          continue;

        // Apply relocation: simple way is to replace with addend
        uint64_t relocated = static_cast<uint64_t>(rel->getRAddend());
        table.offsets[entryIndex] = static_cast<uint32_t>(relocated);
      }
    }
  }

  void writeDataTo(char *buffer) override {
    std::vector<uint8_t> out;

    for (const auto &table : tables) {
      size_t startOffset = out.size();

      // 1. Reserve space for unit_length (4 bytes)
      out.resize(out.size() + 4);

      // 2. Write header: version (2 bytes) + padding (2 bytes)
      out.push_back(table.version & 0xff);
      out.push_back((table.version >> 8) & 0xff);
      out.push_back(0); // padding byte 1
      out.push_back(0); // padding byte 2

      // 3. Write offsets, each is 4 bytes little endian
      for (uint32_t offset : table.offsets) {
        out.push_back(offset & 0xff);
        out.push_back((offset >> 8) & 0xff);
        out.push_back((offset >> 16) & 0xff);
        out.push_back((offset >> 24) & 0xff);
      }

      // 4. Fill in unit_length = total size minus length field
      uint32_t length = static_cast<uint32_t>(out.size() - startOffset - 4);
      out[startOffset + 0] = (length & 0xff);
      out[startOffset + 1] = (length >> 8) & 0xff;
      out[startOffset + 2] = (length >> 16) & 0xff;
      out[startOffset + 3] = (length >> 24) & 0xff;
    }

    // 5. Copy to target buffer
    assert(out.size() <= sh_size && "Rewritten .debug_str_offset larger than original");
    memcpy(buffer, out.data(), out.size());
  }

  void dumpData(std::ostream &oss) const override {
    // TODO dump debug str offset structure
    oss << ".debug_str_offsets contents:\n";
    for (const auto &table : tables) {
      oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << table.offsetBase
          << ": Contribution size = " << std::dec << table.size
          << ", Format = DWARF32"
          << ", Version = " << table.version << "\n";

      for (size_t i = 0; i < table.offsets.size(); ++i) {
        uint64_t absOffset = table.offsetBase + table.headerSize + i * table.entrySize;
        uint32_t strOffset = table.offsets[i];
        std::string str = debugStr.getString(strOffset) ;

        oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << absOffset
            << ": " << std::setw(8) << std::setfill('0') << strOffset
            << " \"" << str << "\"\n";
      }
    }
  }

  // Get table index by base offset
  int getTableIndex(uint64_t baseOffset) const {
    for (size_t i = 0; i < tables.size(); ++i) {
      if (tables[i].offsetBase == baseOffset)
        return static_cast<int>(i);
    }
    return -1;
  }

  // Get string offset by table index and string index
  uint32_t getStringOffset(int tableIndex, uint32_t strxIndex) const {
    if (tableIndex < 0 || static_cast<size_t>(tableIndex) >= tables.size())
      return 0;

    const auto &table = tables[tableIndex];
    if (strxIndex >= table.offsets.size())
      return 0;

    return table.offsets[strxIndex];
  }

  // Get string by table index and string index
  std::string getStringFromStrx(int tableIndex, uint32_t strxIndex) const {
    uint32_t offset = getStringOffset(tableIndex, strxIndex);
    return debugStr.getString(offset);
  }

};

class DebugLineSection final : public Section {
private:
  uint32_t total_length = 0;
  uint16_t version = 0;
  uint8_t address_size = 0;
  uint8_t segment_selector_size = 0;
  uint32_t prologue_length = 0;
  uint8_t min_inst_length = 0;
  uint8_t max_ops_per_inst = 0;
  uint8_t default_is_stmt = 0;
  int8_t line_base = 0;
  uint8_t line_range = 0;
  uint8_t opcode_base = 0;
  std::vector<uint8_t> standard_opcode_lengths;
  std::vector<std::string> include_directories;
  struct FileEntry {
    std::string name;
    uint64_t dir_index = 0;
    std::string md5;
  };
  std::vector<FileEntry> file_names;
  uint64_t prologueOffset = 0;

public:
  DebugLineSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
      : Section(SectionType::DebugLine, shdr, _data) {
    // TODO parse debug line offset structure
    const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
    const uint8_t *p = start;

    prologueOffset = 0;
    total_length = endian::read32le(p);
    p += 4;
    version = endian::read16le(p);
    p += 2;

    if (version != 5) return;

    address_size = *p++;
    segment_selector_size = *p++;
    prologue_length = endian::read32le(p);
    p += 4;

    const uint8_t *prologue_end = p + prologue_length;

    min_inst_length = *p++;
    max_ops_per_inst = *p++;
    default_is_stmt = *p++;
    line_base = static_cast<int8_t>(*p++);
    line_range = *p++;
    opcode_base = *p++;

    for (uint8_t i = 1; i < opcode_base; ++i) {
      standard_opcode_lengths.push_back(*p++);
    }

    uint8_t dir_format_count = *p++;
    std::vector<std::pair<uint16_t, uint16_t>> dir_formats;
    for (int i = 0; i < dir_format_count; ++i) {
      uint16_t content_type = endian::read16le(p);
      p += 2;
      uint16_t form = endian::read16le(p);
      p += 2;
      dir_formats.emplace_back(content_type, form);
    }

    uint8_t directory_count = *p++;
    for (int i = 0; i < directory_count; ++i) {
      for (auto &[content_type, form] : dir_formats) {
        skipFormValue(form, p, prologue_end);
      }
    }

    uint8_t file_format_count = *p++;
    std::vector<std::pair<uint16_t, uint16_t>> file_formats;
    for (int i = 0; i < file_format_count; ++i) {
      uint16_t content_type = endian::read16le(p); p += 2;
      uint16_t form = endian::read16le(p); p += 2;
      file_formats.emplace_back(content_type, form);
    }

    uint8_t file_count = *p++;
    for (int i = 0; i < file_count; ++i) {
      FileEntry entry;
      for (auto &[content_type, form] : file_formats) {
        if (form == 0x08 && content_type == 0x1) {
          entry.name = reinterpret_cast<const char *>(p);
          p += entry.name.size() + 1;
        } else if (form == 0x0f && content_type == 0x2) {
          unsigned bytesRead;
          entry.dir_index = decodeULEB128(p, &bytesRead); p += bytesRead;
        } else if (form == 0x1e && content_type == 0x5) {
          std::ostringstream ss;
          for (int i = 0; i < 16; ++i)
            ss << std::hex << std::setfill('0') << std::setw(2) << (int)p[i];
          entry.md5 = ss.str();
          p += 16;
        }
      }
      file_names.push_back(entry);
    }
  }


    void dumpData(std::ostream &oss) const override {
      // TODO dump debug line offset structure
      oss << ".debug_line contents:\n";
      oss << "debug_line[0x" << std::setw(8) << std::setfill('0') << std::hex
          << prologueOffset << "]\n";

      oss << "Line table prologue:\n";
      oss << "    total_length: 0x" << std::setw(8) << std::setfill('0')
          << std::hex << total_length << "\n";
      oss << "          format: DWARF32\n";
      oss << "         version: " << std::dec << version << "\n";
      oss << "    address_size: " << (int)address_size << "\n";
      oss << " seg_select_size: " << (int)segment_selector_size << "\n";
      oss << " prologue_length: 0x" << std::hex << prologue_length << "\n";
      oss << " min_inst_length: " << std::dec << (int)min_inst_length << "\n";
      oss << "max_ops_per_inst: " << (int)max_ops_per_inst << "\n";
      oss << " default_is_stmt: " << (int)default_is_stmt << "\n";
      oss << "       line_base: " << (int)line_base << "\n";
      oss << "      line_range: " << (int)line_range << "\n";
      oss << "     opcode_base: " << (int)opcode_base << "\n";

      for (size_t i = 0; i < standard_opcode_lengths.size(); ++i) {
        oss << "standard_opcode_lengths[DW_LNS_" << opcodeName(i + 1) << "] = "
            << (int)standard_opcode_lengths[i] << "\n";
      }
      for (size_t i = 0; i < include_directories.size(); ++i) {
        oss << "include_directories[" << std::setw(3) << i << "] = \""
            << include_directories[i] << "\"\n";
      }

      for (size_t i = 0; i < file_names.size(); ++i) {
        oss << "file_names[" << std::setw(3) << i << "]:\n";
        oss << "           name: \"" << file_names[i].name << "\"\n";
        oss << "      dir_index: " << file_names[i].dir_index << "\n";
        if (!file_names[i].md5.empty())
          oss << "   md5_checksum: " << file_names[i].md5 << "\n";
      }
    }
};

FormValueRaw parseFormValue(uint64_t form, const uint8_t *&p, const uint8_t *end,
                           const DebugStrOffsetsSection *strOffsetsSection,
                           const DebugStrSection *strSection,
                           const DebugAddrSection *addrSection,
                           uint64_t dieOffset,
                           int strOffsetsTableIndex,
                           uint64_t addrBaseOffset,
                           std::optional<int64_t> implicitConst = std::nullopt);
class DebugInfoSection final : public Section {
private:
  // Structure representing a compile unit header
  struct CompileUnitHeader {
    uint64_t offset;            // Offset in section
    uint32_t unitLength;        // Unit length
    uint16_t version;           // DWARF version
    uint8_t unitType;           // Unit type
    uint8_t addrSize;           // Address size
    uint32_t abbrevOffset;      // Abbreviation offset
    std::optional<uint64_t> dwoId; // Optional DWO ID
  };

  // Structure representing a DIE (Debugging Information Entry)
  struct DIE {
    uint64_t offset;            // Offset in section
    uint64_t abbrevCode;        // Abbreviation code
    const DebugAbbrevSection::AbbreviationDecl* abbrevDecl; // Abbreviation declaration
    std::vector<std::pair<uint64_t, FormValueRaw>> attributes; // Attributes
    std::vector<DIE> children;  // Child DIEs
  };

  std::vector<CompileUnitHeader> cuHeaders; // Compile unit headers
  std::vector<std::vector<DIE>> cuDIEs;  // Array of top-level DIEs for each CU
  const DebugAbbrevSection &abbrev;
  const DebugStrSection &debugStr;
  const DebugStrOffsetsSection &debugStrOffset;
  const DebugAddrSection &debugAddr;

public:
  DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data, const DebugAbbrevSection &abbrevRef, const DebugStrSection &strRef, const DebugStrOffsetsSection &strOffsetRef, const DebugAddrSection &addrRef)
      : Section(SectionType::DebugInfo, shdr, _data), abbrev(abbrevRef), debugStr(strRef), debugStrOffset(strOffsetRef), debugAddr(addrRef){
    // TODO parse debug info structure
    const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
    const uint8_t *end = start + sh_size;
    const uint8_t *p = start;

    while (p + 12 <= end) {
      uint64_t offset = p - start;

      // Reference: "DWARF5", page 200.
      uint32_t unitLength = *reinterpret_cast<const uint32_t *>(p); p += 4;
      uint16_t version = *reinterpret_cast<const uint16_t *>(p); p += 2;
      uint8_t unitType = *reinterpret_cast<const uint8_t *>(p); p += 1;
      uint8_t addrSize = *reinterpret_cast<const uint8_t *>(p); p += 1;
      uint32_t abbrevOffset = *reinterpret_cast<const uint32_t *>(p); p += 4;

      std::optional<uint64_t> dwoId;
      if (unitType == 0x04 || unitType == 0x05){
        if (p + 8 <= end){
          dwoId = *reinterpret_cast<const uint64_t *>(p);
          p += 8;
        }
      }

      cuHeaders.push_back({
          offset,
          unitLength,
          version,
          unitType,
          addrSize,
          abbrevOffset,
          dwoId
      });

      const uint8_t *cuEnd = start + offset + 4 + unitLength;

      // parse root DIE early to extract str_offsets_base
      int strOffsetsTableIndex = -1;
      uint64_t addrBaseOffset = 0;
      const uint8_t *tmp = p;

      uint64_t dieOffset = tmp - start;
      unsigned len = 0;
      uint64_t abbrevCode = decodeULEB128(tmp, &len, cuEnd);
      tmp += len;

      const auto *decl = abbrev.getAbbreviationDecl(abbrevOffset, abbrevCode);
      if (decl) {
        for (const auto &af : decl->attrForms) {
          if (af.attr == 0x72) {
            uint64_t val = parseFormValue(af.form, tmp, cuEnd, nullptr, nullptr, nullptr, dieOffset, strOffsetsTableIndex, addrBaseOffset).value;
            strOffsetsTableIndex = debugStrOffset.getTableIndex(val);
          }
          else if(af.attr == 0x73){
            addrBaseOffset = parseFormValue(af.form, tmp, cuEnd, nullptr, nullptr, nullptr,dieOffset, strOffsetsTableIndex, addrBaseOffset).value;

          }
          else {
            skipFormValue(af.form, tmp, cuEnd);  // Skip uninteresting attributes
          }
        }
      }

      std::vector<DIE> topLevelDIEs;
      while (p < cuEnd) {
        DIE die = parseDIE(p, start, cuEnd, abbrevOffset, strOffsetsTableIndex, addrBaseOffset);
        if (die.abbrevDecl == nullptr)
          break;
        topLevelDIEs.push_back(std::move(die));
      }
      cuDIEs.push_back(std::move(topLevelDIEs));
      p = start + offset + 4 + unitLength;
    }
  }


  DIE parseDIE(const uint8_t *&p, const uint8_t *start, const uint8_t *end,
               uint32_t abbrevOffset, int strOffsetsTableIndex = -1, uint64_t addrBaseOffset = 0) {
    uint64_t offset = p - start;
    unsigned len = 0;
    uint64_t abbrevCode = decodeULEB128(p, &len, end);
    p += len;

    if (abbrevCode == 0)
      return {};

    const auto *decl = abbrev.getAbbreviationDecl(abbrevOffset, abbrevCode);
    if (!decl) return {};

    DIE die;
    die.offset = offset;
    die.abbrevCode = abbrevCode;
    die.abbrevDecl = decl;

    for (const auto &af : decl->attrForms) {
      FormValueRaw valueRaw;
      if (af.form == 0x21)
      valueRaw = parseFormValue(af.form, p, end, &debugStrOffset, &debugStr, &debugAddr, die.offset, strOffsetsTableIndex, af.implicitConst.value());
      else valueRaw = parseFormValue(af.form, p, end, &debugStrOffset, &debugStr, &debugAddr, die.offset, strOffsetsTableIndex, addrBaseOffset);
      die.attributes.emplace_back(af.attr, valueRaw);
    }

    if (decl->hasChildren) {
      while (true) {
        auto child = parseDIE(p, start, end, abbrevOffset, strOffsetsTableIndex);
        if (child.abbrevDecl == nullptr) // Empty DIE indicates end of children
          break;
        die.children.push_back(std::move(child));
      }
    }

    return die;
  }


  void dumpData(std::ostream &oss) const override {
    // TODO dump debug info structure
    oss << ".debug_info contents:\n";
    // dump Compile Unit
    for (size_t i = 0; i < cuHeaders.size(); ++i) {
      const auto &cu = cuHeaders[i];
      const std::string unitTypeStr = getUnitType(cu.unitType);

      oss << std::hex << std::setfill('0');
      oss << "0x" << std::setw(8) << cu.offset << ": Compile Unit: ";
      oss << "length = 0x" << std::setw(8) << cu.unitLength << ", ";
      oss << "format = DWARF32, ";
      oss << "version = 0x" << std::setw(4) << cu.version << ", ";
      oss << "unit_type = " << unitTypeStr << ", ";
      oss << "abbr_offset = 0x" << std::setw(4) << cu.abbrevOffset << ", ";
      oss << "addr_size = 0x" << std::setw(2) << static_cast<int>(cu.addrSize) << " ";
      if (cu.dwoId.has_value())
      oss << ", DWO_id = " << std::setw(12) << cu.dwoId.value() << " ";
      // NextUnitOffset = Offset + Length + LengthFieldByteSize
      oss << "(next unit at 0x" << std::setw(8) << (cu.offset + 4 + cu.unitLength) << ")\n";
      oss << "\n";
      for (const auto &die : cuDIEs[i]) {
        dumpDIE(oss, die);
      }
    }
  }

  void dumpDIE(std::ostream &oss, const DIE &die) const {
    oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << die.offset
        << ": " << getTagName(die.abbrevDecl->tag) << "\n";

    for (const auto &[attr, val] : die.attributes) {
      if (val.form == 0x08 || val.form == 0x25){
        oss << std::string(14, ' ')
            << getAttrName(attr)
            << "\t(\"" << val.str << "\")\n";
      }
      else if (val.form == 0x03 || val.form == 0x04 || val.form == 0x09 || val.form == 0x0a || val.form == 0x18 || val.form == 0x1e){
        oss << std::string(14, ' ')
            << getAttrName(attr)
            << "\t[BLOCK DATA, size=" << val.blockData.size() << "]: ";

        for (uint8_t byte : val.blockData) {
          oss << "0x" << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte) << " ";
        }
        oss << std::dec << "\n";
      }
      else{
        oss << std::string(14, ' ')
            << getAttrName(attr)
            << "\t(" << val.value << ")\n";
      }
    }

    for (const auto &child : die.children)
      dumpDIE(oss, child);
  }

  void writeDataTo(char *buffer) override {
    std::vector<uint8_t> out;

    for (size_t i = 0; i < cuHeaders.size(); ++i) {
      const auto &cu = cuHeaders[i];
      size_t headerStart = out.size();

      // 1. Reserve space for unit_length (to be filled later)
      out.resize(out.size() + 4); // DWARF32 format

      // 2. Write header
      out.push_back(cu.version & 0xff);
      out.push_back((cu.version >> 8) & 0xff);
      out.push_back(cu.unitType);
      out.push_back(cu.addrSize);

      out.push_back(cu.abbrevOffset & 0xff);
      out.push_back((cu.abbrevOffset >> 8) & 0xff);
      out.push_back((cu.abbrevOffset >> 16) & 0xff);
      out.push_back((cu.abbrevOffset >> 24) & 0xff);

      if (cu.dwoId.has_value()) {
        uint64_t id = cu.dwoId.value();
        for (int j = 0; j < 8; ++j)
          out.push_back((id >> (j * 8)) & 0xff);
      }

      // 3. Write all top-level DIEs
      for (const auto &die : cuDIEs[i])
        writeDIE(die, out);

      // 4. Fill in unit_length
      uint32_t length = static_cast<uint32_t>(out.size() - headerStart - 4);
      out[headerStart + 0] = (length & 0xff);
      out[headerStart + 1] = (length >> 8) & 0xff;
      out[headerStart + 2] = (length >> 16) & 0xff;
      out[headerStart + 3] = (length >> 24) & 0xff;
    }


    // 5. Copy to target buffer
    assert(out.size() <= sh_size && "Rewritten .debug_info larger than original");
    memcpy(buffer, out.data(), out.size());
  }

  void writeDIE(const DIE &die, std::vector<uint8_t> &out) const {
    encodeULEB128(die.abbrevCode, out);

    const auto *decl = die.abbrevDecl;
    if (!decl) return;

    for (size_t i = 0; i < decl->attrForms.size(); ++i) {
      const auto &val = die.attributes[i].second;
      writeFormValue(val, out);
    }

    if (decl->hasChildren) {
      for (const auto &child : die.children)
        writeDIE(child, out);

      out.push_back(0x00); // Null abbrev code for end of children
    }
  }



};

FormValueRaw parseFormValue(uint64_t form, const uint8_t *&p, const uint8_t *end,
                           //                           const DebugInfoSection::CompileUnitHeader &cu,
                                                        const DebugStrOffsetsSection *strOffsets = nullptr,
                                                        const DebugStrSection *strSection = nullptr,
                                                      const DebugAddrSection *addrSection = nullptr,
                           uint64_t dieOffset = 0,
                           int strOffsetsTableIndex = -1,
                            uint64_t addrBaseOffset = 0,
                            std::optional<int64_t> implicitConst){
  FormValueRaw result;
  result.form = form;
  const uint8_t *start = p;

  switch (form) {
  case 0x01: { // DW_FORM_addr
    if (end - p < 8) {
      llvm::errs() << "Error: DW_FORM_addr: not enough bytes left in buffer\n";
      p = end;
      break;
    }
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x03: { // DW_FORM_block2
    uint16_t len = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x04: { // DW_FORM_block4
    uint32_t len = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x05: { // DW_FORM_data2
    result.value = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    break;
  }
  case 0x06: { // DW_FORM_data4
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x07: { // DW_FORM_data8
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x08: { // DW_FORM_string
    result.str = std::string(reinterpret_cast<const char *>(p));
    p += result.str.size() + 1;
    break;
  }
  case 0x09: { // DW_FORM_block
    unsigned size = 0;
    uint64_t len = decodeULEB128(p, &size, end);
    p += size;
    std::ostringstream oss;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x0a: { // DW_FORM_block1
    uint8_t len = *p++;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x0b: { // DW_FORM_data1
    result.value = *p++;
    break;
  }
  case 0x0c: { // DW_FORM_flag
    result.flag = (*p++) != 0;
    break;
  }
  case 0x0d: { // DW_FORM_sdata
    result.value = decodeSLEB128(p, end);
    break;
  }
  case 0x0e: { // DW_FORM_strp
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    if (strSection) {
      result.str = strSection->getString(result.value);
    }
    else {
      result.str = "";
    }
    break;
  }
  case 0x0f: { // DW_FORM_udata
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x10: // DW_FORM_ref_addr
  case 0x1c: // DW_FORM_ref_sup4
  case 0x24: // DW_FORM_ref_sup8
  case 0x20:
  case 0x14:{ // DW_FORM_ref_sig8
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x11: {
    result.value = *p++;
    break;
  }
  case 0x12: {
    result.value = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    break;
  }
  case 0x13: {
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x15: { // DW_FORM_ref_udata
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x16: { // DW_FORM_indirect
    unsigned len = 0;
    uint64_t actualForm = decodeULEB128(p, &len, end);
    p += len;
    return parseFormValue(actualForm, p, end, strOffsets, strSection, addrSection, dieOffset);
  }
  case 0x17: { // DW_FORM_sec_offset
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x18: { // DW_FORM_exprloc
    unsigned len = 0;
    uint64_t size = decodeULEB128(p, &len, end);
    p += len;
    result.blockData.insert(result.blockData.end(), p, p + size);
    p += size;
    break;
  }
  case 0x19: {
    result.flag = true;
    break;
  }
  case 0x1b: {
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x1d:
  case 0x1f: { // DW_FORM_strp_sup
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x1e: { // DW_FORM_data16
    result.blockData.insert(result.blockData.end(), p, p + 16);
    p += 16;
    break;
  }
  case 0x21: { // DW_FORM_implicit_const
    if (implicitConst.has_value())
      result.value = implicitConst.value();
    else
      llvm::errs() << "DW_FORM_implicit_const missing value at DIE offset 0x" << intToHex(dieOffset, 8) << "\n";
    break;
    break;
  }
  case 0x22:
  case 0x23: { // DW_FORM_rnglistx
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x25: { // DW_FORM_strx1
    result.value = *p++;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str = strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  }

  case 0x1a: // DW_FORM_strx
  case 0x26: // DW_FORM_strx2
  case 0x27: // DW_FORM_strx3
  case 0x28: { // DW_FORM_strx4
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str = strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  }
  case 0x29: case 0x2a: case 0x2b: case 0x2c: { // DW_FORM_addrx[1-4]
    unsigned len = 0;
    result.value = decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  default:
    llvm::errs() << "Unsupported form 0x" + intToHex(form, 2);
  }
  result.rawBytes.assign(start, p);
  return  result;
}

} // namespace iclang

#endif // ICLANG_SECTION_HPP
