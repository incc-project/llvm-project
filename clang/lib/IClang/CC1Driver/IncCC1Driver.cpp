#include "iclang/CC1Driver/IncCC1Driver.h"

#include "iclang/ASTSupport/ASTGlobal.h"
#include "iclang/FuncX/ReusableInstAnalysis.h"
#include "illvm/Support/Diagnostics.h"
#include "illvm/Support/FileSystem.h"
#include "illvm/Support/Strings.h"

namespace iclang {

// Capture as many consecutive "#include" as possible and
// ensure that no other macros are captured.
// For example:
// ```
// // hello
// #include <vector>
// #include <string> <--- Only captured here.
// #ifndef A
// #define A
// #include <map>
// #endif
// #include <set>
// ```
static unsigned calCacheLine(const unsigned mainFirstDeclLine,
                             const std::vector<std::string> &firstNLines) {
  unsigned res = 0;
  for (unsigned i = 0; i < mainFirstDeclLine; ++i) {
    const std::string formatStr =
        illvm::Strings::trimWhitespace(firstNLines[i]);
    if (!illvm::Strings::hasPrefix(formatStr, "#")) {
      continue;
    }
    if (illvm::Strings::hasPrefix(formatStr, "#include")) {
      res = i + 1;
    } else {
      break;
    }
  }
  return res;
}

static void recordHeader(const std::shared_ptr<IncMetaData> &metaData,
                         clang::ASTContext &context) {
  metaData->topIncludeRegion.clear();
  metaData->headerTs.clear();

  funcx::TopIncludeRegionVisitor topIncludeRegionVisitor(context);
  topIncludeRegionVisitor.TraverseDecl(context.getTranslationUnitDecl());
  const unsigned mainFirstDeclLine =
      topIncludeRegionVisitor.getMainFirstDeclLine();
  if (mainFirstDeclLine <= 0) {
    return;
  }

  const auto firstNLines = illvm::FileSystem::readFirstNLines(
      metaData->inputPath, mainFirstDeclLine);
  assert(firstNLines.size() == mainFirstDeclLine);
  const unsigned cacheLine = calCacheLine(mainFirstDeclLine, firstNLines);
  if (cacheLine <= 0) {
    return;
  }

  // Record top include region.
  for (size_t i = 0; i < cacheLine; i++) {
    metaData->topIncludeRegion.push_back(firstNLines[i]);
  }

  // Record header timestamp.
  const auto &sm = context.getSourceManager();
  for (auto it = sm.fileinfo_begin(), ed = sm.fileinfo_end(); it != ed; ++it) {
    const std::string rPath = it->getFirst().getName().str();
    if (!illvm::FileSystem::checkFileExists(rPath)) {
      continue;
    }
    const std::string absPath = illvm::FileSystem::toAbsPath(rPath);
    if (absPath == metaData->inputPath) {
      continue;
    }
    const long long ts = illvm::FileSystem::getLastModificationTime(absPath);
    metaData->headerTs[absPath] = ts;
  }
}

static void runBase(clang::Sema *sema,
                    const std::shared_ptr<IncMetaData> &metaData,
                    const std::shared_ptr<IncTestMetaData> &testMetaData,
                    ASTGlobal &astGlobal) {
  // Step6. Record top include region and header timestamp (!incFlag).
  if (!metaData->incFlag) {
    recordHeader(metaData, astGlobal.getContext());
  }

  // Step7. FuncX (incFlag).
  if (metaData->incFlag) {
    const auto start = std::chrono::high_resolution_clock::now();

    funcx::ReusableInstAnalysis reusableInstAnalysis(metaData, astGlobal);
    reusableInstAnalysis.run(sema->PendingInstantiations);

    if (testMetaData != nullptr) {
      illvm::FileSystem::saveSet(testMetaData->funcXTxtPath,
                                 metaData->funcXSet);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> duration = end - start;
    metaData->funcXTime = duration.count();
  }

  sema->PerformPendingInstantiations();
}

void IncCC1Driver::run(clang::Sema *sema) {
  const auto &global = Global::getInstance();

  ILLVM_FCHECK(global.isEnabled(), "IClang is not enabled");
  ILLVM_FCHECK(global.getConfig().getIClangMode() == IClangMode::IncMode,
                      "expected IncMode");

  auto &astGlobal = ASTGlobal::getInstance();

  auto metaData = std::static_pointer_cast<IncMetaData>(global.getMetaData());
  auto astMetaData =
      std::static_pointer_cast<IncASTMetaData>(astGlobal.getASTMetaData());

  ILLVM_FCHECK(metaData != nullptr, "IncMetaData convert failed");
  ILLVM_FCHECK(astMetaData != nullptr, "IncASTMetaData convert failed");

  runBase(sema, metaData, nullptr, astGlobal);
}

void IncTestCC1Driver::run(clang::Sema *sema) {
  const auto &global = Global::getInstance();

  ILLVM_FCHECK(global.isEnabled(), "IClang is not enabled");
  ILLVM_FCHECK(global.getConfig().getIClangMode() ==
                        IClangMode::IncTestMode,
                    "expected IncTestTestMode");

  auto &astGlobal = ASTGlobal::getInstance();

  auto metaData =
      std::static_pointer_cast<IncTestMetaData>(global.getMetaData());
  auto astMetaData =
      std::static_pointer_cast<IncTestASTMetaData>(astGlobal.getASTMetaData());

  ILLVM_FCHECK(metaData != nullptr, "IncTestMetaData convert failed");
  ILLVM_FCHECK(astMetaData != nullptr,
                    "IncTestASTMetaData convert failed");

  runBase(sema, metaData, metaData, astGlobal);
}

} // namespace iclang