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

#include "iclang/ASTSupport/ASTMetaData.h"
#include "iclang/Support/Global.h"

namespace iclang {

class ASTGlobal {
private:
  IClangMode iClangMode = IClangMode::ClangMode;
  std::shared_ptr<ASTMetaData> astMetaData = nullptr;

  ASTGlobal() = default;

public:
  ASTGlobal(const ASTGlobal &) = delete;
  ASTGlobal &operator=(const ASTGlobal &) = delete;

  static ASTGlobal &getInstance() {
    static ASTGlobal instance;
    return instance;
  }

  void init(const Global &global);

  const auto &getASTMetaData() const { return astMetaData; }

  std::shared_ptr<IncASTMetaData> getIncASTMetaData() const {
    if (iClangMode == IClangMode::IncMode) {
      return std::static_pointer_cast<IncASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  std::shared_ptr<IncTestASTMetaData> getIncTestASTMetaData() const {
    if (iClangMode == IClangMode::IncTestMode) {
      return std::static_pointer_cast<IncTestASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareMasterASTMetaData> getShareMasterASTMetaData() const {
    if (iClangMode == IClangMode::ShareMasterMode) {
      return std::static_pointer_cast<ShareMasterASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareClientASTMetaData> getShareClientASTMetaData() const {
    if (iClangMode == IClangMode::ShareClientMode) {
      return std::static_pointer_cast<ShareClientASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareTestASTMetaData> getShareTestASTMetaData() const {
    if (iClangMode == IClangMode::ShareTestMode) {
      return std::static_pointer_cast<ShareTestASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  std::shared_ptr<TestASTMetaData> getTestASTMetaData() const {
    if (iClangMode == IClangMode::TestMode) {
      return std::static_pointer_cast<TestASTMetaData>(astMetaData);
    }
    return nullptr;
  }

  void setContext(clang::ASTContext *_context) const;

  clang::ASTContext &getContext() const;

  std::string getMangledName(const clang::NamedDecl *decl) const;

  illvm::SourceInterval getDeclSourceInterval(const clang::Decl *decl) const;

  std::string dumpDecl(const clang::Decl *decl) const;

  void addDisableWarningDecl(const clang::Decl *decl) const;

  bool isDisableWarningDecl(const clang::Decl *decl) const;
};

} // namespace iclang

#endif // ICLANG_ASTGLOBAL_H
