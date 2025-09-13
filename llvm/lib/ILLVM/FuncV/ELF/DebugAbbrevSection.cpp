#include "illvm/FuncV/ELF/DebugAbbrevSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Logger.h"

namespace illvm {
namespace funcv {
namespace elf {

DebugAbbrevSection::DebugAbbrevSection(const llvm::object::ELF64LE::Shdr *shdr,
                                       const char *_data)
    : Section(SectionType::DebugAbbrev, shdr, _data) {
  const auto &logger = Logger::getInstance();

  // Ref: 7.5.3
  const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
  const uint8_t *end = start + sh_size;
  const uint8_t *p = start;

  const AbbreviationDecl zeroAbbrevDecl;
  unsigned len;

  // Padding 0.
  abbrevTable.push_back(zeroAbbrevDecl);

  while (p < end) {
    AbbreviationDecl decl;

    decl.code = DebugConvert::decodeULEB128(p, &len, end);
    p += len;

    if (decl.code == 0) {
      break;
    }

    decl.tag = DebugConvert::decodeULEB128(p, &len, end);
    p += len;

    memcpy(&decl.hasChildren, p, sizeof(uint8_t));
    p += sizeof(uint8_t);

    while (p < end) {
      const uint64_t attr = DebugConvert::decodeULEB128(p, &len, end);
      p += len;
      const uint64_t form = DebugConvert::decodeULEB128(p, &len, end);
      p += len;
      if (attr == 0 && form == 0) {
        break;
      }

      AttributeForm af = {attr, form};
      if (form == 0x21 /* DW_FORM_implicit_const */) {
        // TODO. enum form type.
        af.implicitConst = DebugConvert::decodeSLEB128(p, &len, end);
        p += len;
      }

      decl.attrForms.push_back(af);
    }

    logger.assertTrue(abbrevTable.size() == decl.code,
                      "abbrevTable.size() != decl.code");

    abbrevTable.push_back(decl);
  }
}

void DebugAbbrevSection::dumpData(std::ostream &oss) const {
  oss << ".debug_abbrev contents:\n";
  for (size_t i = 1; i < abbrevTable.size(); ++i) {
    auto &decl = abbrevTable[i];
    oss << std::dec << decl.code << ". "
        << DebugTypeToString::getTagName(decl.tag) << "\t"
        << (decl.hasChildren ? "DW_CHILDREN_yes" : "DW_CHILDREN_no") << "\n";

    for (const auto &af : decl.attrForms) {
      oss << "\t" << DebugTypeToString::getAttrName(af.attr) << "\t"
          << DebugTypeToString::getFormName(af.form);
      if (af.form == 0x21) {
        oss << " " << af.implicitConst;
      }
      oss << "\n";
    }
    oss << "\n";
  }
}

void DebugAbbrevSection::writeDataTo(char *buffer) {
  auto out = reinterpret_cast<uint8_t *>(buffer);

  for (size_t i = 1; i < abbrevTable.size(); ++i) {
    auto &decl = abbrevTable[i];
    out += DebugConvert::encodeULEB128(decl.code, out);
    out += DebugConvert::encodeULEB128(decl.tag, out);
    memcpy(out, &decl.hasChildren, sizeof(uint8_t));
    out += sizeof(uint8_t);

    for (const auto &af : decl.attrForms) {
      out += DebugConvert::encodeULEB128(af.attr, out);
      out += DebugConvert::encodeULEB128(af.form, out);
      if (af.form == 0x21) {
        out += DebugConvert::encodeSLEB128(af.implicitConst, out);
      }
    }

    // Write attribute-form terminator (0, 0)
    out += DebugConvert::encodeULEB128(0, out);
    out += DebugConvert::encodeULEB128(0, out);
  }
  out += DebugConvert::encodeULEB128(0, out);
}

// Get abbreviation declaration by offset and code
const DebugAbbrevSection::AbbreviationDecl &
DebugAbbrevSection::getAbbreviationDecl(const uint64_t code) const {
  // TODO: check out-of-buffer
  return abbrevTable[code];
}

} // namespace elf
} // namespace funcv
} // namespace illvm
