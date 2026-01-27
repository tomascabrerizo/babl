#!/bin/bash

mkdir -p ./build

clang -g -O0 src/main.c -o ./build/babl -I/usr/include/freetype2 -lfreetype -lSDL2

