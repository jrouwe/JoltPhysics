#!/bin/sh

if [ -z $1 ]
then
	COMPILER=clang++
else
	COMPILER=$1
	shift
fi

BUILD_DIR=Ninja_MultiConfig

echo Usage: ./cmake_ninja.sh [Compiler]
echo "Possible compilers: clang++, clang++-XX, g++, g++-XX where XX is the version"
echo Generating Ninja Multi-Config for compiler \"$COMPILER\" in folder \"$BUILD_DIR\"

cmake -S . -B $BUILD_DIR -G "Ninja Multi-Config" -DCMAKE_CXX_COMPILER=$COMPILER "${@}"

echo Compile by running \"cmake --build . --config Debug \&\& ./UnitTests\" in folder \"$BUILD_DIR\"
