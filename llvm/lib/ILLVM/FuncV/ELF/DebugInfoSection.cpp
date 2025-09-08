#include "illvm/FuncV/ELF/DebugInfoSection.h"

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

DebugInfoSection::DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr,
                                   const char *_data,
                                   const DebugAbbrevSection &abbrevRef,
                                   const DebugStrSection *strRef,
                                   const DebugStrOffsetsSection *strOffsetRef,
                                   const DebugAddrSection *addrRef,
                                   const DebugRnglistSection *rnglistRef)
    : Section(SectionType::DebugInfo, shdr, _data), abbrev(abbrevRef),
      debugStr(strRef), debugStrOffset(strOffsetRef), debugAddr(addrRef),
      debugRnglist(rnglistRef) {
  const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
  const uint8_t *end = start + sh_size;
  const uint8_t *p = start;

  while (p + 12 <= end) {
    uint64_t offset = p - start;

    // Reference: "DWARF5", page 200.
    //      uint32_t unitLength = *reinterpret_cast<const uint32_t *>(p);
    uint32_t unitLength = llvm::support::endian::read32le(p);
    p += 4;
    uint16_t version = llvm::support::endian::read16le(p);
    p += 2;
    uint8_t unitType = *p++;
    uint8_t addrSize = *p++;
    uint32_t abbrevOffset = llvm::support::endian::read32le(p);
    p += 4;

    std::optional<uint64_t> dwoId;
    if (unitType == 0x04 || unitType == 0x05) {
      if (p + 8 <= end) {
        dwoId = llvm::support::endian::read64le(p);
        p += 8;
      }
    }

    cuHeaders.push_back(
        {offset, unitLength, version, unitType, addrSize, abbrevOffset, dwoId});

    const uint8_t *cuEnd = start + offset + 4 + unitLength;

    // parse root DIE early to extract str_offsets_base
    int strOffsetsTableIndex = -1;
    uint64_t addrBaseOffset = 0;
    const uint8_t *tmp = p;

    uint64_t dieOffset = tmp - start;
    unsigned len = 0;
    uint64_t abbrevCode = DebugConvert::decodeULEB128(tmp, &len, cuEnd);
    tmp += len;

    const auto *decl = abbrev.getAbbreviationDecl(abbrevOffset, abbrevCode);
    if (decl) {
      for (const auto &af : decl->attrForms) {
        if (af.attr == 0x72) {
          uint64_t val =
              parseFormValue(af.form, tmp, cuEnd, nullptr, nullptr, nullptr,
                             dieOffset, strOffsetsTableIndex, addrBaseOffset)
                  .value;
          assert(debugStrOffset != nullptr);
          strOffsetsTableIndex = debugStrOffset->getTableIndex(val);
        } else if (af.attr == 0x73) {
          addrBaseOffset =
              parseFormValue(af.form, tmp, cuEnd, nullptr, nullptr, nullptr,
                             dieOffset, strOffsetsTableIndex, addrBaseOffset)
                  .value;

        } else {
          DebugTemp::skipFormValue(af.form, tmp,
                                   cuEnd); // Skip uninteresting attributes
        }
      }
    }

    std::vector<DIE> topLevelDIEs;
    while (p < cuEnd) {
      DIE die = parseDIE(p, start, cuEnd, abbrevOffset, strOffsetsTableIndex,
                         addrBaseOffset);
      if (die.abbrevDecl == nullptr)
        break;
      topLevelDIEs.push_back(std::move(die));
    }
    cuDIEs.push_back(std::move(topLevelDIEs));
    p = start + offset + 4 + unitLength;
  }
}

