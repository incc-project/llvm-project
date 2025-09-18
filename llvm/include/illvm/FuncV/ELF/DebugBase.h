#ifndef ILLVM_DEBUGBASE_H
#define ILLVM_DEBUGBASE_H

#include <memory>
#include <string>
#include <vector>

#include "illvm/FuncV/ELF/Reference.h"
#include "illvm/FuncV/ELF/Relocation.h"

#include "llvm/Support/LEB128.h"
#include "llvm/Support/Endian.h"

// TODO: check: only support 1 table.

namespace illvm {
namespace funcv {
namespace elf {

class DebugTypeToString {
public:
  static std::string getUnitType(uint8_t unitType);

  // Reference: "DWARF5", page 204.
  static std::string getTagName(uint64_t tag);

  // Reference: "DWARF5", page 207.
  static std::string getAttrName(uint64_t attr);

  // Reference: "DWARF5", page 220.
  static std::string getFormName(uint64_t form);

  static std::string getLLEName(uint8_t kind);

  static std::string opcodeName(uint8_t opcode);
};

class DebugConvert {
public:
  static std::string intToHex(uint64_t val, int width);

  // Ref: llvm/include/llvm/Support/LEB128.h
  static uint64_t decodeULEB128(const uint8_t *p, unsigned &len);

  static int64_t decodeSLEB128(const uint8_t *p, unsigned &len);

  static unsigned encodeULEB128(uint64_t val, uint8_t *p);

  static unsigned encodeSLEB128(int64_t val, uint8_t *p);
};

// Structure representing a string entry in .debug_str section
class DebugStrRef {
public:
  std::shared_ptr<IdxRef> offset; // Offset within the section
  std::string str; // The string content

  DebugStrRef(const std::shared_ptr<IdxRef> &_offset, const std::string &_str)
      : offset(_offset), str(_str) {}
};

// Structure representing an entry in .debug_str_offsets section
class DebugStrOffsetRef {
public:
  std::shared_ptr<IdxRef> idx; // entry idx
  std::shared_ptr<DebugStrRef> strRef;

  DebugStrOffsetRef(const std::shared_ptr<IdxRef> &_idx,
                    const std::shared_ptr<DebugStrRef> &_strRef)
      : idx(_idx), strRef(_strRef) {}
};

class DebugAddrRef {
public:
  std::shared_ptr<IdxRef> idx;
  std::shared_ptr<Relocation> relaEntry;

  DebugAddrRef(const std::shared_ptr<IdxRef> &_idx,
               const std::shared_ptr<Relocation> &_relaEntry)
      : idx(_idx), relaEntry(_relaEntry) {}
};

struct DebugLineNumberEntry {
  uint64_t address = 0;
  int32_t line = 1;
  uint32_t column = 0;
  uint32_t file = 1;
  uint32_t isa = 0;
  uint32_t discriminator = 0;
  uint32_t op_index = 0;
  bool is_stmt = false;
  bool basic_block = false;
  bool end_sequence = false;
  bool prologue_end = false;
  bool epilogue_begin = false;
};

// vector blocks: relocation, offset, vector<Row>
class DebugLineNumberBlock {
public:
  uint64_t offset;
  std::shared_ptr<Relocation> relaEntry;
  std::vector<DebugLineNumberEntry> lineNumberEntries;
};

enum DebugLineStdContentCode : int {
  DW_LNCT_path = 1,
  DW_LNCT_directory_index = 2,
  DW_LNCT_timestamp = 3,
  DW_LNCT_size = 4,
  DW_LNCT_MD5 = 5,
};

enum DebugLineStdOpCode : int {
  DW_LNS_copy = 1,
  DW_LNS_advance_pc = 2,
  DW_LNS_advance_line = 3,
  DW_LNS_set_file = 4,
  DW_LNS_set_column = 5,
  DW_LNS_negate_stmt = 6,
  DW_LNS_set_basic_block = 7,
  DW_LNS_const_add_pc = 8,
  DW_LNS_fixed_advance_pc = 9,
  DW_LNS_set_prologue_end = 10,
  DW_LNS_set_epilogue_begin = 11,
  DW_LNS_set_isa = 12
};

enum DebugLineExtOpCode : int {
  DW_LNE_end_sequence = 1,
  DW_LNE_set_address = 2,
  DW_LNE_set_discriminator = 4
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGBASE_H
