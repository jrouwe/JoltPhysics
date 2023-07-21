#!/bin/sh

if [ -z $1 ]
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
	shift
fi

BUILD_DIR=MinGW_$BUILD_TYPE

echo Usage: ./cmake_windows_mingw.sh [Configuration]
echo "Possible configurations: Debug (default), Release, Distribution"
echo Generating Makefile for build type \"$BUILD_TYPE\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE "${@}"

echo Compile by running \"cmake --build $BUILD_DIR -j 8\"