DebugInfoSection::DIE
DebugInfoSection::parseDIE(const uint8_t *&p, const uint8_t *start,
                           const uint8_t *end, uint32_t abbrevOffset,
                           int strOffsetsTableIndex, uint64_t addrBaseOffset) {
  uint64_t offset = p - start;
  unsigned len = 0;
  uint64_t abbrevCode = DebugConvert::decodeULEB128(p, &len, end);
  p += len;

  if (abbrevCode == 0)
    return {};

  const auto *decl = abbrev.getAbbreviationDecl(abbrevOffset, abbrevCode);
  if (!decl)
    return {};

  DIE die;
  die.offset = offset;
  die.abbrevCode = abbrevCode;
  die.abbrevDecl = decl;

  for (const auto &af : decl->attrForms) {
    FormValueRaw valueRaw;
    if (af.form == 0x21)
      valueRaw = parseFormValue(af.form, p, end, debugStrOffset, debugStr,
                                debugAddr, die.offset, strOffsetsTableIndex,
                                addrBaseOffset, af.implicitConst.value());
    else
      valueRaw =
          parseFormValue(af.form, p, end, debugStrOffset, debugStr, debugAddr,
                         die.offset, strOffsetsTableIndex, addrBaseOffset);
    die.attributes.emplace_back(af.attr, valueRaw);

    if (af.attr == 0x55 /* DW_AT_ranges */) {
      uint32_t rnglistIndex = static_cast<uint32_t>(valueRaw.value);
      assert(debugRnglist != nullptr);
      debugRnglist->registerAddrBase(rnglistIndex, addrBaseOffset);
    }
  }

  if (decl->hasChildren) {
    while (true) {
      auto child = parseDIE(p, start, end, abbrevOffset, strOffsetsTableIndex);
      if (child.abbrevDecl == nullptr) // Empty DIE indicates end of children
        break;
      die.children.push_back(std::move(child));
    }
  }

  return die;
}

void DebugInfoSection::dumpData(std::ostream &oss) const {
  oss << ".debug_info contents:\n";
  // dump Compile Unit
  for (size_t i = 0; i < cuHeaders.size(); ++i) {
    const auto &cu = cuHeaders[i];
    const std::string unitTypeStr = DebugTypeToString::getUnitType(cu.unitType);

    oss << std::hex << std::setfill('0');
    oss << "0x" << std::setw(8) << cu.offset << ": Compile Unit: ";
    oss << "length = 0x" << std::setw(8) << cu.unitLength << ", ";
    oss << "format = DWARF32, ";
    oss << "version = 0x" << std::setw(4) << cu.version << ", ";
    oss << "unit_type = " << unitTypeStr << ", ";
    oss << "abbr_offset = 0x" << std::setw(4) << cu.abbrevOffset << ", ";
    oss << "addr_size = 0x" << std::setw(2) << static_cast<int>(cu.addrSize)
        << " ";
    if (cu.dwoId.has_value())
      oss << ", DWO_id = " << std::setw(12) << cu.dwoId.value() << " ";
    // NextUnitOffset = Offset + Length + LengthFieldByteSize
    oss << "(next unit at 0x" << std::setw(8) << (cu.offset + 4 + cu.unitLength)
        << ")\n";
    oss << "\n";
    for (const auto &die : cuDIEs[i]) {
      dumpDIE(oss, die);
    }
  }
}

void DebugInfoSection::dumpDIE(std::ostream &oss, const DIE &die) const {
  oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << die.offset
      << ": " << DebugTypeToString::getTagName(die.abbrevDecl->tag) << "\n";

  for (const auto &[attr, val] : die.attributes) {
    if (val.form == 0x08 || val.form == 0x25) {
      oss << std::string(14, ' ') << DebugTypeToString::getAttrName(attr)
          << "\t(\"" << val.str << "\")\n";
    } else if (val.form == 0x03 || val.form == 0x04 || val.form == 0x09 ||
               val.form == 0x0a || val.form == 0x18 || val.form == 0x1e) {
      oss << std::string(14, ' ') << DebugTypeToString::getAttrName(attr)
          << "\t[BLOCK DATA, size=" << val.blockData.size() << "]: ";

      for (uint8_t byte : val.blockData) {
        oss << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << " ";
      }
      oss << std::dec << "\n";
    } else {
      oss << std::string(14, ' ') << DebugTypeToString::getAttrName(attr)
          << "\t(" << val.value << ")\n";
    }
  }

  for (const auto &child : die.children)
    dumpDIE(oss, child);
}

