#include "illvm/FuncV/ELF/ObjFile.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

bool ObjFile::getIsRela(const uint16_t m) {
  return m == llvm::ELF::EM_AARCH64 || m == llvm::ELF::EM_AMDGPU ||
         m == llvm::ELF::EM_HEXAGON || m == llvm::ELF::EM_PPC ||
         m == llvm::ELF::EM_PPC64 || m == llvm::ELF::EM_RISCV ||
         m == llvm::ELF::EM_X86_64;
}

llvm::Error ObjFile::parseHeader() {
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
  ILLVM_ECHECK(getIsRela(e_machine), "ILLVM do not support rel");

  return llvm::Error::success();
}

llvm::Error ObjFile::parseStrSymTable(
    const char *object,
    const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs) {
  sections.resize(shdrs.size(), nullptr);

  std::shared_ptr<SymtabShndxSection> symTabShndx = nullptr;
  for (size_t i = 0; i < sections.size(); i++) {
    switch (shdrs[i]->sh_type) {
    case llvm::ELF::SHT_STRTAB:
      if (auto err =
              StringTableSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(sections[i])) {
        return err;
      }
      break;
    case llvm::ELF::SHT_SYMTAB:
      ILLVM_FCHECK(symTab == nullptr, "multiple symbol tables");
      if (auto err =
              SymbolTableSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(symTab)) {
        return err;
      }
      sections[i] = symTab;
      break;
    case llvm::ELF::SHT_SYMTAB_SHNDX:
      ILLVM_FCHECK(symTabShndx == nullptr, "multiple symtab shndx sections");
      if (auto err =
              SymtabShndxSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(symTabShndx)) {
        return err;
      }
      sections[i] = symTabShndx;
      break;
    default:
      break;
    }
  }
  ILLVM_FCHECK(symTab != nullptr, "missing symbol table");
  symTab->setSymtabShndx(symTabShndx);
  shstrTab = std::static_pointer_cast<StringTableSection>(sections[e_shstrndx]);
  ILLVM_FCHECK(shstrTab->getType() == SectionType::StrTab, "");
  strTab = std::static_pointer_cast<StringTableSection>(
      sections[symTab->getShLink()]);
  ILLVM_FCHECK(strTab->getType() == SectionType::StrTab, "");
  return llvm::Error::success();
}

