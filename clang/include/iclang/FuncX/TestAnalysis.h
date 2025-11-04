//===--- TestAnalysis.h - Test analysis -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Test analysis
//
//===----------------------------------------------------------------------===/

#ifndef ICLANG_TESTANALYSIS_H
#define ICLANG_TESTANALYSIS_H

#include "iclang/ASTSupport/ASTGlobal.h"

#include "clang/AST/RecursiveASTVisitor.h"

namespace iclang {
namespace funcx {

class TestAnalysis : public clang::RecursiveASTVisitor<TestAnalysis> {
private:
  ASTGlobal &astGlobal;

public:
  explicit TestAnalysis(ASTGlobal &_astGlobal) : astGlobal(_astGlobal) {}

  int depth = 0;

  static std::string dumpPrefix(const int n);

  bool shouldVisitTemplateInstantiations() const { return true; }

  bool shouldVisitImplicitCode() const { return true; }

  bool TraverseDecl(clang::Decl *decl);

  bool TraverseStmt(clang::Stmt *stmt, DataRecursionQueue *queue = nullptr);
};

class LineMacroTestAnalysis : public clang::RecursiveASTVisitor<LineMacroTestAnalysis> {
private:
  ASTGlobal &astGlobal;
  unsigned totalFuncNum = 0;
  unsigned funcWithLineMacroNum = 0;

public:
  explicit LineMacroTestAnalysis(ASTGlobal &_astGlobal) : astGlobal(_astGlobal) {}

  bool shouldVisitTemplateInstantiations() const { return false; }

  bool shouldVisitImplicitCode() const { return true; }

  bool TraverseDecl(clang::Decl *decl);

  bool TraverseStmt(clang::Stmt *stmt, DataRecursionQueue *queue = nullptr);

  unsigned getTotalFuncNum() const { return totalFuncNum; }

  unsigned getFuncWithLineMacroNum() const { return funcWithLineMacroNum; }
};

} // namespace funcx
} // namespace iclang

#endif // ICLANG_TESTANALYSIS_H
