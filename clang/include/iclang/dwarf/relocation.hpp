#ifndef ICLANG_RELOCATION_HPP
#define ICLANG_RELOCATION_HPP

#include "iclang/dwarf/config.hpp"
#include "iclang/dwarf/reference.hpp"
#include "iclang/dwarf/symbol.hpp"

namespace iclang {

class Relocation {
private:
    uint64_t r_offset;
    uint64_t r_info; // Update while writing, according to sym.
    std::shared_ptr<Symbol> sym;
    int64_t r_addend;

public:
    Relocation(const uint64_t r_offset, const uint64_t r_info,
               const std::shared_ptr<Symbol> &sym, const int64_t r_addend)
        : r_offset(r_offset), r_info(r_info), sym(sym), r_addend(r_addend) {}

  Relocation(const llvm::object::ELF64LE::Rela *elf_rela)
      : Relocation(elf_rela->r_offset, elf_rela->r_info, nullptr,
                   elf_rela->r_addend) {}

  uint64_t getROffset() const { return r_offset; }

    void setROffset(const uint64_t _r_offset) { r_offset = _r_offset; }

    uint64_t getRInfo() const { return r_info; }

    void setRInfo(const uint64_t _r_info) { r_info = _r_info; }

    uint64_t getSymbolInfo() const { return r_info >> 32; }

    void setSymbolInfo(const uint64_t symbolInfo) {
        r_info &= 0xffffffff;
        r_info |= (symbolInfo << 32);
    }

    uint64_t getTypeInfo() const { return r_info & 0x0ff; }

    std::shared_ptr<Symbol> getSym() const { return sym; }

    void setSym(const std::shared_ptr<Symbol> &_sym) { sym = _sym; }

    uint64_t getRAddend() const { return r_addend; }

    void setRAddend(const int64_t _addend) { r_addend = _addend; }

    void writeDataTo(llvm::object::ELF64LE::Rela *rela) {
        rela->r_offset = r_offset;
        rela->r_info = r_info;
        rela->r_addend = r_addend;
    }

    void dump(std::ostream &oss) const {
        oss << std::hex;
        oss << std::setfill('0');
        oss << std::setw(16) << r_offset << " ";

        oss << std::setw(16) << r_info << " ";

        oss << std::setfill(' ');
        oss << std::setw(20) << TypeToString::relaTypeToString(getTypeInfo()) << " ";

        oss << std::setfill('0');
        oss << std::setw(16) << sym->getStValue() << " ";

        oss << std::setfill(' ');
        oss << std::setw(20) << sym->getName()->getValue() << " ";

        if (r_addend < 0) {
            oss << "- " << r_addend;
        } else {
            oss << "+ " << r_addend;
        }

        oss << std::dec;
    }
};

} // namespace iclang

#endif //ICLANG_RELOCATION_HPP
