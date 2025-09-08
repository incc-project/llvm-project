#ifndef ILLVM_DEBUGBASE_H
#define ILLVM_DEBUGBASE_H

#include <string>
#include <vector>

#include "llvm/Support/Endian.h"

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
  static uint64_t decodeULEB128(const uint8_t *p, unsigned *n = nullptr,
                                const uint8_t *end = nullptr,
                                const char **error = nullptr);

  static int64_t decodeSLEB128(const uint8_t *p, unsigned *n = nullptr,
                               const uint8_t *end = nullptr,
                               const char **error = nullptr);

  static unsigned encodeULEB128(uint64_t Value, uint8_t *p, unsigned PadTo = 0);

  static unsigned encodeSLEB128(int64_t Value, uint8_t *p, unsigned PadTo = 0);

  static void encodeULEB128(uint64_t Value, std::vector<uint8_t> &out,
                            unsigned PadTo = 0);

  static void encodeSLEB128(int64_t Value, std::vector<uint8_t> &out,
                            unsigned PadTo = 0);
};

class DebugTemp {
public:
  static std::string skipFormValue(uint64_t form, const uint8_t *&p,
                                   const uint8_t *end, uint64_t dieOffset = 0);
};

class FormValueRaw {
public:
  uint64_t form;
  std::vector<uint8_t> rawBytes;  // 原始字节
  uint64_t value = 0;             // 用于常数、引用、偏移
  std::string str;                // 用于 string、strp 解码后的值
  std::vector<uint8_t> blockData; // 用于 block、exprloc
  bool flag = false;              // 用于 DW_FORM_flag

  static void writeFormValue(const FormValueRaw &val,
                             std::vector<uint8_t> &out);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGBASE_H
