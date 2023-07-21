#!/bin/sh

if [ -z $1 ]
then
	BUILD_TYPE=Release
else
	BUILD_TYPE=$1
	shift
fi

BUILD_DIR=MinGW_$BUILD_TYPE

echo Usage: ./cmake_linux_mingw.sh [Configuration]
echo "Possible configurations: Debug, Release (default), Distribution"
echo Generating Makefile for build type \"$BUILD_TYPE\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE "${@}"

echo Compile by running \"cmake --build $BUILD_DIR -j 8\"
