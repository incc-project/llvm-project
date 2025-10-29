set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  ILLVMSupport
  )

add_clang_library(clangIClangCC1Driver
  CC1DriverBase.cpp
  IncCC1Driver.cpp
  ShareCC1Driver.cpp
  TestCC1Driver.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  clangIClangSupport
  clangIClangASTSupport
  clangIClangFuncX
  ${system_libs}
)