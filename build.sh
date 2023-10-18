#!/bin/bash

mkdir -p ./build
pushd ./build
c++ ../code/sdl_handmade.cpp -o handmadehero `sdl2-config --cflags --libs`
popd
