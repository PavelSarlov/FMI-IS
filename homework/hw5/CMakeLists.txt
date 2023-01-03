cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wall
  -Wno-narrowing
  -Wno-unused-result
  -Wno-sign-compare
  -Werror
  -Wshadow
  -pedantic)

project("hw5" VERSION 1.0)
aux_source_directory(source SRC_FILES)
add_executable("hw5" ${SRC_FILES})
target_include_directories("hw5" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

