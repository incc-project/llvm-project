set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  Support
  )

add_clang_library(iClangFuncX
  BasicAnalysis.cpp
  RefSymbolAnalysis.cpp
  ReusableInstAnalysis.cpp
  TestAnalysis.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  iClangSupport
  iClangASTSupport
  ILLVMSupport
  ${system_libs}
)