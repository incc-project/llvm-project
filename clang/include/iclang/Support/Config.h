//===--- Config.h - IClang config ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// IClang config.
// Format:
// {
//   "iClangMode": string,
//   "iClangPaths": [string]
// }
//
// iClangMode:
// * "Inc": function-level incremental compilation.
// * "IncTest": inc test mode for IClang developers.
// * "ShareMaster": master mode of shared compilation optimization.
// * "ShareClient": client mode of shared compilation optimization.
// * "ShareTest": share test mode for IClang developers.
// * "Profile": profile Clang.
// * "Clang": default, equivalent to Clang.
//
// iClangPaths: Only works in Inc mode, provide the absolute paths of the
// source files that require incremental compilation.
//
//===----------------------------------------------------------------------===/

#ifndef ICLANG_CONFIG_H
#define ICLANG_CONFIG_H

#include <string>
#include <unordered_set>

namespace iclang {

enum class IClangMode {
  IncMode,
  IncTestMode,
  ShareMasterMode,
  ShareClientMode,
  ShareTestMode,
  TestMode,
  ProfileMode,
  ClangMode
};

class Config {
private:
  IClangMode iClangMode = IClangMode::ClangMode;

  std::unordered_set<std::string> iClangPaths;

public:
  Config() = default;

  void loadConfig(const std::string &configJsonPath);

  IClangMode getIClangMode() const { return iClangMode; }

  const std::unordered_set<std::string> &getIClangPaths() const {
    return iClangPaths;
  }

  bool containsIClangPath(const std::string &iClangPath) const {
    return iClangPaths.find(iClangPath) != iClangPaths.end();
  }
};

} // namespace iclang

#endif // ICLANG_CONFIG_H
