#include "illvm/FuncV/ELF/Reuse.h"

#include "illvm/FuncV/ELF/ReuseEhFrame.h"
#include "illvm/FuncV/ELF/ReuseRelocation.h"
#include "illvm/FuncV/ELF/ReuseSection.h"
#include "illvm/FuncV/ELF/ReuseSymbol.h"

namespace illvm {
namespace funcv {
namespace elf {

void Reuse::run() {
  // (1) Build BDG.
  BDG bdg;
  bdg.build(oldObjFile, newObjFile);
  bdg.propagation(funcXSet);

  // (2) Reuse sections.
  ReuseSection::run(newObjFile, bdg);

  // (3) Reuse symbols.
  ReuseSymbol::run(newObjFile, bdg);

  // (4) Reuse eh_frame.
  ReuseEhFrame::run(newObjFile, bdg);

  // (5) Reuse relocations.
  ReuseRelocation::run(newObjFile, bdg);
}

} // namespace elf
} // namespace funcv
} // namespace illvm
