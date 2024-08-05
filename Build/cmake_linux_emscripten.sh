#!/bin/sh

if [ -z $1 ]
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
	shift
fi

BUILD_DIR=WASM_$BUILD_TYPE

echo Usage: ./cmake_linux_emscripten.sh [Configuration]
echo "Possible configurations: Debug (default), Release, Distribution"
echo Generating Makefile for build type \"$BUILD_TYPE\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake "${@}"

echo Compile by running \"make -j $(nproc) \&\& node UnitTests.js\" in folder \"$BUILD_DIR\"
