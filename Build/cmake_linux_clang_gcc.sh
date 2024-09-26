#!/bin/sh

if [ -z $1 ]
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
	shift
fi

if [ -z $1 ]
then
	COMPILER=clang++
else
	COMPILER=$1
	shift
fi

BUILD_DIR=Linux_$BUILD_TYPE

echo Usage: ./cmake_linux_clang_gcc.sh [Configuration] [Compiler]
echo "Possible configurations: Debug (default), Release, Distribution, ReleaseUBSAN, ReleaseASAN, ReleaseTSAN, ReleaseCoverage"
echo "Possible compilers: clang++, clang++-XX, g++, g++-XX where XX is the version"
echo Generating Makefile for build type \"$BUILD_TYPE\" and compiler \"$COMPILER\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER "${@}"

echo Compile by running \"make -j $(nproc) \&\& ./UnitTests\" in folder \"$BUILD_DIR\"
