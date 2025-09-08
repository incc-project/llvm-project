//===--- Json.h - ILLVM json utils --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ILLVM json utils.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_JSON_H
#define ILLVM_JSON_H

#include <string>

#include "llvm/Support/JSON.h"

namespace illvm {

class JsonWrapper {
public:
  static llvm::json::Object loadJson(const std::string &jsonData);

  static std::string storeJson(const llvm::json::Object &root);
};

} // namespace illvm

#endif // ILLVM_JSON_H
