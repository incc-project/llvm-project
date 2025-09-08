//===--- TestDriver.h - IClang test driver -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// IClang test driver.
//
//===----------------------------------------------------------------------===/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "iclang/Driver/DriverBase.h"

namespace iclang {

class TestDriver {
public:
  static int run(Global &global,
                 const llvm::SmallVector<const char *, 128> &originalArgv,
                 const clang::driver::Driver &clangDriver);
};

class ProfileDriver {
public:
  static int run(Global &global,
                 const llvm::SmallVector<const char *, 128> &originalArgv,
                 const clang::driver::Driver &clangDriver);
};

} // namespace iclang

#endif //TESTDRIVER_H
