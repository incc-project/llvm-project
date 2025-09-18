#ifndef ILLVM_DEBUGLINESECTION_H
#define ILLVM_DEBUGLINESECTION_H

#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"
#include "illvm/FuncV/ELF/FormValue.h"
#include "illvm/FuncV/ELF/Section.h"

namespace illvm {
namespace funcv {
namespace elf {

// TODO: add comment: update while writing.

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
    uint8_t dir_format_count; // should always 1.
    std::pair<uint64_t, uint64_t> dir_attr;
    uint64_t directory_count;
    std::vector<std::shared_ptr<FormValue>> directories;
    uint8_t file_name_entry_format_count;
    std::vector<std::pair<uint64_t, uint64_t>> file_attrs;
    uint64_t file_names_count;
    struct FileEntry {
      std::string name;
      uint64_t dir_index;
      std::vector<uint8_t> md5; // For DWARFv5
      std::vector<std::shared_ptr<FormValue>> vals;
    };
    std::vector<FileEntry> file_names;
  };

  LineTableHeader header;
  // immutable
  std::vector<DebugLineNumberEntry> lineNumberEntries;
  std::vector<DebugLineNumberBlock> lineNumberBlocks;

  DebugLineSection(const llvm::object::ELF64LE::Shdr *shdr, const char *_data,
                   llvm::Error &err);

public:
  static llvm::Expected<std::shared_ptr<DebugLineSection>>
  Create(const llvm::object::ELF64LE::Shdr *shdr, const char *_data) {
    llvm::Error err = llvm::Error::success();
    auto section = std::shared_ptr<DebugLineSection>(
        new DebugLineSection(shdr, _data, err));
    if (err) {
      return std::move(err);
    }
    return section;
  }

  // // TODO: call in ObjFile.
  // void parseReferences(const std::shared_ptr<RelocationSection> &sec) const;
  //
  // void layout() override;
  //
  // void fini() override;

  void writeDataTo(char *buffer) override;

  void dumpData(std::ostream &oss) const override;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_DEBUGLINESECTION_H