void DebugInfoSection::writeDataTo(char *buffer) {
  std::vector<uint8_t> out;

  for (size_t i = 0; i < cuHeaders.size(); ++i) {
    const auto &cu = cuHeaders[i];
    size_t headerStart = out.size();

    // 1. Reserve space for unit_length (to be filled later)
    out.resize(out.size() + 4); // DWARF32 format

    // 2. Write header
    out.push_back(cu.version & 0xff);
    out.push_back((cu.version >> 8) & 0xff);
    out.push_back(cu.unitType);
    out.push_back(cu.addrSize);

    out.push_back(cu.abbrevOffset & 0xff);
    out.push_back((cu.abbrevOffset >> 8) & 0xff);
    out.push_back((cu.abbrevOffset >> 16) & 0xff);
    out.push_back((cu.abbrevOffset >> 24) & 0xff);

    if (cu.dwoId.has_value()) {
      uint64_t id = cu.dwoId.value();
      for (int j = 0; j < 8; ++j)
        out.push_back((id >> (j * 8)) & 0xff);
    }

    // 3. Write all top-level DIEs
    for (const auto &die : cuDIEs[i])
      writeDIE(die, out);

    // 4. Fill in unit_length
    uint32_t length = static_cast<uint32_t>(out.size() - headerStart - 4);
    out[headerStart + 0] = (length & 0xff);
    out[headerStart + 1] = (length >> 8) & 0xff;
    out[headerStart + 2] = (length >> 16) & 0xff;
    out[headerStart + 3] = (length >> 24) & 0xff;
  }

  // 5. Copy to target buffer
  assert(out.size() <= sh_size && "Rewritten .debug_info larger than original");
  memcpy(buffer, out.data(), out.size());
}

void DebugInfoSection::writeDIE(const DIE &die,
                                std::vector<uint8_t> &out) const {
  DebugConvert::encodeULEB128(die.abbrevCode, out);

  const auto *decl = die.abbrevDecl;
  if (!decl)
    return;

  for (size_t i = 0; i < decl->attrForms.size(); ++i) {
    const auto &val = die.attributes[i].second;
    FormValueRaw::writeFormValue(val, out);
  }

  if (decl->hasChildren) {
    for (const auto &child : die.children)
      writeDIE(child, out);

    out.push_back(0x00); // Null abbrev code for end of children
  }
}

