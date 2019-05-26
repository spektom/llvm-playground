llvm-playground
================

Playing with LLVM for the purpose of building compile-time optimized Jit.

## Prerequisites

 * LLVM 8
 * CMake 3.12+
 * GCC 8+ or Clang 8+

## Compiling

    mkdir build
    CXX=/usr/local/opt/llvm/bin/clang++ cmake ..
    make VERBOSE=1

