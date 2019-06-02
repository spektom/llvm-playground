cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project(llvm-playground
  LANGUAGES CXX
)

find_package(LLVM 8 REQUIRED)

file(GLOB source_files
  "src/*.cc"
)
add_executable(${CMAKE_PROJECT_NAME}
  ${source_files}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LLVM_INCLUDE_DIR}
)

target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CODEGEN_CXX_FLAGS})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${CODEGEN_CXX_FLAGS})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC LLVM c++fs)
#target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC LLVM stdc++fs)
