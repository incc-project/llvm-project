set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  Support
  ILLVMSupport
  )

add_clang_library(clangIClangFuncX
  BasicAnalysis.cpp
  RefSymbolAnalysis.cpp
  ReusableInstAnalysis.cpp
  TestAnalysis.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  clangIClangSupport
  clangIClangASTSupport
  ${system_libs}
)