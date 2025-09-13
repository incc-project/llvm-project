#include "illvm/FuncV/ELF/BDG.h"

#include <iomanip>
#include <queue>
#include <sstream>

#include "illvm/Support/Logger.h"
#include "illvm/Support/Strings.h"

namespace illvm {
namespace funcv {
namespace elf {

void BDG::loadOldReuseVersion(ObjFile &oldObjFile) {
  const auto &symbols = oldObjFile.getSymTab()->getSymbols();
  for (const auto &symbol : symbols) {
    const auto name = symbol->getNameValue();
    if (name != ".iclang.reusev") {
      continue;
    }
    oldReuseVersion = symbol->getStValue();
    newReuseVersion = oldReuseVersion + 1;
  }
}

void BDG::renameSymbol(ObjFile &objFile, const std::string &symbolName,
                       const std::shared_ptr<Symbol> &symbol,
                       const uint64_t reuseVersion) {
  if (symbolName.find("@") == std::string::npos) {
    const auto newSymbolName = symbolName + "@" + std::to_string(reuseVersion);
    symbol->setName(objFile.getStrTab()->push_back(newSymbolName));
  }
}

void BDG::renameSection(ObjFile &objFile, const std::string &sectionName,
                        const std::shared_ptr<Section> &section,
                        const uint64_t reuseVersion) {
  if (sectionName.find("@") == std::string::npos) {
    const auto newSectionName =
        sectionName + "@" + std::to_string(reuseVersion);
    section->setName(objFile.getShstrTab()->push_back(newSectionName));
  }
}

void BDG::renameAnonymousSymbol(ObjFile &objFile, const uint64_t reuseVersion) {
  const auto &symbols = objFile.getSymTab()->getSymbols();
  const auto &sections = objFile.getSections();

  for (const auto &symbol : symbols) {
    const std::string symbolName = symbol->getNameValue();

    const auto secIdx = symbol->getSecIdx();
    if (secIdx == nullptr) {
      continue;
    }

    const auto section = sections[secIdx->getValue()];
    const auto sectionName = section->getNameValue();

    if (Strings::hasPrefix(symbolName, "GCC_except_table")) {
      renameSymbol(objFile, symbolName, symbol, reuseVersion);
    } else if (Strings::hasPrefix(sectionName, ".rodata.str") ||
               Strings::hasPrefix(sectionName, ".rodata.cst")) {
      renameSection(objFile, sectionName, section, reuseVersion);
      renameSymbol(objFile, symbolName, symbol, reuseVersion);
    }
  }
}

// Symbol name -> symbol.
BDG::SymbolNameMapType BDG::symbolNameMapping(ObjFile &objFile) {
  SymbolNameMapType res;

  const auto &symbols = objFile.getSymTab()->getSymbols();
  for (const auto &symbol : symbols) {
    const auto name = symbol->getNameValue();
    if (name.empty() || name == ".iclang.reusev") {
      continue;
    }

    const auto type = symbol->getStType();
    if (type != llvm::ELF::STT_FILE) {
      res[name] = symbol;
    }
  }

  return res;
}

// Symbol -> section.
BDG::SymbolSectionMapType BDG::symbolSectionMapping(ObjFile &objFile) {
  SymbolSectionMapType res;

  const auto &symbols = objFile.getSymTab()->getSymbols();
  const auto &sections = objFile.getSections();

  for (const auto &symbol : symbols) {
    // NOTYPE symbol can also have section ndx, e.g.
    // -------------------------------------------
    // | NOTYPE | LOCAL | DEFAULT | 3 | .LCPI0_0 |
    // -------------------------------------------
    const auto secIdx = symbol->getSecIdx();
    if (secIdx != nullptr) {
      res[symbol] = sections[secIdx->getValue()];
    }
  }

  return res;
}

// Section -> rela section.
BDG::SectionRelaMapType BDG::sectionRelaMapping(ObjFile &objFile) {
  SectionRelaMapType res;

  const auto &sections = objFile.getSections();

  for (const auto &section : sections) {
    const auto type = section->getType();
    if (type == SectionType::RelaTab) {
      const auto info = section->getInfoLink();
      if (info != nullptr) {
        res[sections[info->getValue()]] =
            std::static_pointer_cast<RelocationSection>(section);
      }
    }
  }

  return res;
}

void BDG::initReuseNodesWithSymbol(const SymbolNameMapType &oldSymbolNameMap,
                                   const SymbolNameMapType &newSymbolNameMap) {
  for (auto &p : oldSymbolNameMap) {
    const auto oldSymbolName = p.first;
    const auto oldSymbol = p.second;

    const auto reuseNode = std::make_shared<ReuseNode>(oldSymbol);

    // matching
    const auto it = newSymbolNameMap.find(oldSymbolName);
    if (it != newSymbolNameMap.end()) {
      reuseNode->setNewSymbol(it->second);
    }

    reuseNodeIdrMap[oldSymbolName] = reuseNode;
  }
}

void BDG::addTDSectionsForReusedNodes(ObjFile &oldObjFile,
                                      ObjFile &newObjFile) const {
  // symbol -> section.
  const auto oldSymbolSectionMap = symbolSectionMapping(oldObjFile);
  const auto newSymbolSectionMap = symbolSectionMapping(newObjFile);

  for (const auto &p : reuseNodeIdrMap) {
    const auto reuseNode = p.second;

    // Update sections.
    const auto oldSymbol = reuseNode->getOldSymbol();
    auto it = oldSymbolSectionMap.find(oldSymbol);
    if (it != oldSymbolSectionMap.end()) {
      reuseNode->setOldSection(it->second);
    }
    const auto newSymbol = reuseNode->getNewSymbol();
    if (newSymbol != nullptr) {
      it = newSymbolSectionMap.find(newSymbol);
      if (it != newSymbolSectionMap.end()) {
        reuseNode->setNewSection(it->second);
      }
    }
  }
}

void BDG::addRelaSectionsForReusedNodes(ObjFile &oldObjFile,
                                        ObjFile &newObjFile) const {
  // section -> rela section.
  const auto oldSectionRelaMap = sectionRelaMapping(oldObjFile);
  const auto newSectionRelaMap = sectionRelaMapping(newObjFile);

  for (const auto &p : reuseNodeIdrMap) {
    const auto reuseNode = p.second;

    // Update rela sections.
    const auto oldSection = reuseNode->getOldSection();
    if (oldSection != nullptr) {
      const auto it = oldSectionRelaMap.find(oldSection);
      if (it != oldSectionRelaMap.end()) {
        reuseNode->setOldRelaSection(it->second);
      }
    }
    const auto newSection = reuseNode->getNewSection();
    if (newSection != nullptr) {
      const auto it = newSectionRelaMap.find(newSection);
      if (it != newSectionRelaMap.end()) {
        reuseNode->setNewRelaSection(it->second);
      }
    }
  }
}

void BDG::addEHsForReuseNodes(ObjFile &oldObjFile, ObjFile &newObjFile) {
  const auto &oldCFIs = oldObjFile.getEhFrame()->getCFIs();
  for (const auto &cfi : oldCFIs) {
    const auto &fdes = cfi->getFDEs();
    for (const auto &fde : fdes) {
      const auto pcBeginRelaEntry = fde->getPCBeginRelaEntry();
      if (pcBeginRelaEntry == nullptr) {
        continue;
      }
      const auto symbolName = pcBeginRelaEntry->getSym()->getNameValue();
      const auto it = reuseNodeIdrMap.find(symbolName);
      if (it == reuseNodeIdrMap.end()) {
        continue;
      }
      const auto reuseNode = it->second;
      reuseNode->setOldCFI(cfi);
      reuseNode->setOldFDE(fde);
    }
  }

  const auto &newCFIs = newObjFile.getEhFrame()->getCFIs();
  for (const auto &cfi : newCFIs) {
    const auto &fdes = cfi->getFDEs();
    for (const auto &fde : fdes) {
      const auto pcBeginRelaEntry = fde->getPCBeginRelaEntry();
      if (pcBeginRelaEntry == nullptr) {
        continue;
      }
      const auto symbolName = pcBeginRelaEntry->getSym()->getNameValue();
      const auto it = reuseNodeIdrMap.find(symbolName);
      if (it == reuseNodeIdrMap.end()) {
        continue;
      }
      const auto reuseNode = it->second;
      reuseNode->setNewCFI(cfi);
      reuseNode->setNewFDE(fde);
    }
  }
}

// Example:
// .text._ZL4testv -> _ZL4testv
// .data._ZL1x -> _ZL1x
// .rodata..L__const._ZL4testv.a -> .L__const._ZL4testv.a
// Invalid: return "".
std::string BDG::extractSectionSymbolName(const std::string &name) {
  int dotCnt = 0;
  int dotIdx = -1;
  for (size_t i = 0; i < name.size(); i++) {
    if (name[i] != '.') {
      continue;
    }
    dotCnt += 1;
    if (dotCnt == 1) {
      continue;
    }
    if (dotCnt > 1) {
      dotIdx = i;
      break;
    }
  }
  if (dotIdx == -1) {
    return "";
  }
  return name.substr(dotIdx + 1);
}

void BDG::handleSymSecDep(const std::shared_ptr<ReuseNode> &reuseNode) {
  const auto oldSymbol = reuseNode->getOldSymbol();
  if (oldSymbol->getStType() == llvm::ELF::STT_SECTION) {
    const auto name = oldSymbol->getNameValue();
    const auto targetSymbolName = extractSectionSymbolName(name);
    if (targetSymbolName != "") {
      const auto it = reuseNodeIdrMap.find(targetSymbolName);
      if (it != reuseNodeIdrMap.end()) {
        const auto targetReuseNode = it->second;
        reuseNode->addDependencies(targetReuseNode);
        targetReuseNode->addDependencies(reuseNode);
      }
    }
  }
}

void BDG::handleRelaDep(const std::shared_ptr<ReuseNode> &reuseNode) {
  const auto &logger = Logger::getInstance();

  const auto oldRelaSection = reuseNode->getOldRelaSection();
  if (oldRelaSection == nullptr) {
    return;
  }

  const auto &relaEntries = oldRelaSection->getRelocations();
  for (const auto &relaEntry : relaEntries) {
    const auto symbol = relaEntry->getSym();
    logger.assertTrue(symbol != nullptr,
                      "BDG::handleRelaDep symbol should not be nullptr");

    const auto name = symbol->getNameValue();
    const auto it = reuseNodeIdrMap.find(name);
    if (it != reuseNodeIdrMap.end()) {
      reuseNode->addDependencies(it->second);
    }
  }
}

void BDG::handleFDEDep(const std::shared_ptr<CIE> &cie,
                       const std::vector<std::shared_ptr<FDE>> &fdes) {
  for (const auto &fde : fdes) {
    const auto pcBeginRelaEntry = fde->getPCBeginRelaEntry();
    if (pcBeginRelaEntry == nullptr) {
      continue;
    }

    const auto srcIt =
        reuseNodeIdrMap.find(pcBeginRelaEntry->getSym()->getNameValue());
    if (srcIt == reuseNodeIdrMap.end()) {
      continue;
    }
    const auto sourceReuseNode = srcIt->second;

    const auto &cieRelaEntries = cie->getRelaEntries();
    for (const auto &relaEntry : cieRelaEntries) {
      const auto targetIt =
          reuseNodeIdrMap.find(relaEntry->getSym()->getNameValue());
      if (targetIt != reuseNodeIdrMap.end()) {
        sourceReuseNode->addDependencies(targetIt->second);
      }
    }
    const auto &fdeRelaEntries = fde->getOtherRelaEntries();
    for (const auto &relaEntry : fdeRelaEntries) {
      const auto targetIt =
          reuseNodeIdrMap.find(relaEntry->getSym()->getNameValue());
      if (targetIt != reuseNodeIdrMap.end()) {
        sourceReuseNode->addDependencies(targetIt->second);
      }
    }
  }
}

void BDG::handleEhDep(ObjFile &oldObjFile) {
  const auto &cfis = oldObjFile.getEhFrame()->getCFIs();
  for (const auto &cfi : cfis) {
    const auto cie = cfi->getCIE();
    const auto &fdes = cfi->getFDEs();
    handleFDEDep(cie, fdes);
  }
}

void BDG::buildReuseNodesDependencies(ObjFile &oldObjFile) {
  for (const auto &p : reuseNodeIdrMap) {
    const auto reuseNode = p.second;

    // (1) sym <-> sym section
    handleSymSecDep(reuseNode);

    // (2) Extract dependencies from relocation entries.
    handleRelaDep(reuseNode);
  }

  // (3) Extract eh dependencies.
  handleEhDep(oldObjFile);
}

void BDG::build(ObjFile &oldObjFile, ObjFile &newObjFile) {
  // (1) Init old reuse version.
  loadOldReuseVersion(oldObjFile);

  // (2) Rename anonymouse symbol.
  renameAnonymousSymbol(oldObjFile, oldReuseVersion);
  renameAnonymousSymbol(newObjFile, newReuseVersion);

  // (3) Symbol mapping.
  const auto oldSymbolNameMap = symbolNameMapping(oldObjFile);
  const auto newSymbolNameMap = symbolNameMapping(newObjFile);

  // (4) Init reuse nodes with symbols.
  initReuseNodesWithSymbol(oldSymbolNameMap, newSymbolNameMap);

  // (5) Add associated sections for reuse nodes.
  // (5.1) Add text/data sections.
  addTDSectionsForReusedNodes(oldObjFile, newObjFile);
  // (5.2) Add rela sections.
  addRelaSectionsForReusedNodes(oldObjFile, newObjFile);

  // (6) Add CFI-CIE-FDEs for reuse nodes.
  addEHsForReuseNodes(oldObjFile, newObjFile);

  // (7) Build dependencies between reuse nodes.
  buildReuseNodesDependencies(oldObjFile);
}

void BDG::propagation(const std::unordered_set<std::string> &funcXSet) {
  std::queue<std::shared_ptr<ReuseNode>> que;
  for (const auto &funcMangledName : funcXSet) {
    const auto it = reuseNodeIdrMap.find(funcMangledName);
    if (it == reuseNodeIdrMap.end()) {
      continue;
    }
    que.push(it->second);
  }

  while (!que.empty()) {
    const auto curReuseNode = que.front();
    que.pop();

    // Cut.
    if (curReuseNode->getNewSymbol() != nullptr &&
        curReuseNode->getNewSection() != nullptr) {
      continue;
    }

    if (!funcVReuseNodes.emplace(curReuseNode->getIdr(), curReuseNode).second) {
      continue;
    }

    for (const auto &p : curReuseNode->getDependencies()) {
      const auto childReuseNode = p.second.lock();
      que.push(childReuseNode);
    }
  }
}

void BDG::dump(std::ostream &oss) const {
  for (const auto &p : reuseNodeIdrMap) {
    oss << "----------" << std::endl;
    const auto reuseNode = p.second;
    reuseNode->dump(oss);
  }
}

std::string BDG::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
