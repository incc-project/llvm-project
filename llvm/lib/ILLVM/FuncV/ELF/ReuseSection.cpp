#include "illvm/FuncV/ELF/ReuseSection.h"

namespace illvm {
namespace funcv {
namespace elf {

llvm::Expected<std::shared_ptr<Section>>
ReuseSection::createNewSection(ObjFile &newObjFile,
                               const std::shared_ptr<Section> &oldSection) {
  using Elf_Shdr = llvm::object::ELF64LE::Shdr;

  Elf_Shdr shdr;
  shdr.sh_name = 0;
  shdr.sh_type = oldSection->getShType();
  shdr.sh_flags = oldSection->getShFlags();
  shdr.sh_addr = 0;
  shdr.sh_offset = 0;
  shdr.sh_size = oldSection->getShSize();
  // This function is only for text/data sections, just set link/info to 0.
  shdr.sh_link = 0;
  shdr.sh_info = 0;
  shdr.sh_addralign = oldSection->getShAddralign();
  shdr.sh_entsize = oldSection->getEntSize();

  // Create new section.
  // Set data. (shadow copy)
  std::shared_ptr<Section> newSection;
  if (auto err = OrdinarySection::Create(&shdr, oldSection->getData())
                     .moveInto(newSection)) {
    return err;
  }

  // Create new idx ref.
  newSection->setIdx(std::make_shared<IdxRef>(newObjFile.getSections().size()));

  // Create new str ref.
  const auto name = oldSection->getNameValue();
  newSection->setName(newObjFile.getShstrTab()->push_back(name));

  newObjFile.getSections().push_back(newSection);

  return newSection;
}

llvm::Error ReuseSection::run(ObjFile &newObjFile, const BDG &bdg) {
  const auto &funcVReuseNodes = bdg.getFuncVReuseNodes();

  // Old section -> new section.
  std::unordered_map<std::shared_ptr<Section>, std::shared_ptr<Section>>
      visited;

  for (const auto &p : funcVReuseNodes) {
    const auto reuseNode = p.second;

    const auto oldSection = reuseNode->getOldSection();
    if (oldSection == nullptr || reuseNode->getNewSection() != nullptr) {
      continue;
    }

    const auto it = visited.find(oldSection);
    if (it != visited.end()) {
      reuseNode->setNewSection(it->second);
      continue;
    }

    std::shared_ptr<Section> newSection;
    if (auto err =
            createNewSection(newObjFile, oldSection).moveInto(newSection)) {
      return err;
    }
    reuseNode->setNewSection(newSection);
    visited.emplace(oldSection, newSection);
  }
  return llvm::Error::success();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
