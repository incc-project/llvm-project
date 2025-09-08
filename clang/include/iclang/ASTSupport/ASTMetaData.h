//===--- ASTMetaData.h - IClang AST meta data ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// IClang AST meta data.
//
//===----------------------------------------------------------------------===/

#ifndef ICLANG_ASTMETADATA_H
#define ICLANG_ASTMETADATA_H

#include <memory>
#include <string>
#include <unordered_set>

#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"

namespace iclang {

class ASTMetaData {
public:
  // AST context.
  clang::ASTContext *context = nullptr;

  // Mangled name generator.
  std::unique_ptr<clang::ASTNameGenerator> astNameGenerator = nullptr;

  // Turn off warnings caused by funcx.
  std::unordered_set<const clang::Decl*> disableWarningDecls;
};

class IncASTMetaData final : public ASTMetaData {
public:
};

class IncTestASTMetaData final : public ASTMetaData {
public:
};

class ShareMasterASTMetaData final : public ASTMetaData {
public:
};

class ShareClientASTMetaData final : public ASTMetaData {
public:
};

class ShareTestASTMetaData final : public ASTMetaData {
public:
  std::unordered_set<const clang::FunctionDecl *> emitGlobalFuncDefs = {};

  void addEmitGlobalFuncDef(const clang::FunctionDecl *funcDecl);
};

class TestASTMetaData final : public ASTMetaData {
public:
};

} // namespace iclang

#endif // ICLANG_ASTMETADATA_H
