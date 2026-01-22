cmake_minimum_required(VERSION 3.10)
project(R-Compiler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM 17 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Libraries: ${LLVM_LIBRARIES}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-g")

file(GLOB SOURCES "src/*.cpp" 
    "src/Lexer/*.cpp"
    "src/Parser/*.cpp" 
    "src/Semantic/*.cpp"
    "src/CodeGen/*.cpp"
    "main.cpp")

add_executable(main ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(main ${llvm_libs})
