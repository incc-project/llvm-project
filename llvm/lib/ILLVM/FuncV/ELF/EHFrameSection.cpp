#include "illvm/FuncV/ELF/EHFrameSection.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Logger.h"

namespace illvm {
namespace funcv {
namespace elf {

bool EhFrameSection::loadEntry(uint64_t &offset, uint32_t &length,
                               uint64_t &extLength, uint32_t &entryFlag,
                               const char *&otherData) const {
  const auto &logger = Logger::getInstance();

  uint64_t actLength = 0;
  length = 0;
  extLength = 0;
  if (offset + sizeof(uint32_t) > sh_size) {
    logger.fatal("can not load CIE/FDE length.");
  }
  memcpy(&length, data + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  if (length == 0) {
    return false;
  }

  if (length == 0xffffffff) {
    if (offset + sizeof(uint64_t) > sh_size) {
      logger.fatal("can not load CIE/FDE ext length.");
    }
    memcpy(&extLength, data + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    actLength = extLength;
  } else {
    actLength = length;
  }

  if (offset + sizeof(uint32_t) > sh_size) {
    logger.fatal("can not load CIE/FDE flag.");
  }
  memcpy(&entryFlag, data + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  actLength -= sizeof(uint32_t);

  if (offset + actLength > sh_size) {
    logger.fatal("CIE/FDE data leak.");
  }
  otherData = data + offset;
  offset += actLength;

  return true;
}

void EhFrameSection::linkRelaToCIEFDE(
    const std::vector<RelaPair> &rOffsetToRela) const {
  size_t rOffsetRelaIdx = 0;
  uint64_t offset = 0;

  auto handleCIE = [&rOffsetToRela, &rOffsetRelaIdx,
                    &offset](const std::shared_ptr<CIE> &cie) {
    while (rOffsetRelaIdx < rOffsetToRela.size() &&
           offset <= rOffsetToRela[rOffsetRelaIdx].first &&
           rOffsetToRela[rOffsetRelaIdx].first < offset + cie->getSize()) {
      cie->addRelaEntry(rOffsetToRela[rOffsetRelaIdx].second);
      rOffsetRelaIdx += 1;
    }
  };

  auto handleFDE = [&rOffsetToRela, &rOffsetRelaIdx,
                    &offset](const std::shared_ptr<FDE> &fde) {
    while (rOffsetRelaIdx < rOffsetToRela.size() &&
           offset <= rOffsetToRela[rOffsetRelaIdx].first &&
           rOffsetToRela[rOffsetRelaIdx].first < offset + fde->getSize()) {
      if (rOffsetToRela[rOffsetRelaIdx].first ==
          offset + fde->getPCBeginPreFilling()) {
        // Handle PCBegin rela.
        fde->setPCBeginRelaEntry(rOffsetToRela[rOffsetRelaIdx].second);
      } else {
        // Handle other rela.
        fde->addOtherRelaEntry(rOffsetToRela[rOffsetRelaIdx].second);
      }
      rOffsetRelaIdx += 1;
    }
  };

  for (const auto &cfi : cfis) {
    const auto cie = cfi->getCIE();
    handleCIE(cie);
    offset += cie->getSize();
    const auto &fdes = cfi->getFDEs();
    for (const auto &fde : fdes) {
      handleFDE(fde);
      offset += fde->getSize();
    }
  }
}

void EhFrameSection::adjustRelativeROffset() const {
  uint64_t offset = 0;

  for (const auto &cfi : cfis) {
    const auto cie = cfi->getCIE();

    const auto &cieRelaEntries = cie->getRelaEntries();
    for (const auto &relaEntry : cieRelaEntries) {
      relaEntry->getOffset()->setValue(relaEntry->getROffset() - offset);
    }

    offset += cie->getSize();

    const auto &fdes = cfi->getFDEs();
    for (const auto &fde : fdes) {

      const auto pcBeginRelaEntry = fde->getPCBeginRelaEntry();
      if (pcBeginRelaEntry != nullptr) {
        pcBeginRelaEntry->getOffset()->setValue(pcBeginRelaEntry->getROffset() -
                                                offset);
      }

      const auto &fdeRelaEntries = fde->getOtherRelaEntries();
      for (const auto &relaEntry : fdeRelaEntries) {
        relaEntry->getOffset()->setValue(relaEntry->getROffset() - offset);
      }

      offset += fde->getSize();
    }
  }
}

EhFrameSection::EhFrameSection(const llvm::object::ELF64LE::Shdr *shdr,
                               const char *_data)
    : Section(SectionType::EhFrame, shdr, _data) {
  const auto &logger = Logger::getInstance();
  // ref:
  // https://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html
  uint64_t offset = 0;
  uint32_t length = 0;
  uint64_t extLength = 0;
  uint32_t entryFlag = 0;
  const char *otherData = nullptr;

  // Load the first CIE.
  if (!loadEntry(offset, length, extLength, entryFlag, otherData)) {
    return;
  }
  if (entryFlag != 0) {
    logger.fatal("the first entry of CFI should be CIE.");
  }
  const auto firstCIE =
      std::make_shared<CIE>(length, extLength, entryFlag, otherData);
  auto curCFI = std::make_shared<CFI>(firstCIE);
  cfis.push_back(curCFI);

  while (offset < sh_size) {
    if (!loadEntry(offset, length, extLength, entryFlag, otherData)) {
      return;
    }
    if (entryFlag == 0) {
      // Load CIE.
      const auto cie =
          std::make_shared<CIE>(length, extLength, entryFlag, otherData);
      curCFI = std::make_shared<CFI>(cie);
      cfis.push_back(curCFI);
    } else {
      // Load FDE.
      const auto fde =
          std::make_shared<FDE>(length, extLength, entryFlag, otherData);
      curCFI->addFDE(fde);
    }
  }
}

void EhFrameSection::parseReferences(
    const std::shared_ptr<RelocationSection> &relaEhFrame) const {
  // r_offest -> rela entry.
  const auto &relaEntries = relaEhFrame->getRelocations();

  std::vector<RelaPair> rOffsetToRela;
  for (const auto &relaEntry : relaEntries) {
    rOffsetToRela.emplace_back(relaEntry->getROffset(), relaEntry);
  }

  std::sort(rOffsetToRela.begin(), rOffsetToRela.end(),
            [](const RelaPair &p1, const RelaPair &p2) {
              return p1.first < p2.first;
            });

  // Link rela to CIE/FDE.
  linkRelaToCIEFDE(rOffsetToRela);

  // Adjust r_offset relative to CIE/FDE.
  adjustRelativeROffset();
}

void EhFrameSection::layout() {
  sh_size = 0;
  uint64_t offset = 0;

  for (const auto &cfi : cfis) {
    const auto cie = cfi->getCIE();

    // Update rela.r_offset.
    // Note that this update should be completed before
    // RelocationSection::fini.
    const auto &cieRelaEntries = cie->getRelaEntries();
    for (const auto &relaEntry : cieRelaEntries) {
      const uint64_t newOffset = relaEntry->getOffsetValue() + offset;
      relaEntry->getOffset()->setValue(newOffset);
    }

    const uint64_t cieBaseOffset = offset;
    offset += cie->getSize();

    const auto &fdes = cfi->getFDEs();
    for (const auto &fde : fdes) {

      // Update CIE pointer.
      fde->setCIEPointer(offset - cieBaseOffset +
                         fde->getCIEPointerPreFilling());

      // Update rela.r_offset.
      // Note that this update should be completed before
      // RelocationSection::fini.
      const auto pcBeginRelaEntry = fde->getPCBeginRelaEntry();
      if (pcBeginRelaEntry != nullptr) {
        const uint64_t newOffset = pcBeginRelaEntry->getOffsetValue() + offset;
        pcBeginRelaEntry->getOffset()->setValue(newOffset);
      }

      const auto &fdeRelaEntries = fde->getOtherRelaEntries();
      for (const auto &relaEntry : fdeRelaEntries) {
        const uint64_t newOffset = relaEntry->getOffsetValue() + offset;
        relaEntry->getOffset()->setValue(newOffset);
      }

      offset += fde->getSize();
    }
  }

  sh_size = offset;
}

void EhFrameSection::writeDataTo(char *buffer) {
  uint64_t offset = 0;
  for (const auto &cfi : cfis) {
    const uint64_t inc = cfi->writeDataTo(buffer + offset);
    offset += inc;
  }
}

void EhFrameSection::dumpData(std::ostream &oss) const {
  for (size_t i = 0; i < cfis.size(); i++) {
    auto &cfi = cfis[i];
    oss << "CFI " << i << "==========" << std::endl;
    oss << cfi->toString();
  }
}

std::string EhFrameSection::dataToString() const {
  std::stringstream oss;
  dumpData(oss);
  return oss.str();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
