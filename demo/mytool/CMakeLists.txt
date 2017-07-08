set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_executable(mytool
  MyTool.cpp
  )

target_link_libraries(mytool
  clangAST
  clangBasic
  clangFrontend
  clangTooling
  )

