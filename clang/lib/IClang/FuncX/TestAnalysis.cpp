#include "iclang/FuncX/TestAnalysis.h"

#include <iomanip>
#include <sstream>

namespace iclang {
namespace funcx {

std::string TestAnalysis::dumpPrefix(const int n) {
  std::ostringstream oss;
  for (int i = 0; i < n; i++) {
    oss << "|--";
  }
  return oss.str();
}

bool TestAnalysis::TraverseDecl(clang::Decl *decl) {
  if (!decl) {
    return true;
  }

  llvm::errs() << dumpPrefix(depth) << astGlobal.dumpDecl(decl);

  if (const auto *funcTempDecl =
          llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
    llvm::errs() << "(templatedDecl: "
                 << astGlobal.dumpDecl(funcTempDecl->getTemplatedDecl()) << ")";
  }

  if (const auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
    llvm::errs() << " (isExternC: " << funcDecl->isExternC() << ")";
    const auto *desTempDecl = funcDecl->getDescribedTemplate();
    llvm::errs() << " (desTempDecl: " << astGlobal.dumpDecl(desTempDecl) << ")";
    const auto *priTempDecl = funcDecl->getPrimaryTemplate();
    llvm::errs() << " (priTempDecl: " << astGlobal.dumpDecl(priTempDecl) << ")";
  }

  llvm::errs() << "\n";

  if (const auto *usingShadowDecl =
          llvm::dyn_cast<clang::UsingShadowDecl>(decl)) {
    llvm::errs() << dumpPrefix(depth + 1) << "<UsingShadowDecl::getTargetDecl> "
                 << astGlobal.dumpDecl(usingShadowDecl->getTargetDecl())
                 << "\n";
    llvm::errs() << dumpPrefix(depth + 1)
                 << "<UsingShadowDecl::getUnderlyingDecl> "
                 << astGlobal.dumpDecl(usingShadowDecl->getUnderlyingDecl())
                 << "\n";
  }
  if (const auto *usingDecl = llvm::dyn_cast<clang::UsingDecl>(decl)) {
    llvm::errs() << dumpPrefix(depth + 1) << "<UsingDecl::getUnderlyingDecl> "
                 << astGlobal.dumpDecl(usingDecl->getUnderlyingDecl()) << "\n";
  }

  depth += 1;

  const bool res = RecursiveASTVisitor::TraverseDecl(decl);

  depth -= 1;

  return res;
}

bool TestAnalysis::TraverseStmt(clang::Stmt *stmt, DataRecursionQueue *queue) {
  if (!stmt) {
    return true;
  }

  if (const auto *declRefExpr = llvm::dyn_cast<clang::DeclRefExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<DeclRefExpr> "
                 << astGlobal.dumpDecl(declRefExpr->getDecl()) << "\n";
  } else if (const auto *memberExpr = llvm::dyn_cast<clang::MemberExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<MemberExpr> "
                 << astGlobal.dumpDecl(memberExpr->getMemberDecl()) << "\n";
  } else if (const auto *ctor = llvm::dyn_cast<clang::CXXConstructExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<CXXConstructExpr> "
                 << astGlobal.dumpDecl(ctor->getConstructor()) << "\n";
  } else if (const auto *usLookupExpr =
                 llvm::dyn_cast<clang::UnresolvedLookupExpr>(stmt)) {
    for (const auto *decl : usLookupExpr->decls()) {
      llvm::errs() << dumpPrefix(depth) << "<UnresolvedLookupExpr> "
                   << astGlobal.dumpDecl(decl) << "\n";
    }
  } else if (const auto *usMemberExpr =
                 llvm::dyn_cast<clang::UnresolvedMemberExpr>(stmt)) {
    for (const auto *decl : usMemberExpr->decls()) {
      llvm::errs() << dumpPrefix(depth) << "<UnresolvedMemberExpr> "
                   << astGlobal.dumpDecl(decl) << "\n";
    }
  } else if (const auto *newExpr = llvm::dyn_cast<clang::CXXNewExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<CXXNewExpr::getOperatorNew> "
                 << astGlobal.dumpDecl(newExpr->getOperatorNew()) << "\n";
    llvm::errs() << dumpPrefix(depth) << "<CXXNewExpr::getOperatorDelete> "
                 << astGlobal.dumpDecl(newExpr->getOperatorDelete()) << "\n";
  } else if (const auto *deleteExpr =
                 llvm::dyn_cast<clang::CXXDeleteExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<CXXDeleteExpr::getOperatorDelete> "
                 << astGlobal.dumpDecl(deleteExpr->getOperatorDelete()) << "\n";
  } else if (const auto *icie =
                 llvm::dyn_cast<clang::CXXInheritedCtorInitExpr>(stmt)) {
    llvm::errs() << dumpPrefix(depth) << "<CXXInheritedCtorInitExpr> "
                 << astGlobal.dumpDecl(icie->getConstructor()) << "\n";
  }

  return RecursiveASTVisitor::TraverseStmt(stmt, queue);
}

} // namespace funcx
} // namespace iclang