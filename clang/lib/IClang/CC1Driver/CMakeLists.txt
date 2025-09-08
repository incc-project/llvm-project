set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  )

add_clang_library(iClangCC1Driver
  CC1DriverBase.cpp
  IncCC1Driver.cpp
  ShareCC1Driver.cpp
  TestCC1Driver.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  iClangSupport
  iClangASTSupport
  iClangFuncX
  ILLVMSupport
  ${system_libs}
)