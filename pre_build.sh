#!/usr/bin/env bash

ROOT=$(pwd)

rm -rf build
mkdir build
mkdir build/googletest
mkdir pre_build/googletest
cd build/googletest
cmake ../../submodules/googletest
make
cp -r lib ${ROOT}/pre_build/googletest
cp -r ${ROOT}/submodules/googletest/googletest/include ${ROOT}/pre_build/googletest






