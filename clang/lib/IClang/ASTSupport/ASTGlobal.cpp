#include "iclang/ASTSupport/ASTGlobal.h"

#include <iomanip>
#include <sstream>

namespace iclang {

void ASTGlobal::init(const Global &global) {
  iClangMode = global.getConfig().getIClangMode();
  if (iClangMode == IClangMode::IncMode) {
    astMetaData = std::make_unique<IncASTMetaData>();
  } else if (iClangMode == IClangMode::IncTestMode) {
    astMetaData = std::make_unique<IncTestASTMetaData>();
  } else if (iClangMode == IClangMode::ShareMasterMode) {
    astMetaData = std::make_unique<ShareMasterASTMetaData>();
  } else if (iClangMode == IClangMode::ShareClientMode) {
    astMetaData = std::make_unique<ShareClientASTMetaData>();
  } else if (iClangMode == IClangMode::ShareTestMode) {
    astMetaData = std::make_unique<ShareTestASTMetaData>();
  } else if (iClangMode == IClangMode::TestMode) {
    astMetaData = std::make_unique<TestASTMetaData>();
  } else {
    astMetaData = std::make_unique<ASTMetaData>();
  }
}

void ASTGlobal::setContext(clang::ASTContext *_context) const {
  astMetaData->context = _context;
  astMetaData->astNameGenerator =
      std::make_unique<clang::ASTNameGenerator>(*_context);
}

clang::ASTContext &ASTGlobal::getContext() const {
  return *astMetaData->context;
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
    return astMetaData->astNameGenerator->getName(decl);
  }
  return "";
}

illvm::SourceInterval
ASTGlobal::getDeclSourceInterval(const clang::Decl *decl) const {
  illvm::SourceInterval res{};

  res.isValid = false;

  const auto sr = decl->getSourceRange();
  const auto &sm = astMetaData->context->getSourceManager();

  // start
  const clang::FullSourceLoc startFullSourceLoc(sr.getBegin(), sm);
  if (startFullSourceLoc.isInvalid()) {
    return res;
  }
  res.startLine = startFullSourceLoc.getExpansionLineNumber();
  res.startColumn = startFullSourceLoc.getExpansionColumnNumber();

  // end
  const clang::SourceLocation endSourceLoc = clang::Lexer::getLocForEndOfToken(
      sr.getEnd(), 0, sm, astMetaData->context->getLangOpts());
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

void ASTGlobal::addDisableWarningDecl(const clang::Decl *decl) const {
  astMetaData->disableWarningDecls.insert(decl->getCanonicalDecl());
}

bool ASTGlobal::isDisableWarningDecl(const clang::Decl *decl) const {
  return astMetaData->disableWarningDecls.find(decl->getCanonicalDecl()) !=
         astMetaData->disableWarningDecls.end();
}

} // namespace iclang