cmake_minimum_required(VERSION 3.10)
project(R-Compiler)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g")

include_directories(include)

file(GLOB SOURCES "src/*.cpp" 
    "src/Lexer/*.cpp"
    "src/Parser/*.cpp" 
    "src/Semantic/*.cpp"
    "main.cpp")

add_executable(main ${SOURCES})