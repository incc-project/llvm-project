#include "illvm/FuncV/ELF/GroupSection.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

GroupSection::GroupSection(const llvm::object::ELF64LE::Shdr *shdr,
                           const char *_data, llvm::Error &err)
    : Section(SectionType::Group, shdr, _data) {
  ILLVM_FCHECK(shdr->sh_entsize == sizeof(uint32_t), "");
  ILLVM_FCHECK(shdr->sh_size % shdr->sh_entsize == 0, "");
}

void GroupSection::parseReferences(
    const std::vector<std::shared_ptr<Section>> &allSections) {
  const size_t num = sh_size / sh_entsize;
  const uint32_t *secNdx =
      reinterpret_cast<uint32_t *>(const_cast<char *>(data));
  for (size_t i = 1; i < num; i++) {
    sections.push_back(allSections[secNdx[i]]);
  }
}

void GroupSection::layout() {
  sh_size = sections.size() * sh_entsize + sh_entsize;
}

void GroupSection::writeDataTo(char *buffer) {
  uint64_t off = 0;
  uint32_t comdatFlag = llvm::ELF::GRP_COMDAT;
  memcpy(buffer, &comdatFlag, sh_entsize);
  off += sh_entsize;
  for (const auto &sec : sections) {
    const uint32_t secNdx = sec->getIdxValue();
    memcpy(buffer + off, &secNdx, sh_entsize);
    off += sh_entsize;
  }
}

void GroupSection::dumpData(std::ostream &oss) const {
  for (const auto &sec : sections) {
    oss << "section: " << sec->getIdxValue() << " " << sec->getNameValue()
        << std::endl;
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
