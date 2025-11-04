#include "iclang/CC1Driver/TestCC1Driver.h"

#include "iclang/CC1Driver/CC1DriverBase.h"
#include "iclang/FuncX/TestAnalysis.h"

namespace iclang {

void LineMacroTestCC1Driver::run() {
  auto &global = Global::getInstance();

  assert(global.getIClangMode() == IClangMode::LineMacroTestMode);

  auto &astGlobal = ASTGlobal::getInstance();
  auto &context = astGlobal.getContext();

  auto metaData = global.getMetaData<LineMacroTestMetaData>();
  const auto astMetaData = astGlobal.getASTMetaData<LineMacroTestASTMetaData>();

  funcx::LineMacroTestAnalysis lineMacroTestAnalysis(astGlobal);
  lineMacroTestAnalysis.TraverseDecl(context.getTranslationUnitDecl());

  metaData->totalFuncNum = lineMacroTestAnalysis.getTotalFuncNum();
  metaData->funcWithLineMacroNum =
      lineMacroTestAnalysis.getFuncWithLineMacroNum();
}

}