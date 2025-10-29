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

#include <unordered_set>

#include "clang/AST/Decl.h"

namespace iclang {

class ASTMetaData {
public:
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
