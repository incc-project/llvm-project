#include "iclang/Driver/Driver.h"

#include "iclang/ASTSupport/ASTGlobal.h"
#include "iclang/Driver/DriverBase.h"
#include "iclang/Driver/IncDriver.h"
#include "iclang/Driver/ShareDriver.h"
#include "iclang/Driver/TestDriver.h"

#include "illvm/Support/Logger.h"

namespace iclang {

int Driver::run(const clang::driver::Action::ActionClass &kind,
                const std::vector<clang::driver::InputInfo> &inputInfos,
                const std::vector<std::string> &outputFilenames,
                const llvm::SmallVector<const char *, 128> &originalArgv,
                const clang::driver::Driver &clangDriver) {
  auto &global = Global::getInstance();
  auto &astGlobal = ASTGlobal::getInstance();

  const auto &logger = illvm::Logger::getInstance();

  logger.assertTrue(global.isEnabled(), "IClang is not enabled");

  if (!DriverBase::init(global, astGlobal, kind, inputInfos, outputFilenames,
                        originalArgv)) {
    global.setEnabled(false);
    return DriverBase::clangCompile(clangDriver, originalArgv);
  }

  const auto iClangMode = global.getConfig().getIClangMode();
  if (iClangMode == IClangMode::IncMode) {
    return IncDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::IncTestMode) {
    return IncTestDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::ShareMasterMode) {
    return ShareMasterDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::ShareClientMode) {
    return ShareClientDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::ShareTestMode) {
    return ShareTestDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::TestMode) {
    return TestDriver::run(global, originalArgv, clangDriver);
  }
  if (iClangMode == IClangMode::ProfileMode) {
    return ProfileDriver::run(global, originalArgv, clangDriver);
  }

  return DriverBase::clangCompile(clangDriver, originalArgv);
}

} // namespace iclang