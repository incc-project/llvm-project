#include "illvm/FuncV/ELF/ReuseEhFrame.h"

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

std::shared_ptr<CFI>
ReuseEhFrame::createNewCFI(ObjFile &newObjFile,
                           const std::shared_ptr<CFI> &oldCFI) {
  const auto oldCIE = oldCFI->getCIE();
  // Create new CIE.
  const auto newCIE =
      std::make_shared<CIE>(oldCIE->getLength(), oldCIE->getExtLength(),
                            oldCIE->getCIEID(), oldCIE->getOtherData());
  // Create new CFI.
  const auto newCFI = std::make_shared<CFI>(newCIE);
  newObjFile.getEhFrame()->addCFI(newCFI);

  return newCFI;
}

std::shared_ptr<FDE>
ReuseEhFrame::createNewFDE(const std::shared_ptr<CFI> &newCFI,
                           const std::shared_ptr<FDE> &oldFDE) {
  // Create FDE.
  const auto newFDE = std::make_shared<FDE>(
      oldFDE->getLength(), oldFDE->getExtLength(), 0, oldFDE->getOtherData());

  newCFI->addFDE(newFDE);

  return newFDE;
}

llvm::Error ReuseEhFrame::run(ObjFile &newObjFile, const BDG &bdg) {
  const auto &funcVReuseNodes = bdg.getFuncVReuseNodes();

  // Old CFI -> new CFI.
  std::unordered_map<std::shared_ptr<CFI>, std::shared_ptr<CFI>> visitedCFI;

  // Old FDE -> new FDE.
  std::unordered_map<std::shared_ptr<FDE>, std::shared_ptr<FDE>> visitedFDE;

  for (const auto &p : funcVReuseNodes) {
    const auto reuseNode = p.second;

    const auto oldCFI = reuseNode->getOldCFI();
    if (oldCFI == nullptr) {
      continue;
    }

    // (1) Reuse CFI.
    std::shared_ptr<CFI> newCFI = reuseNode->getNewCFI();
    if (newCFI == nullptr) {
      const auto cfiIt = visitedCFI.find(oldCFI);
      if (cfiIt != visitedCFI.end()) {
        newCFI = cfiIt->second;
        reuseNode->setNewCFI(newCFI);
      } else {
        newCFI = createNewCFI(newObjFile, oldCFI);
        reuseNode->setNewCFI(newCFI);
        visitedCFI.emplace(oldCFI, newCFI);
      }
    }

    // (2) Reuse FDE.
    const auto oldFDE = reuseNode->getOldFDE();
    ILLVM_ECHECK(oldFDE != nullptr, "");
    if (reuseNode->getNewFDE() == nullptr) {
      const auto fdeIt = visitedFDE.find(oldFDE);
      if (fdeIt != visitedFDE.end()) {
        reuseNode->setNewFDE(fdeIt->second);
      } else {
        const auto newFDE = createNewFDE(newCFI, oldFDE);
        reuseNode->setNewFDE(newFDE);
        visitedFDE.emplace(oldFDE, newFDE);
      }
    }
  }
  return llvm::Error::success();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