FormValueRaw DebugInfoSection::parseFormValue(
    uint64_t form, const uint8_t *&p, const uint8_t *end,
    const DebugStrOffsetsSection *strOffsets, const DebugStrSection *strSection,
    const DebugAddrSection *addrSection, uint64_t dieOffset,
    int strOffsetsTableIndex, uint64_t addrBaseOffset,
    std::optional<int64_t> implicitConst) {
  FormValueRaw result;
  result.form = form;
  const uint8_t *start = p;

  switch (form) {
  case 0x01: { // DW_FORM_addr
    if (end - p < 8) {
      llvm::errs() << "Error: DW_FORM_addr: not enough bytes left in buffer\n";
      p = end;
      break;
    }
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x03: { // DW_FORM_block2
    uint16_t len = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x04: { // DW_FORM_block4
    uint32_t len = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x05: { // DW_FORM_data2
    result.value = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    break;
  }
  case 0x06: { // DW_FORM_data4
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x07: { // DW_FORM_data8
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x08: { // DW_FORM_string
    result.str = std::string(reinterpret_cast<const char *>(p));
    p += result.str.size() + 1;
    break;
  }
  case 0x09: { // DW_FORM_block
    unsigned size = 0;
    uint64_t len = DebugConvert::decodeULEB128(p, &size, end);
    p += size;
    std::ostringstream oss;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x0a: { // DW_FORM_block1
    uint8_t len = *p++;
    result.blockData.insert(result.blockData.end(), p, p + len);
    p += len;
    break;
  }
  case 0x0b: { // DW_FORM_data1
    result.value = *p++;
    break;
  }
  case 0x0c: { // DW_FORM_flag
    result.flag = (*p++) != 0;
    break;
  }
  case 0x0d: { // DW_FORM_sdata
    unsigned size = 0;
    result.value = DebugConvert::decodeSLEB128(p, &size, end);
    p += size;
    break;
  }
  case 0x0e: { // DW_FORM_strp
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    if (strSection) {
      result.str = strSection->getString(result.value);
    } else {
      result.str = "";
    }
    break;
  }
  case 0x0f: { // DW_FORM_udata
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x10: // DW_FORM_ref_addr
  case 0x1c: // DW_FORM_ref_sup4
  case 0x24: // DW_FORM_ref_sup8
  case 0x20:
  case 0x14: { // DW_FORM_ref_sig8
    result.value = *reinterpret_cast<const uint64_t *>(p);
    p += 8;
    break;
  }
  case 0x11: {
    result.value = *p++;
    break;
  }
  case 0x12: {
    result.value = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    break;
  }
  case 0x13: {
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x15: { // DW_FORM_ref_udata
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x16: { // DW_FORM_indirect
    unsigned len = 0;
    uint64_t actualForm = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    return parseFormValue(actualForm, p, end, strOffsets, strSection,
                          addrSection, dieOffset);
  }
  case 0x17: { // DW_FORM_sec_offset
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x18: { // DW_FORM_exprloc
    unsigned len = 0;
    uint64_t size = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    result.blockData.insert(result.blockData.end(), p, p + size);
    p += size;
    break;
  }
  case 0x19: {
    result.flag = true;
    break;
  }
  case 0x1b: {
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x1d:
  case 0x1f: { // DW_FORM_strp_sup
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    break;
  }
  case 0x1e: { // DW_FORM_data16
    result.blockData.insert(result.blockData.end(), p, p + 16);
    p += 16;
    break;
  }
  case 0x21: { // DW_FORM_implicit_const
    assert(implicitConst.has_value());
    if (implicitConst.has_value())
      result.value = implicitConst.value();
    else
      llvm::errs() << "DW_FORM_implicit_const missing value at DIE offset 0x"
                   << DebugConvert::intToHex(dieOffset, 8) << "\n";
    break;
  }
  case 0x22:
  case 0x23: { // DW_FORM_rnglistx
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  case 0x25: { // DW_FORM_strx1
    result.value = *p++;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str =
          strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  }

  case 0x1a: { // DW_FORM_strx
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str =
          strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  }
  case 0x26: // strx2
    result.value = *reinterpret_cast<const uint16_t *>(p);
    p += 2;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str =
          strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  case 0x27: // strx3
    result.value = (*reinterpret_cast<const uint32_t *>(p)) & 0xFFFFFFu;
    p += 3;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str =
          strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  case 0x28: // strx4
    result.value = *reinterpret_cast<const uint32_t *>(p);
    p += 4;
    if (strOffsets && strSection && strOffsetsTableIndex >= 0) {
      result.str =
          strOffsets->getStringFromStrx(strOffsetsTableIndex, result.value);
    }
    break;
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2c: { // DW_FORM_addrx[1-4]
    unsigned len = 0;
    result.value = DebugConvert::decodeULEB128(p, &len, end);
    p += len;
    break;
  }
  default:
    llvm::errs() << "Unsupported form 0x" + DebugConvert::intToHex(form, 2);
  }
  result.rawBytes.assign(start, p);
  return result;
}

} // namespace elf
} // namespace funcv
} // namespace illvm
