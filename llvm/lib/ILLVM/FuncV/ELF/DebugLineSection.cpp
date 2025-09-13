#include "illvm/FuncV/ELF/DebugLineSection.h"

#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Logger.h"

#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/Support/LEB128.h"

namespace illvm {
namespace funcv {
namespace elf {

DebugLineSection::DebugLineSection(const llvm::object::ELF64LE::Shdr *shdr,
                                   const char *_data)
    : Section(SectionType::DebugLine, shdr, _data) {
  const auto &logger = Logger::getInstance();

  auto buf = reinterpret_cast<const uint8_t *>(data);
  auto *start = buf;

  // header
  memcpy(&header.unit_length, buf, sizeof(uint32_t));
  buf += sizeof(uint32_t);
  const uint8_t *end = buf + header.unit_length;

  memcpy(&header.version, buf, sizeof(uint16_t));
  buf += sizeof(uint16_t);
  memcpy(&header.address_size, buf, sizeof(uint8_t));
  logger.assertTrue(header.address_size == sizeof(uint32_t),
                    "header.address_size != sizeof(uint32_t)!");
  buf += sizeof(uint8_t);
  memcpy(&header.segment_selector_size, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.header_length, buf, sizeof(uint32_t));
  buf += sizeof(uint32_t);

  memcpy(&header.min_inst_length, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.max_ops_per_inst, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.default_is_stmt, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.line_base, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.line_range, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);
  memcpy(&header.opcode_base, buf, sizeof(uint8_t));
  buf += sizeof(uint8_t);

  header.standard_opcode_lengths.resize(header.opcode_base - 1);
  for (uint8_t &len : header.standard_opcode_lengths) {
    memcpy(&len, buf, sizeof(uint8_t));
    buf += sizeof(uint8_t);
  }

  // --- DWARFv5 include_directories_table ---
  unsigned n;
  header.dir_format_count = DebugConvert::decodeULEB128(buf, &n);
  logger.assertTrue(header.dir_format_count == 1,
                      "header.dir_format_count != 1");
  buf += n;
  uint64_t stdContentCode = DebugConvert::decodeULEB128(buf, &n);
  logger.assertTrue(
      stdContentCode == static_cast<int>(DW_LNCT_path),
      "content_type != static_cast<int>(DebugStdContentCode::DW_LNCT_path)");
  buf += n;
  uint64_t formId = DebugConvert::decodeULEB128(buf, &n);
  buf += n;
  header.dir_attr = {stdContentCode, formId};

  header.directory_count = DebugConvert::decodeULEB128(buf, &n);
  buf += n;
  for (uint64_t i = 0; i < header.directory_count; ++i) {
    auto fv = FormValueFactory::createFormValue(header.dir_attr.second);
    buf += fv->read(reinterpret_cast<const char *>(buf));
    header.directories.push_back(fv);
  }

  header.file_name_entry_format_count = DebugConvert::decodeULEB128(buf, &n);
  buf += n;
  for (uint64_t i = 0; i < header.file_name_entry_format_count; ++i) {
    uint64_t content_type = DebugConvert::decodeULEB128(buf, &n);
    buf += n;
    uint64_t form = DebugConvert::decodeULEB128(buf, &n);
    buf += n;
    header.file_attrs.emplace_back(content_type, form);
  }

  header.file_names_count = DebugConvert::decodeULEB128(buf, &n);
  buf += n;
  for (uint64_t i = 0; i < header.file_names_count; ++i) {
    LineTableHeader::FileEntry entry;
    for (const auto &p : header.file_attrs) {
      stdContentCode = p.first;
      formId = p.second;
      auto fv = FormValueFactory::createFormValue(formId);
      fv->read(reinterpret_cast<const char *>(buf));
      entry.vals.push_back(fv);
      if (stdContentCode == DW_LNCT_path) {
        entry.name = fv->getStringValue();
      } else if (stdContentCode == DW_LNCT_directory_index) {
        entry.dir_index = fv->getUIntegerValue();
      } else if (stdContentCode == DW_LNCT_MD5) {
        entry.md5 = fv->getUIArrayValue();
      } else {
        logger.fatal("Unknown stdContentCode");
      }
    }
    header.file_names.push_back(entry);
  }

  logger.assertTrue(buf - start == header.header_length,
                    "Parse debug line header error");

  // line table program
  DebugLineNumberEntry state;
  state.is_stmt = header.default_is_stmt;

  uint8_t opcode = 0;
  while (buf < end) {
    memcpy(&opcode, buf, sizeof(uint8_t));
    buf += sizeof(uint8_t);

    // Sp
    if (opcode >= header.opcode_base) {
      uint8_t adj_opcode = opcode - header.opcode_base;
      uint64_t addr_inc =
          (adj_opcode / header.line_range) * header.min_inst_length;
      int64_t line_inc = header.line_base + (adj_opcode % header.line_range);
      state.address += addr_inc;
      state.line += line_inc;
      lineNumberEntries.push_back(state);
      state.basic_block = false;
      state.epilogue_begin = false;
      state.prologue_end = false;
      state.discriminator = 0;
      continue;
    }
    // Ext
    if (opcode == 0) {
      uint64_t len = DebugConvert::decodeULEB128(buf, &n);
      buf += n;
      const uint8_t *ext_end = buf + len;
      uint8_t extOpcode = 0;
      memcpy(&extOpcode, buf, sizeof(uint8_t));
      buf += sizeof(uint8_t);
      switch (extOpcode) {
      case DW_LNE_end_sequence:
        state.end_sequence = true;
        lineNumberEntries.push_back(state);
        state = {};
        state.line = 1;
        state.is_stmt = header.default_is_stmt;
        break;
      case DW_LNE_set_address:
        memcpy(&state.address, buf, header.address_size);
        buf += header.address_size;
        break;
      case DW_LNE_set_discriminator:
        uint64_t discrim = DebugConvert::decodeULEB128(buf, &n);
        buf += n;
        state.discriminator = discrim;
        break;
      }
      buf = ext_end;
      continue;
    }
    // Std
    switch (opcode) {
    case DW_LNS_copy:
      lineNumberEntries.push_back(state);
      state.basic_block = false;
      state.prologue_end = false;
      state.epilogue_begin = false;
      state.discriminator = 0;
      break;
    case DW_LNS_advance_pc:
      state.address +=
          DebugConvert::decodeULEB128(buf, &n) * header.min_inst_length;
      buf += n;
      break;
    case DW_LNS_advance_line:
      state.line += DebugConvert::decodeSLEB128(buf, &n);
      buf += n;
      break;
    case DW_LNS_set_file:
      state.file = DebugConvert::decodeULEB128(buf, &n);
      buf += n;
      break;
    case DW_LNS_set_column:
      state.column = DebugConvert::decodeULEB128(buf, &n);
      buf += n;
      break;
    case DW_LNS_negate_stmt:
      state.is_stmt = !state.is_stmt;
      break;
    case DW_LNS_set_basic_block:
      state.basic_block = true;
      break;
    case DW_LNS_const_add_pc: {
      uint8_t adjusted = 255 - header.opcode_base;
      uint64_t addr_inc =
          (adjusted / header.line_range) * header.min_inst_length;
      state.address += addr_inc;
      break;
    }
    case DW_LNS_fixed_advance_pc: {
      uint16_t inc = 0;
      memcpy(&inc, buf, sizeof(uint16_t));
      buf += sizeof(uint16_t);
      state.address += inc;
      break;
    }
    case DW_LNS_set_prologue_end:
      state.prologue_end = true;
      break;
    case DW_LNS_set_epilogue_begin:
      state.epilogue_begin = true;
      break;
    case DW_LNS_set_isa:
      state.isa = DebugConvert::decodeULEB128(buf, &n);
      buf += n;
      break;
    default:
      logger.fatal("Unknown line number program opcode");
      break;
    }
  }
}

void DebugLineSection::dumpData(std::ostream &oss) const {
  oss << ".debug_line contents:\n";
  oss << "debug_line[0x00000000]\n";
  oss << "Line table prologue:\n";
  oss << "    total_length: 0x" << std::hex << std::setw(8) << std::setfill('0')
      << header.unit_length << "\n";
  oss << "          format: DWARF32\n";
  oss << "         version: " << std::dec << header.version << "\n";
  oss << "    address_size: " << static_cast<int>(header.address_size) << "\n";
  oss << " seg_select_size: " << static_cast<int>(header.segment_selector_size)
      << "\n";
  oss << " prologue_length: 0x" << std::hex << std::setw(8) << std::setfill('0')
      << header.header_length << "\n";
  oss << " min_inst_length: " << std::dec
      << static_cast<int>(header.min_inst_length) << "\n";
  oss << "max_ops_per_inst: " << static_cast<int>(header.max_ops_per_inst)
      << "\n";
  oss << " default_is_stmt: " << static_cast<int>(header.default_is_stmt)
      << "\n";
  oss << "       line_base: " << static_cast<int>(header.line_base) << "\n";
  oss << "      line_range: " << static_cast<int>(header.line_range) << "\n";
  oss << "     opcode_base: " << static_cast<int>(header.opcode_base) << "\n";

  for (size_t i = 0; i < header.standard_opcode_lengths.size(); ++i)
    oss << "standard_opcode_lengths[DW_LNS_"
        << DebugTypeToString::opcodeName(i + 1)
        << "] = " << static_cast<int>(header.standard_opcode_lengths[i])
        << "\n";

  for (size_t i = 0; i < header.directories.size(); ++i)
    oss << "include_directories[" << std::setw(3) << i << "] = \"" << std::hex
        << header.directories[i]->toString() << "\"\n";

  for (size_t i = 0; i < header.file_names.size(); ++i) {
    const auto &f = header.file_names[i];
    oss << "file_names[" << std::setw(3) << i << "]:\n";
    oss << "           name: \"" << f.name << "\"\n";
    oss << "      dir_index: " << f.dir_index << "\n";
    if (header.file_names.size() <= 1)
      oss << "   md5_checksum: ";
    for (uint8_t b : f.md5)
      oss << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>(b);
    oss << std::dec << "\n";
  }

  oss << "\nAddress             Line    Column  File    ISA  Discriminator  "
         "OpIndex  Flags\n";
  oss << "------------------  ------  ------  ------  ---  -------------  "
         "-------  -------------\n";
  for (const auto &row : lineNumberEntries) {
    oss << "0x" << std::hex << std::setw(16) << std::setfill('0') << row.address
        << "  ";
    oss << std::dec << std::setw(6) << row.line << "  ";
    oss << std::setw(6) << row.column << "  ";
    oss << std::setw(6) << row.file << "  ";
    oss << std::setw(3) << row.isa << "  ";
    oss << std::setw(13) << row.discriminator << "  ";
    oss << std::setw(7) << row.op_index << "  ";
    std::string flags;
    if (row.is_stmt)
      flags += "is_stmt ";
    if (row.basic_block)
      flags += "basic_block ";
    if (row.prologue_end)
      flags += "prologue_end ";
    if (row.end_sequence)
      flags += "end_sequence ";
    if (row.epilogue_begin)
      flags += "epilogue_begin ";
    oss << flags << "\n";
  }
}

// TODO: char* -> uint8_t*
void DebugLineSection::writeDataTo(char *buffer) {
  const auto &logger = Logger::getInstance();

  // --- Header (version, address size, etc) ---
  memcpy(buffer, &header.unit_length, sizeof(uint32_t));
  buffer += sizeof(uint32_t);
  memcpy(buffer, &header.version, sizeof(uint16_t));
  buffer += sizeof(uint16_t);
  memcpy(buffer, &header.address_size, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.segment_selector_size, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.header_length, sizeof(uint32_t));
  buffer += sizeof(uint32_t);

  memcpy(buffer, &header.min_inst_length, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.max_ops_per_inst, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.default_is_stmt, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.line_base, sizeof(int8_t));
  buffer += sizeof(int8_t);
  memcpy(buffer, &header.line_range, sizeof(uint8_t));
  buffer += sizeof(uint8_t);
  memcpy(buffer, &header.opcode_base, sizeof(uint8_t));
  buffer += sizeof(uint8_t);

  memcpy(buffer, header.standard_opcode_lengths.data(), header.opcode_base - 1);
  buffer += header.opcode_base - 1;

  // attr_count
  buffer += DebugConvert::encodeULEB128(header.dir_format_count,
                              reinterpret_cast<uint8_t *>(buffer));
  // attr spec
  buffer += DebugConvert::encodeULEB128(header.dir_attr.first,
                                        reinterpret_cast<uint8_t *>(buffer));
  buffer += DebugConvert::encodeULEB128(header.dir_attr.second,
                                        reinterpret_cast<uint8_t *>(buffer));
  // directory count
  buffer += DebugConvert::encodeULEB128(header.directory_count,
                              reinterpret_cast<uint8_t *>(buffer));
  // entries
  for (const auto &dir : header.directories) {
    buffer += dir->write(buffer);
  }

  // attr_count
  buffer += DebugConvert::encodeULEB128(header.file_name_entry_format_count,
                                        reinterpret_cast<uint8_t *>(buffer));
  // attr spec
  for (const auto &pr : header.file_attrs) {
    buffer += DebugConvert::encodeULEB128(pr.first,
                                          reinterpret_cast<uint8_t *>(buffer));
    buffer += DebugConvert::encodeULEB128(pr.second,
                                          reinterpret_cast<uint8_t *>(buffer));
  }

  // file count
  buffer += DebugConvert::encodeULEB128(header.file_names_count,
                                        reinterpret_cast<uint8_t *>(buffer));
  // entries
  for (const auto &f : header.file_names) {
    for (const auto &fv : f.vals) {
      buffer += fv->write(buffer);
    }
  }

  // TODO, check header size.
  int tempType = 0;
  DebugLineNumberEntry prev;
  prev.is_stmt = header.default_is_stmt;
  prev.end_sequence = true;
  uint64_t maxSpecialAddrDelta = (255 - header.opcode_base) / header.line_range;
  for (auto row : lineNumberEntries) {
    // TODO, ref llvm
    int64_t addrDelta = row.address - prev.address;
    if (row.end_sequence) {
      if (addrDelta >= 0 &&
          static_cast<uint64_t>(addrDelta) == maxSpecialAddrDelta) {
        tempType = DW_LNS_const_add_pc;
        memcpy(buffer, &tempType, sizeof(uint8_t));
        buffer += sizeof(uint8_t);
      } else {
        tempType = DW_LNS_advance_pc;
        memcpy(buffer, &tempType, sizeof(uint8_t));
        buffer += sizeof(uint8_t);

        buffer +=
            DebugConvert::encodeULEB128(addrDelta / header.min_inst_length,
                                        reinterpret_cast<uint8_t *>(buffer));

        prev.address = row.address;
      }
      tempType = 0;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(
          sizeof(uint8_t), reinterpret_cast<uint8_t *>(buffer));

      tempType = DW_LNE_end_sequence;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      prev = {};
      prev.line = 1;
      prev.is_stmt = header.default_is_stmt;
      prev.end_sequence = true;
      continue;
    }

    int64_t lineDelta = row.line - prev.line;

    if (row.file != prev.file) {
      tempType = DW_LNS_set_file;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(row.file, reinterpret_cast<uint8_t *>(buffer));

      prev.file = row.file;
    }

    if (row.column != prev.column) {
      tempType = DW_LNS_set_column;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(row.column, reinterpret_cast<uint8_t *>(buffer));

      prev.column = row.column;
    }

    if (row.discriminator != prev.discriminator) {
      tempType = 0;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      unsigned size = llvm::getULEB128Size(row.discriminator);
      // len : 1 + size: ext opcode + row.discriminator
      buffer += DebugConvert::encodeULEB128(1 + size, reinterpret_cast<uint8_t *>(buffer));

      tempType = DW_LNE_set_discriminator;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(row.discriminator, reinterpret_cast<uint8_t *>(buffer));

      prev.discriminator = row.discriminator;
    }

    if (row.isa != prev.isa) {
      tempType = DW_LNS_set_isa;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(row.isa, reinterpret_cast<uint8_t *>(buffer));

      prev.isa = row.isa;
    }

    if (row.is_stmt != prev.is_stmt) {
      tempType = DW_LNS_negate_stmt;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      prev.is_stmt = row.is_stmt;
    }

    if (row.basic_block && !prev.basic_block) {
      tempType = DW_LNS_set_basic_block;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      prev.basic_block = true;
    }

    if (row.prologue_end && !prev.prologue_end) {
      tempType = DW_LNS_set_prologue_end;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      prev.prologue_end = true;
    }

    if (row.epilogue_begin && !prev.epilogue_begin) {
      tempType = DW_LNS_set_epilogue_begin;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      prev.epilogue_begin = true;
    }

    if (prev.end_sequence && !row.end_sequence) {
      prev.end_sequence = false;

      tempType = 0;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeULEB128(1 + header.address_size, reinterpret_cast<uint8_t *>(buffer));

      tempType = DW_LNE_set_address;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      memcpy(buffer, &row.address, header.address_size);
      buffer += header.address_size;

      prev.address = row.address;
    }

    int64_t tempSigned;
    uint64_t temp;
    uint64_t opcode;
    bool needCopy = false;

    tempSigned = lineDelta - header.line_base;

    if (tempSigned >= header.line_range ||
        tempSigned + header.opcode_base > 255 || tempSigned < 0) {
      tempType = DW_LNS_advance_line;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      buffer += DebugConvert::encodeSLEB128(lineDelta, reinterpret_cast<uint8_t *>(buffer));

      lineDelta = 0;
      prev.line = row.line;
      tempSigned = 0 - header.line_base;
      needCopy = true;
    }

    if (lineDelta == 0 && addrDelta == 0) {
      tempType = DW_LNS_copy;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);

      continue;
    }

    temp = static_cast<uint64_t>(tempSigned) + header.opcode_base;

    if (addrDelta >= 0 &&
        static_cast<uint64_t>(addrDelta) < 256 + maxSpecialAddrDelta) {

      opcode = temp + addrDelta * header.line_range;

      if (opcode <= 255) {
        memcpy(buffer, &opcode, sizeof(uint8_t));
        buffer += sizeof(uint8_t);

        prev.discriminator = 0;
        prev.address = row.address;
        prev.line = row.line;
        prev.basic_block = false;
        prev.prologue_end = false;
        prev.epilogue_begin = false;
        continue;
      }

      // Try using DW_LNS_const_add_pc followed by special op.
      opcode = temp + (addrDelta - maxSpecialAddrDelta) * header.line_range;
      if (opcode <= 255) {
        tempType = DW_LNS_const_add_pc;
        memcpy(buffer, &tempType, sizeof(uint8_t));
        buffer += sizeof(uint8_t);

        memcpy(buffer, &opcode, sizeof(uint8_t));
        buffer += sizeof(uint8_t);

        prev.discriminator = 0;
        prev.address = row.address;
        prev.line = row.line;
        prev.basic_block = false;
        prev.prologue_end = false;
        prev.epilogue_begin = false;

        continue;
      }
    }

    tempType = DW_LNS_advance_pc;
    memcpy(buffer, &tempType, sizeof(uint8_t));
    buffer += sizeof(uint8_t);

    buffer += DebugConvert::encodeULEB128(addrDelta / header.min_inst_length, reinterpret_cast<uint8_t *>(buffer));

    if (needCopy) {
      tempType = DW_LNS_copy;
      memcpy(buffer, &tempType, sizeof(uint8_t));
      buffer += sizeof(uint8_t);
    } else {
      logger.fatal("Buggy special opcode encoding.");
    }

    prev.discriminator = 0;
    prev.address = row.address;
    prev.line = row.line;
  }

  // TODO check sh_size.
}

} // namespace elf
} // namespace funcv
} // namespace illvm
