set(LLVM_LINK_COMPONENTS
  Option
  ProfileData
  Support
  )

add_clang_library(iClangSupport
  Config.cpp
  Global.cpp
  MetaData.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  ILLVMSupport
  ${system_libs}
)