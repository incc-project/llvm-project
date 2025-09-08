#include "illvm/FuncV/ELF/ObjFile.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Logger.h"

namespace illvm {
namespace funcv {
namespace elf {

bool ObjFile::getIsRela(const uint16_t m) {
  return m == llvm::ELF::EM_AARCH64 || m == llvm::ELF::EM_AMDGPU ||
         m == llvm::ELF::EM_HEXAGON || m == llvm::ELF::EM_PPC ||
         m == llvm::ELF::EM_PPC64 || m == llvm::ELF::EM_RISCV ||
         m == llvm::ELF::EM_X86_64;
}

void ObjFile::parseHeader() {
  const auto &logger = Logger::getInstance();

  using Elf_Ehdr = llvm::object::ELF64LE::Ehdr;

  const char *object = binFile.readBytes(0);
  const auto *ehdr = reinterpret_cast<const Elf_Ehdr *>(object);

  for (size_t i = 0; i < llvm::ELF::EI_NIDENT; i++) {
    e_ident[i] = ehdr->e_ident[i];
  }
  e_type = ehdr->e_type;
  e_machine = ehdr->e_machine;
  e_version = ehdr->e_version;
  e_entry = ehdr->e_entry;
  e_phoff = ehdr->e_phoff;
  e_shoff = ehdr->e_shoff;
  e_flags = ehdr->e_flags;
  e_ehsize = ehdr->e_ehsize;
  e_phentsize = ehdr->e_phentsize;
  e_phnum = ehdr->e_phnum;
  e_shentsize = ehdr->e_shentsize;
  e_shnum = ehdr->e_shnum;
  e_shstrndx = ehdr->e_shstrndx;

  // We do not support 32bit architecture.
  if (!getIsRela(e_machine)) {
    logger.fatal("We do not support rel");
  }
}

void ObjFile::parseStrSymTable(
    const char *object,
    const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs) {
  const auto &logger = Logger::getInstance();

  sections.resize(shdrs.size(), nullptr);

  std::shared_ptr<SymtabShndxSection> symTabShndx = nullptr;
  for (size_t i = 0; i < sections.size(); i++) {
    switch (shdrs[i]->sh_type) {
    case llvm::ELF::SHT_STRTAB:
      sections[i] = std::make_shared<StringTableSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      break;
    case llvm::ELF::SHT_SYMTAB:
      if (symTab != nullptr) {
        logger.fatal("multiple symbol tables");
      }
      symTab = std::make_shared<SymbolTableSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      sections[i] = symTab;
      break;
    case llvm::ELF::SHT_SYMTAB_SHNDX:
      if (symTabShndx != nullptr) {
        logger.fatal("multiple symtab shndx sections");
      }
      symTabShndx = std::make_shared<SymtabShndxSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      sections[i] = symTabShndx;
      break;
    default:
      break;
    }
  }
  if (symTab == nullptr) {
    logger.fatal("missing symbol table");
  }
  symTab->setSymtabShndx(symTabShndx);
  shstrTab = std::static_pointer_cast<StringTableSection>(sections[e_shstrndx]);
  if (shstrTab->getType() != SectionType::StrTab) {
    logger.fatal("invalid shstrtab");
  }
  strTab = std::static_pointer_cast<StringTableSection>(
      sections[symTab->getShLink()]);
  if (strTab->getType() != SectionType::StrTab) {
    logger.fatal("invalid strtab");
  }
}

void ObjFile::parseOtherSections(
    const char *object,
    const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs) {

  for (size_t i = 0; i < sections.size(); i++) {
    const auto secName = shstrTab->parseOriginalIndex(shdrs[i]->sh_name);
    if (secName->getValue() == ".eh_frame") {
      sections[i] = std::make_shared<EhFrameSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      ehFrame = std::static_pointer_cast<EhFrameSection>(sections[i]);
      continue;
    }
    switch (shdrs[i]->sh_type) {
    case llvm::ELF::SHT_STRTAB:
    case llvm::ELF::SHT_SYMTAB:
      continue;
    case llvm::ELF::SHT_RELA:
      sections[i] = std::make_shared<RelocationSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      if (secName->getValue() == ".rela.eh_frame") {
        relaEhFrame = std::static_pointer_cast<RelocationSection>(sections[i]);
      }
      break;
    case llvm::ELF::SHT_GROUP:
      sections[i] = std::make_shared<GroupSection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      break;
    default:
      sections[i] = std::make_shared<OrdinarySection>(
          shdrs[i], object + shdrs[i]->sh_offset);
      break;
    }
  }

  assert(ehFrame != nullptr && relaEhFrame != nullptr);
}

void ObjFile::parseReferences() {
  // 1 Parse section references.
  for (size_t i = 0; i < sections.size(); i++) {
    const auto section = sections[i];
    // Name offset -> strRef.
    section->setName(shstrTab->parseOriginalIndex(section->getShName()));
    // idx -> idxRef.
    section->setIdx(std::make_shared<IdxRef>(i));
  }
  // 2 Parse symbol references.
  symTab->parseReferences(sections, strTab);
  // 3 Parse section link, info references.
  for (size_t i = 0; i < sections.size(); i++) {
    const auto section = sections[i];
    // link idx -> section idx.
    if (i != 0 && section->getShLink() != 0) {
      section->setLink(sections[section->getShLink()]->getIdx());
    }
    // Parse sh_info.
    // sh_info for rela. (we do not consider SHT_REL)
    if (section->getShType() == llvm::ELF::SHT_RELA) {
      section->setInfoLink(sections[section->getShInfo()]->getIdx());
    }
    // sh_info for group.
    if (section->getShType() == llvm::ELF::SHT_GROUP) {
      section->setInfoLink(symTab->getSymbol(section->getShInfo())->getIdx());
    }
  }
  // 4 Parse other references: relocation, group, eh_frame.
  for (const auto &section : sections) {
    if (section->getType() == SectionType::RelaTab) {
      const auto relaSec = std::static_pointer_cast<RelocationSection>(section);
      relaSec->parseReferences(symTab);
    }
  }
  // Note that eh_frame depends on relocation.
  for (const auto &section : sections) {
    if (section->getType() == SectionType::Group) {
      const auto groupSec = std::static_pointer_cast<GroupSection>(section);
      groupSec->parseReferences(sections);
    } else if (section->getType() == SectionType::EhFrame) {
      const auto ehFrameSec = std::static_pointer_cast<EhFrameSection>(section);
      ehFrameSec->parseReferences(relaEhFrame);
    }
  }
}

void ObjFile::parseSections() {
  const auto &logger = Logger::getInstance();

  using Elf_Shdr = llvm::object::ELF64LE::Shdr;

  const char *object = binFile.readBytes(0);
  std::vector<const Elf_Shdr *> shdrs;
  shdrs.reserve(e_shnum);

  if (e_shentsize != sizeof(Elf_Shdr)) {
    logger.fatal("Invalid ELF file: e_shentsize != sizeof(Elf_Shdr)");
  }

  // 1. Parse shdrs.
  // 1.1. Add the first shdr.
  const auto *firstShdr = reinterpret_cast<const Elf_Shdr *>(object + e_shoff);
  shdrs.push_back(firstShdr);
  // The ELF header can only store numbers up to SHN_LORESERVE in the e_shnum
  // and e_shstrndx fields. When the value of one of these fields exceeds
  // SHN_LORESERVE ELF requires us to put sentinel values in the ELF header
  // and use fields in the section header at index 0 to store the value. The
  // sentinel values and fields are: e_shnum = 0, SHdrs[0].sh_size = number of
  // sections. e_shstrndx = SHN_XINDEX, SHdrs[0].sh_link = .shstrtab section
  // index.
  if (e_shnum == 0) {
    e_shnum = shdrs[0]->sh_size;
  }
  if (e_shstrndx == llvm::ELF::SHN_XINDEX) {
    e_shstrndx = shdrs[0]->sh_link;
  }
  // 1.2. Add other shdrs.
  for (uint64_t i = 1; i < e_shnum; i++) {
    const auto *shdr =
        reinterpret_cast<const Elf_Shdr *>(object + e_shoff + i * e_shentsize);
    shdrs.push_back(shdr);
  }

  // 2. Parse shdrs to sections.
  // 2.1. Parse string table and symbol table.
  parseStrSymTable(object, shdrs);
  // 2.2. Parse other sections.
  parseOtherSections(object, shdrs);

  // 3. Parse references.
  parseReferences();
}

std::shared_ptr<SymtabShndxSection> ObjFile::createSymtabShndx() {
  using Elf_Shdr = llvm::object::ELF64LE::Shdr;

  Elf_Shdr shdr;
  shdr.sh_name = 0;
  shdr.sh_type = llvm::ELF::SHT_SYMTAB_SHNDX;
  shdr.sh_flags = 0;
  shdr.sh_addr = 0;
  shdr.sh_offset = 0;
  shdr.sh_size = 0;
  shdr.sh_link = 0;
  shdr.sh_info = 0;
  shdr.sh_addralign = sizeof(llvm::object::ELF64LE::Word);
  shdr.sh_entsize = sizeof(llvm::object::ELF64LE::Word);

  // Create new section.
  const auto newSection = std::make_shared<SymtabShndxSection>(&shdr, nullptr);

  // Create new idx ref.
  newSection->setIdx(std::make_shared<IdxRef>(sections.size()));

  // Create new str ref.
  const std::string name = ".symtab_shndx";
  newSection->setName(shstrTab->push_back(name));

  // Update linkage.
  newSection->setLink(symTab->getIdx());

  sections.push_back(newSection);

  return newSection;
}

std::size_t ObjFile::alignOffset(const std::uint64_t offset,
                                 const std::uint64_t sh_addralign) {
  const auto &logger = Logger::getInstance();

  if (sh_addralign == 0) {
    logger.fatal("sh_addralign cannot be zero.");
  }
  if (offset % sh_addralign == 0) {
    return offset;
  }
  return (offset + sh_addralign - 1) & ~(sh_addralign - 1);
}

void ObjFile::layout() {
  // Update idx and size.
  int idx = 0;
  for (const auto &section : sections) {
    section->getIdx()->setValue(idx++);
    section->layout();
  }

  // Update offset.
  uint64_t secOff = e_ehsize;
  // Layout SHF_ALLOC sections before non-SHF_ALLOC sections. A non-SHF_ALLOC
  // will not occupy file offsets contained by a PT_LOAD.
  for (size_t i = 1; i < sections.size(); i++) {
    const auto section = sections[i];
    if (!(section->getShFlags() & llvm::ELF::SHF_ALLOC)) {
      continue;
    }
    secOff = alignOffset(secOff, section->getShAddralign());
    section->setShOffset(secOff);
    if (section->getShType() != llvm::ELF::SHT_NOBITS) {
      secOff += section->getShSize();
    }
  }
  // Layout non-SHF_ALLOC sections.
  for (size_t i = 1; i < sections.size(); i++) {
    const auto section = sections[i];
    if (section->getShFlags() & llvm::ELF::SHF_ALLOC) {
      continue;
    }
    secOff = alignOffset(secOff, section->getShAddralign());
    section->setShOffset(secOff);
    if (section->getShType() != llvm::ELF::SHT_NOBITS) {
      secOff += section->getShSize();
    }
  }
  // Update section header table offset.
  secOff = alignOffset(secOff, WordSize);
  e_shoff = secOff;
}

void ObjFile::init() {
  parseHeader();
  parseSections();
}

void ObjFile::fini() {
  // 1. Create necessary sections if needed.
  if (symTab->needSymtabShNdx() && symTab->getSymtabShndx() == nullptr) {
    symTab->setSymtabShndx(createSymtabShndx());
  }

  // 2. Layout.
  layout();

  // 3. Fini each section.
  for (const auto &section : sections) {
    section->fini();
    // Update name.
    section->setShName(section->getName()->getOffset());
    // Update link.
    const auto &link = section->getLink();
    if (section->getIdxValue() != 0 && link != nullptr) {
      section->setShLink(link->getValue());
    }
    // Update info.
    const auto &infoLink = section->getInfoLink();
    if (infoLink != nullptr) {
      section->setShInfo(infoLink->getValue());
    }
  }

  // update e_shnum
  e_shnum = sections.size();

  // Note: Update the first section.
  // The ELF header can only store numbers up to SHN_LORESERVE in the e_shnum
  // and e_shstrndx fields. When the value of one of these fields exceeds
  // SHN_LORESERVE ELF requires us to put sentinel values in the ELF header
  // and use fields in the section header at index 0 to store the value. The
  // sentinel values and fields are: e_shnum = 0, SHdrs[0].sh_size = number of
  // sections. e_shstrndx = SHN_XINDEX, SHdrs[0].sh_link = .shstrtab section
  // index.
  if (e_shnum >= llvm::ELF::SHN_LORESERVE) {
    sections[0]->setShSize(e_shnum);
    e_shnum = 0;
  }
  if (e_shstrndx >= llvm::ELF::SHN_LORESERVE) {
    sections[0]->setShLink(e_shstrndx);
    e_shstrndx = llvm::ELF::SHN_XINDEX;
  }
}

void ObjFile::save(const std::string &outputPath) const {
  const auto &logger = Logger::getInstance();

  // Note that e_shnum may be set to 0, here we should use sections.size().
  const uint64_t fileSize = e_shoff + sections.size() * e_shentsize;
  auto *buffer = new char[fileSize];
  memset(buffer, 0, fileSize);

  // 1. Write ELF header.
  using Elf_Ehdr = llvm::object::ELF64LE::Ehdr;
  auto *ehdr = reinterpret_cast<Elf_Ehdr *>(buffer);

  for (size_t i = 0; i < llvm::ELF::EI_NIDENT; i++) {
    ehdr->e_ident[i] = e_ident[i];
  }
  ehdr->e_type = e_type;
  ehdr->e_machine = e_machine;
  ehdr->e_version = e_version;
  ehdr->e_entry = e_entry;
  ehdr->e_phoff = e_phoff;
  ehdr->e_shoff = e_shoff;
  ehdr->e_flags = e_flags;
  ehdr->e_ehsize = e_ehsize;
  ehdr->e_phentsize = e_phentsize;
  ehdr->e_phnum = e_phnum;
  ehdr->e_shentsize = e_shentsize;
  ehdr->e_shnum = e_shnum;
  ehdr->e_shstrndx = e_shstrndx;

  // 2. Write Section data.
  // skip section 0 (NULL).
  for (size_t i = 1; i < sections.size(); i++) {
    const auto section = sections[i];
    if (section->getShType() != llvm::ELF::SHT_NOBITS) {
      section->writeDataTo(buffer + section->getShOffset());
    }
  }

  // 3. Write Section header table.
  using Elf_Shdr = llvm::object::ELF64LE::Shdr;
  auto *shdr = reinterpret_cast<Elf_Shdr *>(buffer + e_shoff);
  for (const auto &section : sections) {
    section->writeHeaderTo(shdr);
    ++shdr;
  }

  // 4. Write buffer.
  std::ofstream outputFile(outputPath, std::ios::binary);
  if (!outputFile) {
    logger.fatal("can not open " + outputPath);
  }
  outputFile.write(buffer, fileSize);
  outputFile.close();

  delete[] buffer;
}

void ObjFile::dump(std::ostream &oss) const {
  oss << "==================== Ehdr ====================" << std::endl;
  oss << "[Magic]";
  oss << std::hex;
  for (const unsigned char i : e_ident) {
    oss << " " << static_cast<int>(i);
  }
  oss << std::endl;
  oss << std::dec;
  // http://www.uxsglobal.com/developers/gabi/latest/ch4.eheader.html
  oss << "[e_machine] " << e_machine << std::endl;
  oss << "[e_shoff] " << e_shoff << " (bytes into file)" << std::endl;
  oss << "[e_ehsize] " << e_ehsize << " (bytes)" << std::endl;
  oss << "[e_shentsize] " << e_shentsize << " (bytes)" << std::endl;
  oss << "[e_shnum] " << e_shnum << "(" << sections.size() << ")" << std::endl;
  oss << "[e_shstrndx] " << (shstrTab == nullptr ? -1 : shstrTab->getIdxValue())
      << std::endl;

  oss << "==================== Shdr ====================" << std::endl;

  oss << std::setfill(' ');
  oss << "[" << std::setw(5) << "Nr" << "] ";
  oss << std::setw(20) << "Name" << " ";
  oss << std::setw(20) << "Type" << " ";
  oss << std::setw(6) << "Off" << " ";
  oss << std::setw(6) << "Size" << " ";
  oss << std::setw(4) << "ES" << " ";
  oss << std::setw(4) << "Flg" << " ";
  oss << std::setw(5) << "Lk" << " ";
  oss << std::setw(4) << "Inf" << " ";
  oss << std::setw(4) << "Al";
  oss << std::endl;

  for (const auto &section : sections) {
    section->dumpHeader(oss);
    oss << std::endl;
  }

  oss << "==================== Sections' Data ===================="
      << std::endl;
  for (const auto &section : sections) {
    oss << "[" << section->getIdxValue() << "] " << section->getNameValue()
        << std::endl;
    section->dumpData(oss);
    oss << std::endl;
  }
}

std::string ObjFile::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
