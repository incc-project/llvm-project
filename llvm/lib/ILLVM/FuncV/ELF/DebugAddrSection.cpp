#include "illvm/FuncV/ELF/DebugAddrSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

DebugAddrSection::DebugAddrSection(const llvm::object::ELF64LE::Shdr *shdr,
                                   const char *_data, llvm::Error &err)
    : Section(SectionType::DebugAddr, shdr, _data) {
  // Ref 7.27
  // Read header.
  ILLVM_FCHECK(sizeof(AddrTableHeader) <= sh_size, "");
  const auto *hdr = reinterpret_cast<const AddrTableHeader *>(data);
  ILLVM_FCHECK(hdr->unitLength == sh_size - sizeof(uint32_t), "");
  header.unitLength = hdr->unitLength;
  header.version = hdr->version;
  header.addrSize = hdr->addrSize;
  header.segSize = hdr->segSize;

  ILLVM_FCHECK((sh_size - sizeof(AddrTableHeader)) % header.addrSize == 0, "");

  for (uint64_t i = sizeof(AddrTableHeader), idx = 0; i < sh_size;
       i += header.addrSize, idx += 1) {
    addresses.emplace_back(
        std::make_shared<DebugAddrRef>(std::make_shared<IdxRef>(idx), nullptr));
  }
}

llvm::Error DebugAddrSection::parseReferences(
    const std::shared_ptr<RelocationSection> &relaSection) const {
  std::unordered_map<uint64_t, size_t> addressMap;
  uint64_t offset = sizeof(AddrTableHeader);
  for (size_t i = 0; i < addresses.size(); i += 1) {
    addressMap[offset] = i;
    offset += header.addrSize;
  }

  const auto &relaEntries = relaSection->getRelocations();
  for (const auto &relaEntry : relaEntries) {
    auto rOffset = relaEntry->getROffset();
    const auto it = addressMap.find(rOffset);
    ILLVM_ECHECK(it != addressMap.end(), "");
    addresses[it->second]->relaEntry = relaEntry;
  }
  return llvm::Error::success();
}

void DebugAddrSection::layout() {
  sh_size = sizeof(AddrTableHeader) + header.addrSize * addresses.size();
  header.unitLength = sh_size - sizeof(uint32_t);
  size_t i = 0;
  uint64_t offset = sizeof(AddrTableHeader);
  for (const auto &elem : addresses) {
    elem->idx->setValue(i);
    elem->relaEntry->getOffset()->setValue(offset);
    i += 1;
    offset += header.addrSize;
  }
}

void DebugAddrSection::writeDataTo(char *buffer) {
  uint64_t bufOff = 0;

  auto *hdr = reinterpret_cast<AddrTableHeader *>(buffer);
  hdr->unitLength = header.unitLength;
  hdr->version = header.version;
  hdr->addrSize = header.addrSize;
  hdr->segSize = header.segSize;

  bufOff += sizeof(AddrTableHeader);

  for (size_t i = 0; i < addresses.size(); i += 1) {
    uint64_t zero = 0;
    memcpy(buffer + bufOff, &zero, header.addrSize);
    bufOff += header.addrSize;
  }
}

void DebugAddrSection::dumpData(std::ostream &oss) const {
  oss << ".debug_addr contents:\n";

  oss << "Address table header: length = 0x" << std::hex << std::setw(8)
      << std::setfill('0') << header.unitLength
      << ", format = DWARF32, version = 0x" << std::setw(4) << header.version
      << ", addr_size = 0x" << std::setw(2) << static_cast<int>(header.addrSize)
      << ", seg_size = 0x" << std::setw(2) << static_cast<int>(header.segSize)
      << std::dec << "\n";

  oss << "Addrs: [\n";
  for (size_t i = 0; i < addresses.size(); i += 1) {
    if (header.addrSize == 4) {
      oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << 0
          << std::dec << "\n";
    } else if (header.addrSize == 8) {
      oss << "0x" << std::hex << std::setw(16) << std::setfill('0') << 0
          << std::dec << "\n";
    } else {
      oss << "# unsupported addr_size: " << static_cast<int>(header.addrSize)
          << "\n";
    }
  }
  oss << "]\n";
}

} // namespace elf
} // namespace funcv
} // namespace illvm
