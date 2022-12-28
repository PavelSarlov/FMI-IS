#!/bin/bash
  
PROJECT="$(basename "$(pwd)")"

function generateCMakeLists() {
  echo "cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER \"g++\")
set(CMAKE_C_COMPILER \"gcc\")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wall
  -Wno-narrowing
  -Wno-unused-result
  -Wno-sign-compare
  -Werror
  -Wshadow
  -pedantic)

project(\"$PROJECT\" VERSION 1.0)
aux_source_directory(source SRC_FILES)
add_executable(\"$PROJECT\" \${SRC_FILES})
target_include_directories(\"$PROJECT\" PUBLIC \${CMAKE_CURRENT_SOURCE_DIR}/include/)
" >CMakeLists.txt

  if [[ ! -d build ]]; then
    mkdir build
  fi
  cd build
  cmake -G "MinGW Makefiles" -S ../
  mv compile_commands.json ../
  make
}

function run() {
  if [ -x "./build/$PROJECT" ]; then
    "./build/$PROJECT"
  fi
}

if [[ $1 =~ make ]]; then
  generateCMakeLists
elif [ $# -eq 0 ] || [[ $1 =~ run ]]; then
  run
fi
