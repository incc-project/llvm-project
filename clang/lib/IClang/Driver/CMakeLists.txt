set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  Support
  )

add_clang_library(iClangDriver
  Driver.cpp
  DriverBase.cpp
  IncDriver.cpp
  ShareDriver.cpp
  TestDriver.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  iClangSupport
  iClangASTSupport
  ILLVMSupport
#  ILLVMFuncVELF
  ${system_libs}
  )
