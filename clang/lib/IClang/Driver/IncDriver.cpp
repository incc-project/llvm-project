#include "iclang/Driver/IncDriver.h"

#include "illvm/FuncV/ELF/FuncV.h"
#include "illvm/Support/FileSystem.h"
#include "illvm/Support/Logger.h"
#include "illvm/Support/Time.h"

namespace iclang {

static bool isCacheOption(const std::string &opt) {
  return opt == "c++-header" || opt == "-include-pch" || opt == "-emit-pch";
}

static bool isOptOption(const std::string &opt) {
  if (opt == "-O0") {
    return false;
  }
  if (opt == "-O1" || opt == "-O2" || opt == "-O3" || opt == "-O4" ||
      opt == "-Ofast" || opt == "-Os" || opt == "-Oz" || opt == "-Og" ||
      opt == "-O") {
    return true;
  }
  return false;
}

static bool
checkArgs(const llvm::SmallVector<const char *, 128> &originalArgv) {
  std::string opt;
  for (size_t i = 0; i < originalArgv.size(); i++) {
    opt = originalArgv[i];
    if (isCacheOption(opt)) {
      return false;
    }
    if (isOptOption(opt)) {
      illvm::Logger::getInstance().warning(
          "Please ensure that IClang is only enabled under -O0");
      return false;
    }
  }
  return true;
}

static void configPaths(const std::shared_ptr<IncMetaData> &metaData,
                        const std::shared_ptr<IncTestMetaData> &testMetaData) {
  const auto preWorkPath = metaData->iClangDirPath[PrevDir];
  const auto workPath = metaData->iClangDirPath[CurDir];

  metaData->iInputDir =
      "-I" + illvm::FileSystem::parentPath(metaData->inputPath);
  metaData->cachePath[PrevDir] =
      illvm::FileSystem::linkPath(preWorkPath, "iclang.cache");
  metaData->cachePath[CurDir] =
      illvm::FileSystem::linkPath(workPath, "iclang.cache");
  metaData->cacheHeaderPath[PrevDir] =
      illvm::FileSystem::linkPath(preWorkPath, "iclang.h");
  metaData->cacheHeaderPath[CurDir] =
      illvm::FileSystem::linkPath(workPath, "iclang.h");
  metaData->prevOPath = illvm::FileSystem::linkPath(workPath, "prev.o");

  if (testMetaData == nullptr) {
    return;
  }

  testMetaData->cacheSrcPath =
      illvm::FileSystem::linkPath(workPath, "iclang.cpp");
  testMetaData->partialOPath =
      illvm::FileSystem::linkPath(workPath, "partial.o");
  testMetaData->outputOPath = illvm::FileSystem::linkPath(workPath, "output.o");
  testMetaData->funcXTxtPath =
      illvm::FileSystem::linkPath(workPath, "funcx.txt");
}

static bool init(Global &global, const std::shared_ptr<IncMetaData> &metaData,
                 const std::shared_ptr<IncTestMetaData> &testMetaData,
                 const llvm::SmallVector<const char *, 128> &originalArgv) {
  if (!checkArgs(originalArgv)) {
    return false;
  }

  configPaths(metaData, testMetaData);

  return true;
}

// This function can also update topIncludeRegion, headerTs and recoverFlag
// according to the previous compilation metadata.
static bool calCanInc(const Global &global,
                      const std::shared_ptr<IncMetaData> &metaData) {
  const auto &logger = illvm::Logger::getInstance();

  // (1) .iclang/compile.json exits.
  if (!illvm::FileSystem::checkFileExists(metaData->compileJsonPath[PrevDir])) {
    metaData->cannotIncReason = "prev compile.json does not exist";
    return false;
  }
  // (2) Previous output exists.
  if (!illvm::FileSystem::checkFileExists(metaData->outputPath)) {
    metaData->cannotIncReason = "prev output does not exist";
    return false;
  }
  // (3) The last modified timestamp of previous output <= that of
  // .iclang/compile.json.
  const auto prevOutputTs =
      illvm::FileSystem::getLastModificationTime(metaData->outputPath);
  const auto compileStatTs = illvm::FileSystem::getLastModificationTime(
      metaData->compileJsonPath[PrevDir]);
  if (prevOutputTs > compileStatTs) {
    metaData->cannotIncReason =
        "the last modified timestamp of previous output has changed";
    return false;
  }
  // (4) The current command equals to the previous command
  const auto prevMetaData = std::static_pointer_cast<IncMetaData>(
      Global::loadMetaDataFromFile(metaData->compileJsonPath[PrevDir],
                                   global.getConfig().getIClangMode()));
  logger.assertTrue(prevMetaData != nullptr, "Parse prev compile.json error");
  if (prevMetaData->originalCommand != metaData->originalCommand) {
    metaData->cannotIncReason = "compilation command has changed";
    return false;
  }
  // (5) The top include region remains unchanged.
  if (prevMetaData->topIncludeRegion.empty()) {
    metaData->cannotIncReason = "top include region has changed";
    return false;
  }
  metaData->topIncludeRegion = illvm::FileSystem::readFirstNLines(
      metaData->inputPath, prevMetaData->topIncludeRegion.size());
  if (metaData->topIncludeRegion.size() !=
          prevMetaData->topIncludeRegion.size() ||
      (metaData->topIncludeRegion != prevMetaData->topIncludeRegion)) {
    metaData->topIncludeRegion.clear();
    metaData->cannotIncReason = "top include region has changed";
    return false;
  }
  // (6) The timestamp of all header files remains unchanged.
  for (auto &p : prevMetaData->headerTs) {
    if (!illvm::FileSystem::checkFileExists(p.first) ||
        illvm::FileSystem::getLastModificationTime(p.first) != p.second) {
      metaData->headerTs.clear();
      metaData->cannotIncReason = "header timestamps have changed";
      return false;
    }
    metaData->headerTs[p.first] = p.second;
  }
  // (7) Check recoverFlag.
  if (prevMetaData->recoverFlag) {
    metaData->recoverFlag = true;
    metaData->cannotIncReason = "recover mode";
    return false;
  }
  return true;
}

static int
cacheCompile(Global &global, const std::shared_ptr<IncMetaData> &metaData,
             const clang::driver::Driver &clangDriver,
             const llvm::SmallVector<const char *, 128> &originalArgv) {
  global.setEnabled(false);
  const int res = DriverBase::compile(
      clangDriver, originalArgv, metaData->inputIdx,
      metaData->cacheHeaderPath[PrevDir].c_str(), metaData->outputIdx,
      metaData->cachePath[PrevDir].c_str(), metaData->emitObjIdx, "-emit-pch",
      {{"-dependency-file", 1}, {"-MT", 1}, {"-x", 1}},
      {"-x", "c++-header", metaData->iInputDir.c_str(), "-ffunction-sections",
       "-fdata-sections"});
  global.setEnabled(true);
  return res;
}

static void
buildCache(Global &global, const std::shared_ptr<IncMetaData> &metaData,
           const clang::driver::Driver &clangDriver,
           const llvm::SmallVector<const char *, 128> &originalArgv) {
  const auto &logger = illvm::Logger::getInstance();

  metaData->skipTopIncludeRegionFlag = true;

  if (illvm::FileSystem::checkFileExists(metaData->cacheHeaderPath[PrevDir]) &&
      illvm::FileSystem::checkFileExists(metaData->cachePath[PrevDir])) {
    return;
  }

  illvm::FileSystem::saveVector(metaData->cacheHeaderPath[PrevDir],
                                metaData->topIncludeRegion);
  if (cacheCompile(global, metaData, clangDriver, originalArgv) != 0) {
    logger.fatal("Build cache failed");
  }
}

static int
initCompile(const clang::driver::Driver &clangDriver,
            const llvm::SmallVector<const char *, 128> &originalArgv) {
  return DriverBase::compile(clangDriver, originalArgv, -1, "", -1, "", -1, "",
                             {}, {"-ffunction-sections", "-fdata-sections"});
}

static int
incCompile(const std::shared_ptr<IncMetaData> &metaData,
           const clang::driver::Driver &clangDriver,
           const llvm::SmallVector<const char *, 128> &originalArgv) {
  return DriverBase::compile(
      clangDriver, originalArgv, -1, "", -1, "", -1, "", {},
      {"-ffunction-sections", "-fdata-sections", "-include-pch",
       metaData->cachePath[PrevDir].c_str(), metaData->iInputDir.c_str()});
}

static void funcv(const std::shared_ptr<IncMetaData> &metaData,
                  const std::shared_ptr<IncTestMetaData> &testMetaData) {
  const auto startFuncVTS = illvm::Time::currentTsMs();
  if (testMetaData != nullptr) {
    illvm::FileSystem::cpFile(metaData->outputPath, testMetaData->partialOPath);
  }
  illvm::funcv::elf::FuncV::run(metaData->prevOPath, metaData->outputPath,
                                metaData->outputPath, metaData->funcXSet);
  if (testMetaData != nullptr) {
    illvm::FileSystem::cpFile(metaData->outputPath, testMetaData->outputOPath);
  } else {
    illvm::FileSystem::rmFile(metaData->prevOPath);
  }
  const auto endFuncVTs = illvm::Time::currentTsMs();
  metaData->funcVTime = endFuncVTs - startFuncVTS;
}

static int runBase(Global &global, const std::shared_ptr<IncMetaData> &metaData,
                   const std::shared_ptr<IncTestMetaData> &testMetaData,
                   const llvm::SmallVector<const char *, 128> &originalArgv,
                   const clang::driver::Driver &clangDriver) {
  // Step1. Init.
  if (!init(global, metaData, testMetaData, originalArgv)) {
    // Back to clang.
    global.setEnabled(false);
    return DriverBase::clangCompile(clangDriver, originalArgv);
  }

  // Step2. Determine inc mode (iClangFlag).
  metaData->incFlag = calCanInc(global, metaData);

  if (metaData->incFlag) {
    // Step3. Build cache (iClangFlag && incFlag).
    buildCache(global, metaData, clangDriver, originalArgv);
    // Step4. Load prev binary symbol table (iClangFlag && incFlag).
    illvm::FileSystem::cpFile(metaData->outputPath, metaData->prevOPath);
    metaData->prevAPIs =
        illvm::funcv::elf::FuncV::onlyLoadSymbolTable(metaData->prevOPath);
  }

  // Step5. Compilation (iClangFlag).
  int res;
  if (!metaData->incFlag) {
    res = initCompile(clangDriver, originalArgv);
  } else {
    res = incCompile(metaData, clangDriver, originalArgv);
    if (res != 0) {
      if (testMetaData == nullptr) {
        illvm::FileSystem::rmFile(metaData->prevOPath);
      }
      return DriverBase::recover(global, clangDriver, originalArgv);
    }
  }

  // Steps 6 ~ 7 are in CC1Driver.

  if (metaData->incFlag) {
    // Step8. Post processing cache (iClangFlag && incFlag).
    illvm::FileSystem::mvFile(metaData->cacheHeaderPath[PrevDir],
                              metaData->cacheHeaderPath[CurDir]);
    illvm::FileSystem::mvFile(metaData->cachePath[PrevDir],
                              metaData->cachePath[CurDir]);
    // Step9. FuncV (iClangFlag && incFlag).
    funcv(metaData, testMetaData);
  }

  // Step10. Fini (iClangFlag).
  DriverBase::fini(global);
  return res;
  return 0;
}

int IncDriver::run(Global &global,
                   const llvm::SmallVector<const char *, 128> &originalArgv,
                   const clang::driver::Driver &clangDriver) {
  const auto &logger = illvm::Logger::getInstance();

  logger.assertTrue(global.isEnabled(), "IClang is not enabled");
  logger.assertTrue(global.getConfig().getIClangMode() == IClangMode::IncMode,
                    "expected IncMode");

  auto metaData = std::static_pointer_cast<IncMetaData>(global.getMetaData());

  logger.assertTrue(metaData != nullptr, "IncMetaData convert failed");

  return runBase(global, metaData, nullptr, originalArgv, clangDriver);
}

int IncTestDriver::run(Global &global,
                       const llvm::SmallVector<const char *, 128> &originalArgv,
                       const clang::driver::Driver &clangDriver) {
  const auto &logger = illvm::Logger::getInstance();

  logger.assertTrue(global.isEnabled(), "IClang is not enabled");
  logger.assertTrue(global.getConfig().getIClangMode() ==
                        IClangMode::IncTestMode,
                    "expected IncTestMode");

  auto metaData =
      std::static_pointer_cast<IncTestMetaData>(global.getMetaData());

  logger.assertTrue(metaData != nullptr, "IncTestMetaData convert failed");

  return runBase(global, metaData, metaData, originalArgv, clangDriver);
}

} // namespace iclang