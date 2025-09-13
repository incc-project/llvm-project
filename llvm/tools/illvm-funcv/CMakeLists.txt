set(LLVM_LINK_COMPONENTS
  MC
  MCParser
  Support
  TargetParser
)

add_llvm_tool(illvm-funcv
  main.cpp

  DEPENDS
  intrinsics_gen
)
