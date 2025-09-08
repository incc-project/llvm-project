#ifndef ILLVM_DEBUGLINESECTION_H
#define ILLVM_DEBUGLINESECTION_H

#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

class DebugLineSection final : public Section {
private:
  // Ref : 6.2.4
  struct LineTableHeader {
    uint32_t unit_length;
    uint16_t version;
    uint8_t address_size;
    uint8_t segment_selector_size;
    uint32_t header_length;
    uint8_t min_inst_length;
    uint8_t max_ops_per_inst;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    std::vector<uint8_t> standard_opcode_lengths;
    uint8_t dir_format_count;
    std::vector<std::pair<uint64_t, uint64_t>> dir_attrs;
    uint64_t directory_count;
    std::vector<FormValueRaw> directories;
    uint8_t file_name_entry_format_count;
    std::vector<std::pair<uint64_t, uint64_t>> file_attrs;
    uint64_t file_names_count;
    struct FileEntry {
      std::string name;
      uint64_t dir_index;
      std::array<uint8_t, 16> md5; // For DWARFv5
      FormValueRaw val;
    };
    std::vector<FileEntry> file_names;
  };

  struct Row {
    uint64_t address = 0;
    int32_t line = 1;
    uint32_t column = 0;
    uint32_t file = 1;
    uint32_t isa = 0;
    uint32_t discriminator = 0;
    uint32_t op_index = 0;
    bool is_stmt;
    bool basic_block = false;
    bool end_sequence = false;
    bool prologue_end = false;
    bool epilogue_begin = false;
  };

  LineTableHeader header;
  std::vector<Row> rows;

public:
  DebugLineSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data);

  void dumpData(std::ostream &oss) const override;

  void writeDataTo(char *buffer) override;

  FormValueRaw parseFormValue(
      uint64_t form, const uint8_t *&p, const uint8_t *end,
      const DebugStrOffsetsSection *strOffsets = nullptr,
      const DebugStrSection *strSection = nullptr,
      const DebugAddrSection *addrSection = nullptr, uint64_t dieOffset = 0,
      int strOffsetsTableIndex = -1, uint64_t addrBaseOffset = 0,
      std::optional<int64_t> implicitConst = std::nullopt);
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGLINESECTION_H
