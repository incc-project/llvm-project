#include "iclang/ASTSupport/ASTGlobal.h"

#include <iomanip>
#include <sstream>

#include "clang/Lex/Lexer.h"

namespace iclang {

void ASTGlobal::init(const Global &global, clang::Sema *_sema) {
  iClangMode = global.getIClangMode();
  std::unique_ptr<ASTMetaData> ptr;
  if (iClangMode == IClangMode::IncMode) {
    astMetaData = illvm::make_owner<IncASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::IncTestMode) {
    astMetaData = illvm::make_owner<IncTestASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::ShareMasterMode) {
    astMetaData =
        illvm::make_owner<ShareMasterASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::ShareClientMode) {
    astMetaData =
        illvm::make_owner<ShareClientASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::ShareTestMode) {
    astMetaData =
        illvm::make_owner<ShareTestASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::TestMode) {
    astMetaData = illvm::make_owner<TestASTMetaData>().moveTo<ASTMetaData>();
  } else if (iClangMode == IClangMode::LineMacroTestMode) {
    astMetaData =
        illvm::make_owner<LineMacroTestASTMetaData>().moveTo<ASTMetaData>();
  } else {
    astMetaData = illvm::make_owner<ASTMetaData>();
  }
  sema = _sema;
  astNameGenerator =
      std::make_unique<clang::ASTNameGenerator>(sema->getASTContext());
}

std::string ASTGlobal::getMangledName(const clang::NamedDecl *decl) const {
  if (decl && decl->getDeclName()) {
    if (llvm::isa<clang::RequiresExprBodyDecl>(decl->getDeclContext())) {
      return "";
    }
    auto *varDecl = llvm::dyn_cast<clang::VarDecl>(decl);
    if (varDecl && varDecl->hasLocalStorage()) {
      return "";
    }
    return astNameGenerator->getName(decl);
  }
  return "";
}

illvm::SourceInterval
ASTGlobal::getDeclSourceInterval(const clang::Decl *decl) const {
  illvm::SourceInterval res{};

  res.isValid = false;

  const auto sr = decl->getSourceRange();
  const auto &sm =  getSourceManager();

  // start
  const clang::FullSourceLoc startFullSourceLoc(sr.getBegin(), sm);
  if (startFullSourceLoc.isInvalid()) {
    return res;
  }
  res.startLine = startFullSourceLoc.getExpansionLineNumber();
  res.startColumn = startFullSourceLoc.getExpansionColumnNumber();

  // end
  const clang::SourceLocation endSourceLoc = clang::Lexer::getLocForEndOfToken(
      sr.getEnd(), 0, sm, getLangOpts());
  const clang::FullSourceLoc endFullSourceLoc(endSourceLoc, sm);
  if (endFullSourceLoc.isInvalid()) {
    return res;
  }
  res.endLine = endFullSourceLoc.getExpansionLineNumber();
  res.endColumn = endFullSourceLoc.getExpansionColumnNumber();

  // [)
  res.startOffset = startFullSourceLoc.getFileOffset();
  res.endOffset = endFullSourceLoc.getFileOffset();

  res.filename = "";

  res.isValid = true;

  return res;
}

std::string ASTGlobal::dumpDecl(const clang::Decl *decl) const {
  if (decl == nullptr) {
    return "nullptr";
  }

  std::ostringstream oss;

  oss << "[" << decl->getDeclKindName() << "] " << decl << " ";

  if (auto *namedDecl = llvm::dyn_cast<clang::NamedDecl>(decl)) {
    oss << namedDecl->getNameAsString() + "(" + getMangledName(namedDecl) + ")";
  }

  oss << getDeclSourceInterval(decl).toString();

  return oss.str();
}

void ASTGlobal::addDisableWarningDecl(const clang::Decl *decl) {
  disableWarningDecls.insert(decl->getCanonicalDecl());
}

bool ASTGlobal::isDisableWarningDecl(const clang::Decl *decl) const {
  return disableWarningDecls.find(decl->getCanonicalDecl()) !=
         disableWarningDecls.end();
}

} // namespace iclang