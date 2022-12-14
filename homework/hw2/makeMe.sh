#!/bin/bash

function generateCcls() {
    echo "clang
%h -x c++-header
%cpp -std=c++17
%c -std=c17
-Wall
-Wno-narrowing
-Werror
-Wshadow
-pedantic
-Wextra
-I./include
" >.ccls
}

OUT="target/$(pwd | sed -E 's/ /\\ /g' | xargs basename).exe"
CC="$(which g++)"
FLAGS="-O2\
    -g\
    -Wall\
    -Wno-narrowing\
    -Werror\
    -Wshadow\
    -pedantic\
    -Wextra\
    --std=c++17\
    -I "./include"\
    "

SRC="$(find "source/" -name "*.cpp")"

if [ ! -d target/ ]; then
    mkdir target
fi

$CC $FLAGS -o $OUT $SRC

chmod +x $OUT
