#include "illvm/FuncV/ELF/DebugAbbrevSection.h"

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

DebugAbbrevSection::DebugAbbrevSection(const llvm::object::ELF64LE::Shdr *shdr,
                                       const char *_data)
    : Section(SectionType::DebugAbbrev, shdr, _data) {
  // Ref: 7.5.3
  const uint8_t *start = reinterpret_cast<const uint8_t *>(data);
  const uint8_t *end = start + sh_size;
  const uint8_t *p = start;

  while (p < end) {
    uint64_t abbrevOffset = p - start;
    std::map<uint64_t, AbbreviationDecl> declMap;

    while (p < end) {
      unsigned bytesRead;
      uint64_t code = DebugConvert::decodeULEB128(p, &bytesRead, end);
      p += bytesRead;
      if (code == 0)
        break;

      uint64_t tag = DebugConvert::decodeULEB128(p, &bytesRead, end);
      p += bytesRead;
      uint8_t hasChildrenByte = *p++;

      AbbreviationDecl decl;
      decl.code = code;
      decl.tag = tag;
      decl.hasChildren = (hasChildrenByte == 1); // DW_CHILDREN_yes

      while (p < end) {
        uint64_t attr = DebugConvert::decodeULEB128(p, &bytesRead, end);
        p += bytesRead;
        uint64_t form = DebugConvert::decodeULEB128(p, &bytesRead, end);
        p += bytesRead;
        if (attr == 0 && form == 0)
          break;

        AttributeForm af = {attr, form};
        if (form == 0x21 /* DW_FORM_implicit_const */) {
          af.implicitConst = DebugConvert::decodeSLEB128(p, &bytesRead, end);
          p += bytesRead;
        }

        decl.attrForms.push_back(af);
      }

      declMap[code] = std::move(decl);
    }

    if (!declMap.empty())
      abbrevTables[abbrevOffset] = std::move(declMap);
  }
}

void DebugAbbrevSection::dumpData(std::ostream &oss) const {
  oss << ".debug_abbrev contents:\n";
  for (const auto &[offset, decls] : abbrevTables) {
    oss << "Abbrev table for offset: 0x" << std::setw(8) << std::setfill('0')
        << std::hex << offset << "\n";
    for (const auto &[code, decl] : decls) {
      oss << std::dec << code << ". " << DebugTypeToString::getTagName(decl.tag)
          << "\t" << (decl.hasChildren ? "DW_CHILDREN_yes" : "DW_CHILDREN_no")
          << "\n";

      for (const auto &af : decl.attrForms) {
        oss << "\t" << DebugTypeToString::getAttrName(af.attr) << "\t"
            << DebugTypeToString::getFormName(af.form);
        if (af.form == 0x21 && af.implicitConst.has_value()) {
          oss << " " << af.implicitConst.value();
        }
        oss << "\n";
      }
      oss << "\n";
    }
  }
}

void DebugAbbrevSection::writeDataTo(char *buffer) {
  std::vector<uint8_t> out;

  for (const auto &[offset, decls] : abbrevTables) {
    (void)offset;

    for (const auto &[code, decl] : decls) {
      DebugConvert::encodeULEB128(code, out);
      DebugConvert::encodeULEB128(decl.tag, out);
      out.push_back(decl.hasChildren ? 1 : 0);

      for (const auto &af : decl.attrForms) {
        DebugConvert::encodeULEB128(af.attr, out);
        DebugConvert::encodeULEB128(af.form, out);
        if (af.form == 0x21 && af.implicitConst.has_value()) {
          DebugConvert::encodeSLEB128(af.implicitConst.value(), out);
        }
      }

      // Write attribute-form terminator (0, 0)
      DebugConvert::encodeULEB128(0, out);
      DebugConvert::encodeULEB128(0, out);
    }
    DebugConvert::encodeULEB128(0, out);
  }

  // Copy to target buffer
  assert(out.size() <= sh_size &&
         "Rewritten .debug_abbrev exceeds original section size");
  memcpy(buffer, out.data(), out.size());
}

// Get abbreviation declaration by offset and code
const DebugAbbrevSection::AbbreviationDecl *
DebugAbbrevSection::getAbbreviationDecl(uint64_t abbrevOffset,
                                        uint64_t code) const {
  auto abbrevTableIt = abbrevTables.find(abbrevOffset);
  if (abbrevTableIt == abbrevTables.end())
    return nullptr;

  const auto &decls = abbrevTableIt->second;
  auto declIt = decls.find(code);
  if (declIt == decls.end())
    return nullptr;

  return &declIt->second;
}

} // namespace elf
} // namespace funcv
} // namespace illvm
