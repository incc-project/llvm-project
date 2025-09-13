#include "illvm/FuncV/ELF/DebugInfoSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Logger.h"

#include "llvm/IR/InlineAsm.h"

namespace illvm {
namespace funcv {
namespace elf {

// TODO: fatal -> error.

DebugInfoSection::DebugInfoSection(const llvm::object::ELF64LE::Shdr *shdr,
                                   const char *_data,
                                   FormValueBaseSections &baseSections)
    : Section(SectionType::DebugInfo, shdr, _data) {
  // Reference: "DWARF5", page 200.
  const auto &logger = Logger::getInstance();

  const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
  const uint8_t *p = start;

  // Read header.
  logger.assertTrue(sizeof(CompileUnitHeader) <= sh_size,
                    "Can not read compile unit table header");
  const auto *hdr = reinterpret_cast<const CompileUnitHeader *>(p);
  logger.assertTrue(hdr->unitLength == sh_size - sizeof(uint32_t),
                    "Invalid compile unit table size");
  header.unitLength = hdr->unitLength;
  header.version = hdr->version;
  header.unitType = hdr->unitType;
  header.addrSize = hdr->addrSize;
  header.abbrevOffset = hdr->abbrevOffset;

  // TODO: check: not dwo

  p += sizeof(CompileUnitHeader);

  DIE root = parseDIE(p, baseSections);

  // TODO check only 1 compile unit die.
}

DebugInfoSection::DIE
DebugInfoSection::parseDIE(const uint8_t *&p,
                           FormValueBaseSections &baseSections) {
  unsigned len = 0;
  const uint64_t abbrevCode = DebugConvert::decodeULEB128(p, &len);
  p += len;

  if (abbrevCode == 0) {
    return {};
  }

  const auto &abbrevSec = baseSections.debugAbbrevSection;
  assert(abbrevSec != nullptr);
  const auto &decl = abbrevSec->getAbbreviationDecl(abbrevCode);

  DIE die;
  die.abbrevCode = abbrevCode;
  die.abbrevDecl = &decl;

  for (const auto &af : decl.attrForms) {
    auto fv = FormValueFactory::createFormValue(af.form);
    p += fv->read(reinterpret_cast<const char *>(p));
    die.attributes.emplace_back(af.attr, fv);
  }

  if (decl.hasChildren) {
    while (true) {
      auto child = parseDIE(p, baseSections);
      if (child.abbrevDecl == nullptr) {
        break;
      }
      die.children.push_back(std::move(child));
    }
  }

  return die;
}

void DebugInfoSection::dumpData(std::ostream &oss) const {
  oss << ".debug_info content:\n";
  // dump Compile Unit
  const std::string unitTypeStr = DebugTypeToString::getUnitType(header.unitType);

  oss << "Compile Unit: ";
  oss << "length = 0x" << std::setw(8) << header.unitLength << ", ";
  oss << "format = DWARF32, ";
  oss << "version = 0x" << std::setw(4) << header.version << ", ";
  oss << "unit_type = " << unitTypeStr << ", ";
  oss << "abbr_offset = 0x" << std::setw(4) << header.abbrevOffset << ", ";
  oss << "addr_size = 0x" << std::setw(2) << static_cast<int>(header.addrSize)
      << " ";
  oss << "\n";
  dumpDIE(oss, root);
}

void DebugInfoSection::dumpDIE(std::ostream &oss, const DIE &die) {
  oss << DebugTypeToString::getTagName(die.abbrevDecl->tag) << "\n";

  for (const auto &pr : die.attributes) {
    const auto attr = pr.first;
    const auto val = pr.second;
    oss << DebugTypeToString::getAttrName(attr) << " " << val->toString()
        << "\n";
    // TODO dump implicit const.
  }

  for (const auto &child : die.children) {
    dumpDIE(oss, child);
  }
}

void DebugInfoSection::writeDataTo(char *buffer) {
  auto p = reinterpret_cast<uint8_t *>(buffer);

  auto *hdr = reinterpret_cast<CompileUnitHeader *>(buffer);
  hdr->unitLength = header.unitLength;
  hdr->version = header.version;
  hdr->unitType = header.unitType;
  hdr->addrSize = header.addrSize;
  hdr->abbrevOffset = header.abbrevOffset;

  p += sizeof(CompileUnitHeader);

  writeDIE(root, p);
}

void DebugInfoSection::writeDIE(const DIE &die, uint8_t *&out) {
  out += DebugConvert::encodeULEB128(die.abbrevCode, out);

  const auto *decl = die.abbrevDecl;
  if (!decl) {
    return;
  }

  for (size_t i = 0; i < decl->attrForms.size(); ++i) {
    const auto &val = die.attributes[i].second;
    out += val->write(reinterpret_cast<char *>(out));
  }

  if (decl->hasChildren) {
    for (const auto &child : die.children) {
      writeDIE(child, out);
    }
    // Null abbrev code for end of children
    const uint8_t zero = 0;
    memcpy(out, &zero, sizeof(uint8_t));
    out += sizeof(uint8_t);
  }
}

// TODO: mem buf utils.

} // namespace elf
} // namespace funcv
} // namespace illvm