llvm::Error ObjFile::parseOtherSections(
    const char *object,
    const std::vector<const llvm::object::ELF64LE::Shdr *> &shdrs) {
  // const llvm::object::ELF64LE::Shdr *debugInfoShdr = nullptr;
  // const char *debugInfoData = nullptr;
  // size_t debugInfoIndex = -1;
  //
  // const llvm::object::ELF64LE::Shdr *debugStrOffShdr = nullptr;
  // const char *debugStrOffData = nullptr;
  // size_t debugStrOffIndex = -1;
  //
  // const llvm::object::ELF64LE::Shdr *debugAddrShdr = nullptr;
  // const char *debugAddrData = nullptr;
  // size_t debugAddrIndex = -1;
  //
  // const llvm::object::ELF64LE::Shdr *debugRnglistShdr = nullptr;
  // const char *debugRnglistData = nullptr;
  // size_t debugRnglistIndex = -1;

  for (size_t i = 0; i < sections.size(); i++) {
    const auto secName = shstrTab->parseOriginalIndex(shdrs[i]->sh_name);
    const auto secNameStr = secName->getValue();

    if (secNameStr == ".eh_frame") {
      if (auto err =
              EhFrameSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(ehFrame)) {
        return err;
      }

      sections[i] = ehFrame;
      continue;
    }

    // if (secNameStr == ".debug_abbrev") {
    //   debugAbbrev = std::make_shared<DebugAbbrevSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   sections[i] = debugAbbrev; // 保存进 sections 映射
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_info") {
    //   // 延迟构造 .debug_info，暂时记录必要信息
    //   debugInfoShdr = shdrs[i];
    //   debugInfoData = object + shdrs[i]->sh_offset;
    //   debugInfoIndex = i;
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_str_offsets") {
    //   debugStrOffShdr = shdrs[i];
    //   debugStrOffData = object + shdrs[i]->sh_offset;
    //   debugStrOffIndex = i;
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_str") {
    //   debugStr = std::make_shared<DebugStrSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   sections[i] = debugStr;
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_line_str") {
    //   debugLineStr = std::make_shared<DebugLineStrSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   sections[i] = debugLineStr;
    //   continue;
    // }
    //
    // if (secNameStr == ".rela.debug_str_offsets") {
    //   relaDebugStrOffsets = std::static_pointer_cast<RelocationSection>(
    //       sections[i] = std::make_shared<RelocationSection>(
    //           shdrs[i], object + shdrs[i]->sh_offset));
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_addr") {
    //   debugAddrShdr = shdrs[i];
    //   debugAddrData = object + shdrs[i]->sh_offset;
    //   debugAddrIndex = i;
    //   continue;
    // }
    //
    // if (secNameStr == ".rela.debug_addr") {
    //   relaDebugAddr = std::static_pointer_cast<RelocationSection>(
    //       sections[i] = std::make_shared<RelocationSection>(
    //           shdrs[i], object + shdrs[i]->sh_offset));
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_line") {
    //   sections[i] = std::make_shared<DebugLineSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_rnglists") {
    //   debugRnglistShdr = shdrs[i];
    //   debugRnglistData = object + shdrs[i]->sh_offset;
    //   debugRnglistIndex = i;
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_loclists") {
    //   sections[i] = std::make_shared<DebugLoclistsSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   continue;
    // }
    //
    // if (secNameStr == ".debug_aranges") {
    //   sections[i] = std::make_shared<DebugArangeSection>(
    //       shdrs[i], object + shdrs[i]->sh_offset);
    //   continue;
    // }

    switch (shdrs[i]->sh_type) {
    case llvm::ELF::SHT_STRTAB:
    case llvm::ELF::SHT_SYMTAB:
      continue;
    case llvm::ELF::SHT_RELA:
      if (auto err =
              RelocationSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(sections[i])) {
        return err;
      }
      if (secName->getValue() == ".rela.eh_frame") {
        relaEhFrame = std::static_pointer_cast<RelocationSection>(sections[i]);
      }
      break;
    case llvm::ELF::SHT_GROUP:
      if (auto err =
              GroupSection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(sections[i])) {
        return err;
      }
      break;
    default:
      if (auto err =
              OrdinarySection::Create(shdrs[i], object + shdrs[i]->sh_offset)
                  .moveInto(sections[i])) {
        return err;
      }
      break;
    }
  }

  ILLVM_FCHECK(ehFrame != nullptr && relaEhFrame != nullptr,
               "Missing eh_frame");

  // TODO Handle rela after 3.4.
  // if (debugStrOffShdr) {
  //   logger.assertTrue(debugStr != nullptr, "Missing debug str");
  //   debugStrOff = std::make_shared<DebugStrOffsetsSection>(
  //       debugStrOffShdr, debugStrOffData, *debugStr);
  //   sections[debugStrOffIndex] = debugStrOff;
  //
  //   if (relaDebugStrOffsets) {
  //     debugStrOff->applyRelocations(relaDebugStrOffsets->getRelocations());
  //   }
  // }
  //
  // if (debugAddrShdr) {
  //   debugAddr =
  //       std::make_shared<DebugAddrSection>(debugAddrShdr, debugAddrData);
  //   sections[debugAddrIndex] = debugAddr;
  //
  //   if (relaDebugAddr) {
  //     debugAddr->applyRelocations(relaDebugAddr->getRelocations());
  //   }
  // }
  //
  // if (debugRnglistShdr) {
  //   //      assert(debugAddr != nullptr);
  //   debugRnglist = std::make_shared<DebugRnglistSection>(
  //       debugRnglistShdr, debugRnglistData,
  //       debugAddr ? debugAddr.get() : nullptr);
  //   sections[debugRnglistIndex] = debugRnglist;
  // }
  //
  // if (debugInfoShdr) {
  //   logger.assertTrue(debugAbbrev != nullptr, "Missing debug abbrev");
  //   debugInfoSection = std::make_shared<DebugInfoSection>(
  //       debugInfoShdr, debugInfoData, *debugAbbrev,
  //       debugStr ? debugStr.get() : nullptr,
  //       debugStrOff ? debugStrOff.get() : nullptr,
  //       debugAddr ? debugAddr.get() : nullptr,
  //       debugRnglist ? debugRnglist.get() : nullptr);
  //   sections[debugInfoIndex] = debugInfoSection;
  // }

  return llvm::Error::success();
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

llvm::Error ObjFile::parseSections() {
  using Elf_Shdr = llvm::object::ELF64LE::Shdr;

  const char *object = binFile.readBytes(0);
  std::vector<const Elf_Shdr *> shdrs;
  shdrs.reserve(e_shnum);

  ILLVM_FCHECK(e_shentsize == sizeof(Elf_Shdr), "");

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
  if (auto err = parseStrSymTable(object, shdrs)) {
    return err;
  }
  // 2.2. Parse other sections.
  if (auto err = parseOtherSections(object, shdrs)) {
    return err;
  }

  // 3. Parse references.
  parseReferences();

  return llvm::Error::success();
}

  llvm::Expected<std::shared_ptr<SymtabShndxSection>> ObjFile::createSymtabShndx() {
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
  std::shared_ptr<SymtabShndxSection> newSection;
  if (auto err = SymtabShndxSection::Create(&shdr, nullptr).moveInto(newSection)) {
    return err;
  }

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
  ILLVM_FCHECK(sh_addralign != 0, "");
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

llvm::Error ObjFile::init() {
  if (auto err = parseHeader()) {
    return err;
  }
  if (auto err = parseSections()) {
    return err;
  }
  return llvm::Error::success();
}

llvm::Error ObjFile::fini() {
  // 1. Create necessary sections if needed.
  if (symTab->needSymtabShNdx() && symTab->getSymtabShndx() == nullptr) {
    std::shared_ptr<SymtabShndxSection> newSection;
    if (auto err = createSymtabShndx().moveInto(newSection)) {
      return err;
    }
    symTab->setSymtabShndx(newSection);
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

  return llvm::Error::success();
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
