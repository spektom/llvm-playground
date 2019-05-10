cmake_minimum_required(VERSION 3.10)

project(llvm-playground)

message("")
message("Host")

if(WIN32)
  set(rtti_flags /GR- /EHs- /EHc-)
  set(cxx_flags /std:c++17)
else()
  set(rtti_flags -fno-rtti -fno-exceptions)
  set(cxx_flags -std=c++17)
endif()

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Arch: ${CMAKE_SYSTEM_PROCESSOR}")

message("")
message("Looking for LLVM")

if(LLVM_DIR)
  file(TO_CMAKE_PATH ${LLVM_DIR} LLVM_DIR)
  message(STATUS "Configure with explicit LLVM_DIR=${LLVM_DIR}")
else()
  message(WARNING
    "No explicit LLVM_DIR provided. If find_package() fails, pass "
    "-DLLVM_DIR=/path/to/llvm-build/lib/cmake/llvm to CMake.")
endif()

find_package(LLVM 8.0 REQUIRED)
message(STATUS "Package version: ${LLVM_PACKAGE_VERSION}")

if(LLVM_BUILD_MAIN_SRC_DIR)
  message(STATUS "Source directory: ${LLVM_BUILD_MAIN_SRC_DIR}")
  message(STATUS "Build directory: ${LLVM_BINARY_DIR}")
else()
  message(STATUS "Install directory: ${LLVM_BINARY_DIR}")
endif()

message(STATUS "Build type: ${LLVM_BUILD_TYPE}")

if(LLVM_ENABLE_RTTI)
  message(WARNING "LLVM built with RTTI; enabling it for ${CMAKE_PROJECT_NAME} to avoid quirks")
  set(rtti_flags) # Clearing the default flags enables RTTI.
endif()

# Import extra CMake helpers from LLVM
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)

message("")
message("Configure TARGET ${CMAKE_PROJECT_NAME}")

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No explicit CMAKE_BUILD_TYPE provided; ${CMAKE_PROJECT_NAME} defaults to the build type of LLVM")
  set(CMAKE_BUILD_TYPE ${LLVM_BUILD_TYPE} CACHE STRING "" FORCE)
elseif(NOT ${LLVM_BUILD_TYPE} STREQUAL ${CMAKE_BUILD_TYPE})
  message(WARNING "${CMAKE_PROJECT_NAME} set to build as ${CMAKE_BUILD_TYPE}, which differs from LLVM's build type")
endif()

add_executable(${CMAKE_PROJECT_NAME}
  main.cc
  Jit.cc
  Codegen.cc
  Optimizer.cc
  ObjectCache.cc
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LLVM_INCLUDE_DIR}
)

# Resolve static libraries from component names.
llvm_map_components_to_libnames(llvm_libs
  core
  executionengine
  ipo
  native
  orcjit
  runtimedyld
  support
  transformutils
)

separate_arguments(LLVM_DEFINITIONS)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${llvm_libs})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${cxx_flags} ${rtti_flags})

# Allow dlopen() to access exported symbols.
# This will add linker flags like -Wl,--export-dynamic -rdynamic
export_executable_symbols(${CMAKE_PROJECT_NAME})

function(dump_target_properties target property)
  get_target_property(values ${target} ${property})
  if(values)
    if(NOT "${values}" STREQUAL "${property}-NOTFOUND")
      message(STATUS "${property}=${values}")
    endif()
  endif()
endfunction()

dump_target_properties(${CMAKE_PROJECT_NAME} INCLUDE_DIRECTORIES)
dump_target_properties(${CMAKE_PROJECT_NAME} COMPILE_OPTIONS)
dump_target_properties(${CMAKE_PROJECT_NAME} COMPILE_DEFINITIONS)
dump_target_properties(${CMAKE_PROJECT_NAME} LINK_LIBRARIES)
dump_target_properties(${CMAKE_PROJECT_NAME} LINK_FLAGS)
