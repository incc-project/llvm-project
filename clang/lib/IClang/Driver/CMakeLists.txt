set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  Support
  ILLVMSupport
  ILLVMFuncVELF
  )

add_clang_library(clangIClangDriver
  Driver.cpp
  DriverBase.cpp
  IncDriver.cpp
  ShareDriver.cpp
  TestDriver.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  clangIClangSupport
  ${system_libs}
  )
