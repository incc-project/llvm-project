#include "illvm/FuncV/ELF/ReuseNode.h"

#include <iomanip>
#include <sstream>

namespace illvm {
namespace funcv {
namespace elf {

void ReuseNode::dumpSymbol(const std::string &prefix, std::ostream &oss,
                           const std::shared_ptr<Symbol> &symbol) {
  oss << prefix << " ";
  if (symbol == nullptr) {
    oss << "NULL" << std::endl;
  } else {
    symbol->dump(oss);
    oss << std::endl;
  }
}

void ReuseNode::dumpSectionHeader(const std::string &prefix, std::ostream &oss,
                                  const std::shared_ptr<Section> &section) {
  oss << prefix << " ";
  if (section == nullptr) {
    oss << "NULL" << std::endl;
  } else {
    section->dumpHeader(oss);
    oss << std::endl;
  }
}

void ReuseNode::dumpSection(const std::string &prefix, std::ostream &oss,
                            const std::shared_ptr<Section> &section) {
  oss << prefix << " ";
  if (section == nullptr) {
    oss << "NULL" << std::endl;
  } else {
    section->dumpHeader(oss);
    oss << std::endl;
    section->dumpData(oss);
    oss << std::endl;
  }
}

void ReuseNode::dumpCFI(const std::string &prefix, std::ostream &oss,
                        const std::shared_ptr<CFI> &cfi) {
  oss << prefix << " ";
  if (cfi == nullptr) {
    oss << "NULL" << std::endl;
  } else {
    cfi->getCIE()->dump(oss);
    oss << std::endl;
  }
}

void ReuseNode::dumpFDE(const std::string &prefix, std::ostream &oss,
                        const std::shared_ptr<FDE> &fde) {
  oss << prefix << " ";
  if (fde == nullptr) {
    oss << "NULL" << std::endl;
  } else {
    fde->dump(oss);
    oss << std::endl;
  }
}

void ReuseNode::dump(std::ostream &oss) const {
  oss << "[idr] " << getIdr() << std::endl;

  dumpSymbol("[oldSymbol]", oss, oldSymbol);
  dumpSymbol("[newSymbol]", oss, newSymbol);

  dumpSectionHeader("[oldSection]", oss, oldSection);
  dumpSectionHeader("[newSection]", oss, newSection);

  dumpSection("[oldRelaSection]", oss, oldRelaSection);
  dumpSectionHeader("[newRelaSection]", oss, newRelaSection);

  dumpCFI("[oldCFI-CIE]", oss, oldCFI);
  dumpCFI("[newCFI-CIE]", oss, newCFI);

  dumpFDE("[oldFDE]", oss, oldFDE);
  dumpFDE("[newFDE]", oss, newFDE);

  oss << "[dependencies]";
  for (const auto &dep : dependencies) {
    oss << " " << dep.second.lock()->getIdr();
  }
  oss << std::endl;
}

std::string ReuseNode::toString() const {
  std::stringstream oss;
  dump(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
