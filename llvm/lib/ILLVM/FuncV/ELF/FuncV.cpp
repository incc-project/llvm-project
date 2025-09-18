#include "illvm/FuncV/ELF/FuncV.h"

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

bool FuncV::startsWith(const char *str, const char *prefix) {
  const size_t prefixLen = std::strlen(prefix);
  return std::strncmp(str, prefix, prefixLen) == 0;
}

std::pair<unsigned char, unsigned char>
FuncV::getElfArchType(const BinFile &binFile) {
  if (binFile.getFileSize() < llvm::ELF::EI_NIDENT) {
    return std::make_pair(static_cast<uint8_t>(llvm::ELF::ELFCLASSNONE),
                          static_cast<uint8_t>(llvm::ELF::ELFDATANONE));
  }
  const char *object = binFile.readBytes(0);
  return std::make_pair(static_cast<uint8_t>(object[llvm::ELF::EI_CLASS]),
                        static_cast<uint8_t>(object[llvm::ELF::EI_DATA]));
}

ELFKind FuncV::getELFKind(const BinFile &binFile) {
  const auto &logger = Logger::getInstance();

  const auto pr = getElfArchType(binFile);
  const auto size = pr.first;
  const auto endian = pr.second;

  const char *object = binFile.readBytes(0);
  if (!startsWith(object, llvm::ELF::ElfMagic)) {
    logger.fatal(binFile.getFilePath() + " is not an ELF file");
  }
  if (endian != llvm::ELF::ELFDATA2LSB && endian != llvm::ELF::ELFDATA2MSB) {
    logger.fatal(binFile.getFilePath() + " is a corrupted ELF file: "
                                         "invalid data encoding");
  }
  if (size != llvm::ELF::ELFCLASS32 && size != llvm::ELF::ELFCLASS64) {
    logger.fatal(binFile.getFilePath() + " is a corrupted ELF file: "
                                         "invalid file class");
  }

  const size_t fileSize = binFile.getFileSize();
  if ((size == llvm::ELF::ELFCLASS32 &&
       fileSize < sizeof(llvm::ELF::Elf32_Ehdr)) ||
      (size == llvm::ELF::ELFCLASS64 &&
       fileSize < sizeof(llvm::ELF::Elf64_Ehdr))) {
    logger.fatal(binFile.getFilePath() + " is a corrupted ELF file: "
                                         "file is too short");
  }

  if (size == llvm::ELF::ELFCLASS32) {
    return (endian == llvm::ELF::ELFDATA2LSB) ? ELFKind::ELF32LEKind
                                              : ELFKind::ELF32BEKind;
  }
  return (endian == llvm::ELF::ELFDATA2LSB) ? ELFKind::ELF64LEKind
                                            : ELFKind::ELF64BEKind;
}

llvm::Expected<std::unordered_set<std::string>>
FuncV::onlyLoadSymbolTable(const std::string &objPath) {
  const BinFile binFile(objPath);

  const ELFKind kind = getELFKind(binFile);

  std::unordered_set<std::string> res;

  if (kind != ELFKind::ELF64LEKind) {
    return res;
  }

  ObjFile objFile(binFile);

  // Parse ELF header and sections.
  if (auto err = objFile.init()) {
    return err;
  }

  // Load symbol table.
  const auto &symbols = objFile.getSymTab()->getSymbols();
  for (const auto &symbol : symbols) {
    const auto type = symbol->getStType();
    const auto bind = symbol->getStBind();
    const auto name = symbol->getNameValue();
    if (type == llvm::ELF::STT_FUNC &&
        (bind == llvm::ELF::STB_GLOBAL || bind == llvm::ELF::STB_WEAK) &&
        !name.empty()) {
      res.insert(name);
    }
  }

  return res;
}

llvm::Error FuncV::run(const std::string &oldObjPath, const std::string &newObjPath,
                const std::string &outputPath,
                const std::unordered_set<std::string> &funcXSet) {
  auto &logger = Logger::getInstance();

  if (oldObjPath == newObjPath) {
    logger.fatal("old-new overlap");
  }

  const BinFile oldBinFile(oldObjPath);
  const BinFile newBinFile(newObjPath);

  const ELFKind oldELFKind = getELFKind(oldBinFile);
  const ELFKind newELFKind = getELFKind(newBinFile);
  if (oldELFKind != newELFKind) {
    logger.fatal("old ELF kind != new ELF kind");
  }

  if (newELFKind != ELFKind::ELF64LEKind) {
    logger.fatal("We only support ELF64LEKind");
  }

  ObjFile oldObjFile(oldBinFile);
  ObjFile newObjFile(newBinFile);

  // Parse ELF header and sections.
  if (auto err = oldObjFile.init()) {
    return err;
  }
  if (auto err = newObjFile.init()) {
    return err;
  }

  auto reuseDriver = Reuse(oldObjFile, newObjFile, funcXSet);
  if (auto err = reuseDriver.run()) {
    return err;
  }

  // Reset layout.
  if (auto err = newObjFile.fini()) {
    return err;
  }

  newObjFile.save(outputPath);

  return llvm::Error::success();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
