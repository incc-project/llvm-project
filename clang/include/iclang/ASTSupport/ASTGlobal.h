//===--- ASTGlobal.h - IClang AST global data -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// IClang AST global data.
// This module was originally part of Utils. However, when link to llvm, it will
// introduce AST dependency for the llvm citizens that do not need
// AST dependency. Therefore, we decouple ASTUtils from Utils.
//
//===----------------------------------------------------------------------===/

#ifndef ICLANG_ASTGLOBAL_H
#define ICLANG_ASTGLOBAL_H

#include <memory>
#include <string>

#include "clang/AST/Mangle.h"
#include "clang/Basic/SourceManager.h"

#include "iclang/ASTSupport/ASTMetaData.h"
#include "iclang/Support/Global.h"

#include "illvm/Support/Memory.h"

namespace iclang {

class ASTGlobal {
private:
  IClangMode iClangMode = IClangMode::ClangMode;

  // AST context.
  clang::ASTContext *context = nullptr;

  // Mangled name generator.
  std::unique_ptr<clang::ASTNameGenerator> astNameGenerator = nullptr;

  // Turn off warnings caused by funcx.
  std::unordered_set<const clang::Decl*> disableWarningDecls = {};

  illvm::OPtr<ASTMetaData> astMetaData;

  ASTGlobal() = default;

public:
  ASTGlobal(const ASTGlobal &) = delete;
  ASTGlobal &operator=(const ASTGlobal &) = delete;

  static ASTGlobal &getInstance() {
    static ASTGlobal instance;
    return instance;
  }

  void init(const Global &global, clang::ASTContext *_context);

  template<typename T>
  illvm::BPtr<T> getASTMetaData() {
    return astMetaData.borrow().copyTo<T>();
  }

  clang::ASTContext &getContext() const;

  void addDisableWarningDecl(const clang::Decl *decl);

  bool isDisableWarningDecl(const clang::Decl *decl) const;

  std::string getMangledName(const clang::NamedDecl *decl) const;

  illvm::SourceInterval getDeclSourceInterval(const clang::Decl *decl) const;

  std::string dumpDecl(const clang::Decl *decl) const;
};

} // namespace iclang

#endif // ICLANG_ASTGLOBAL_H
