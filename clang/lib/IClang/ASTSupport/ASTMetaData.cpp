#include "iclang/ASTSupport/ASTMetaData.h"

#include <iomanip>
#include <sstream>

namespace iclang {

void ShareCheckASTMetaData::addEmitGlobalFuncDef(
    const clang::FunctionDecl *funcDecl) {
  emitGlobalFuncDefs.insert(funcDecl->getCanonicalDecl());
}

} // namespace iclang
