#include "illvm/FuncV/ELF/Reuse.h"

#include "illvm/FuncV/ELF/ReuseEhFrame.h"
#include "illvm/FuncV/ELF/ReuseRelocation.h"
#include "illvm/FuncV/ELF/ReuseSection.h"
#include "illvm/FuncV/ELF/ReuseSymbol.h"

namespace illvm {
namespace funcv {
namespace elf {

llvm::Error Reuse::run() {
  // (1) Build BDG.
  BDG bdg;
  if (auto err = bdg.build(oldObjFile, newObjFile)) {
    return err;
  }
  bdg.propagation(funcXSet);

  // (2) Reuse sections.
  if (auto err = ReuseSection::run(newObjFile, bdg)) {
    return err;
  }

  // (3) Reuse symbols.
  if (auto err = ReuseSymbol::run(newObjFile, bdg)) {
    return err;
  }

  // (4) Reuse eh_frame.
  if (auto err = ReuseEhFrame::run(newObjFile, bdg)) {
    return err;
  }

  // (5) Reuse relocations.
  if (auto err = ReuseRelocation::run(newObjFile, bdg)) {
    return err;
  }

  return llvm::Error::success();
}

} // namespace elf
} // namespace funcv
} // namespace illvm
