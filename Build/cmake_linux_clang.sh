#!/bin/sh

BUILD_TYPE=$1
if [ -z $1 ] 
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
fi

BUILD_DIR=Linux_$BUILD_TYPE

echo Usage: ./cmake_linux_clang.sh [Configuration]
echo "Possible configurations: Debug (default), Release, Distribution, ReleaseUBSAN, ReleaseASAN, ReleaseCoverage"
echo Generating Makefile for build type \"$BUILD_TYPE\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo Compile by running \"make -j 8 \&\& ./UnitTests\" in folder \"$BUILD_DIR\"