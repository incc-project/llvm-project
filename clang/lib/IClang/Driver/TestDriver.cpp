#include "iclang/Driver/TestDriver.h"

namespace iclang {

int TestDriver::run(Global &global,
                 const llvm::SmallVector<const char *, 128> &originalArgv,
                 const clang::driver::Driver &clangDriver) {
  ILLVM_FCHECK(false, "We haven't implemented TestDriver yet");
  return 0;
}

int LineMacroTestDriver::run(
    Global &global, const llvm::SmallVector<const char *, 128> &originalArgv,
    const clang::driver::Driver &clangDriver) {
  assert(global.getIClangMode() == IClangMode::LineMacroTestMode);
  auto metaData = global.getMetaData<LineMacroTestMetaData>();
  const int res = DriverBase::compile(clangDriver, originalArgv, -1, "", -1, "",
                                      -1, "", {}, {});
  DriverBase::fini(global);
  return res;
}

int ProfileDriver::run(Global &global,
                 const llvm::SmallVector<const char *, 128> &originalArgv,
                 const clang::driver::Driver &clangDriver) {
  ILLVM_FCHECK(false, "We haven't implemented ProfileDriver yet");
  return 0;
}

} // namespace iclang