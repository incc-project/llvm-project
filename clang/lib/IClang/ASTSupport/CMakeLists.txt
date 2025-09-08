set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  )

add_clang_library(iClangASTSupport
  ASTGlobal.cpp
  ASTMetaData.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  iClangSupport
  ILLVMSupport
  ${system_libs}
)