#!/bin/sh

if [ -z $1 ] 
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
fi

if [ -z $2 ] 
then
	COMPILER=clang++
else
	COMPILER=$2
fi

BUILD_DIR=Linux_$BUILD_TYPE

echo Usage: ./cmake_linux_clang_gcc.sh [Configuration] [Compiler]
echo "Possible configurations: Debug (default), Release, Distribution, ReleaseUBSAN, ReleaseASAN, ReleaseCoverage"
echo "Possible compilers: clang++, clang++-XX, g++, g++-XX where XX is the version"
echo Generating Makefile for build type \"$BUILD_TYPE\" and compiler \"$COMPILER\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER

echo Compile by running \"make -j 8 \&\& ./UnitTests\" in folder \"$BUILD_DIR\"