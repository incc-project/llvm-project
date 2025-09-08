#include "illvm/FuncV/ELF/DebugLoclistsSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace illvm {
namespace funcv {
namespace elf {

DebugLoclistsSection::DebugLoclistsSection(
    const llvm::object::ELF64LE::Shdr *shdr, const char *_data)
    : Section(SectionType::DebugLoclists, shdr, _data) {
  const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
  const uint8_t *end = start + sh_size;

  header.length = llvm::support::endian::read32le(start);
  start += 4;
  header.version = llvm::support::endian::read16le(start);
  start += 2;
  header.addrSize = *start++;
  header.segSize = *start++;
  header.offsetEntryCount = llvm::support::endian::read32le(start);
  start += 4;

  for (uint32_t i = 0; i < header.offsetEntryCount; i++) {
    uint32_t offset = llvm::support::endian::read32le(start);
    start += 4;
    offsets.push_back(offset);
  }

  unsigned n;
  while (start < end) {
    uint64_t entryoff = start - reinterpret_cast<const uint8_t *>(data);
    uint8_t kind = *start++;

    LocEntry entry;
    entry.offset = entryoff;
    entry.kind = kind;
    if (kind == 0) { // DW_LLE_end_of_list
      entries.push_back(entry);
      continue;
    }
    switch (kind) {
    case 0x01: {
      uint64_t value = DebugConvert::decodeULEB128(start, &n);
      start += n;
      entry.values.push_back(value);
      break;
    }
    case 0x02:
    case 0x03:
    case 0x04: {
      uint64_t value0 = DebugConvert::decodeULEB128(start, &n);
      start += n;
      uint64_t value1 = DebugConvert::decodeULEB128(start, &n);
      start += n;
      entry.values = {value0, value1};
      break;
    }
    case 0x06: {
      uint64_t addr = (header.addrSize == 8)
                          ? llvm::support::endian::read64le(start)
                          : llvm::support::endian::read32le(start);
      start += header.addrSize;
      entry.values.push_back(addr);
      break;
    }
    case 0x07: {
      uint64_t addrStart = (header.addrSize == 8)
                               ? llvm::support::endian::read64le(start)
                               : llvm::support::endian::read32le(start);
      start += header.addrSize;
      uint64_t addrEnd = (header.addrSize == 8)
                             ? llvm::support::endian::read64le(start)
                             : llvm::support::endian::read32le(start);
      start += header.addrSize;
      entry.values = {addrStart, addrEnd};
      break;
    }
    case 0x08: {
      uint64_t addr = (header.addrSize == 8)
                          ? llvm::support::endian::read64le(start)
                          : llvm::support::endian::read32le(start);
      start += header.addrSize;
      uint64_t length = DebugConvert::decodeULEB128(start, &n);
      start += n;
      entry.values = {addr, length};
      break;
    }
    default: {
      assert(false && "Unhandled loclist entry kind");
      break;
    }
    }

    if (kind != 0x01 && kind != 0x06) {
      uint64_t exprLength = DebugConvert::decodeULEB128(start, &n);
      start += n;
      entry.expr.insert(entry.expr.end(), start, start + exprLength);
      start += exprLength;
    }
    entries.push_back(std::move(entry));
  }
}

void DebugLoclistsSection::writeDataTo(char *buffer) {
  std::vector<uint8_t> out;

  uint32_t len = (uint32_t)header.length;
  out.push_back(len & 0xff);
  out.push_back((len >> 8) & 0xff);
  out.push_back((len >> 16) & 0xff);
  out.push_back((len >> 24) & 0xff);

  out.push_back(header.version & 0xff);
  out.push_back((header.version >> 8) & 0xff);

  out.push_back(header.addrSize);
  out.push_back(header.segSize);

  uint32_t oc = header.offsetEntryCount;
  out.push_back(oc & 0xff);
  out.push_back((oc >> 8) & 0xff);
  out.push_back((oc >> 16) & 0xff);
  out.push_back((oc >> 24) & 0xff);

  for (auto off : offsets) {
    uint32_t val = (uint32_t)off;
    out.push_back(val & 0xff);
    out.push_back((val >> 8) & 0xff);
    out.push_back((val >> 16) & 0xff);
    out.push_back((val >> 24) & 0xff);
  }

  for (auto &e : entries) {
    out.push_back(e.kind);

    switch (e.kind) {
    case 0x01:
      DebugConvert::encodeULEB128(e.values[0], out);
      break;
    case 0x02:
    case 0x03:
    case 0x04:
      DebugConvert::encodeULEB128(e.values[0], out);
      DebugConvert::encodeULEB128(e.values[1], out);
      break;
    case 0x06: {
      uint64_t v = e.values[0];
      for (int i = 0; i < header.addrSize; i++)
        out.push_back((v >> (i * 8)) & 0xff);
      break;
    }
    case 0x07: {
      uint64_t v1 = e.values[0];
      uint64_t v2 = e.values[1];
      for (int i = 0; i < header.addrSize; i++)
        out.push_back((v1 >> (i * 8)) & 0xff);
      for (int i = 0; i < header.addrSize; i++)
        out.push_back((v2 >> (i * 8)) & 0xff);
      break;
    }
    case 0x08: { // start_length
      uint64_t v1 = e.values[0];
      for (int i = 0; i < header.addrSize; i++)
        out.push_back((v1 >> (i * 8)) & 0xff);
      DebugConvert::encodeULEB128(e.values[1], out);
      break;
    }
    default:
      break;
    }
    if (!e.expr.empty()) {
      DebugConvert::encodeULEB128(e.expr.size(), out); // 先写长度
      out.insert(out.end(), e.expr.begin(), e.expr.end());
    }
  }

  assert(out.size() <= sh_size &&
         "Rewritten .debug_aranges larger than original");
  memcpy(buffer, out.data(), out.size());
}

void DebugLoclistsSection::dumpData(std::ostream &oss) const {
  oss << ".debug_loclists contents:\n";
  oss << "locations list header: length = 0x"
      << DebugConvert::intToHex(header.length, 8) << ", format = DWARF32"
      << ", version = 0x" << DebugConvert::intToHex(header.version, 4)
      << ", addr_size = 0x" << DebugConvert::intToHex(header.addrSize, 2)
      << ", seg_size = 0x" << DebugConvert::intToHex(header.segSize, 2)
      << ", offset_entry_count = 0x"
      << DebugConvert::intToHex(header.offsetEntryCount, 8) << "\n";

  oss << "offsets: [\n";
  for (auto off : offsets)
    oss << "0x" << DebugConvert::intToHex(off, 8) << "\n";
  oss << "]\n";

  for (auto &e : entries) {
    if (e.kind == 0x00)
      continue;
    if (e.kind == 0x01 || e.kind == 0x06)
      oss << "0x" << DebugConvert::intToHex(e.offset, 8) << ": \n";
    oss << "            " << DebugTypeToString::getLLEName(e.kind) << "(0x";
    switch (e.kind) {
    case 0x01:
    case 0x06:
      oss << DebugConvert::intToHex(e.values[0], 16) << ")";
      break;
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x07:
      oss << DebugConvert::intToHex(e.values[0], 16) << ", 0x"
          << DebugConvert::intToHex(e.values[1], 16) << ")";
      break;
    case 0x08:
      oss << DebugConvert::intToHex(e.values[0], 16) << ", len=" << e.values[1]
          << ")";
      break;
    default:
      break;
    }
    if (!e.expr.empty()) {
      oss << ":";
      for (auto op : e.expr)
        oss << " 0x" << DebugConvert::intToHex(op, 1);
    }
    oss << "\n";
  }
}

} // namespace elf
} // namespace funcv
} // namespace illvm
