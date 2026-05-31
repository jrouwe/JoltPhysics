#!/bin/sh

BUILD_TYPE=""
COMPILER=""
SHARED_LIB=false
POSTFIX_DEBUG=false
ENV_SCRIPT=""

while [ $# -gt 0 ]; do
  case "$1" in
    --shared)
      SHARED_LIB=true
      shift
      ;;
    --postfix-debug)
      POSTFIX_DEBUG=true
      shift
      ;;
    --env-script=*)
      ENV_SCRIPT="${1#*=}"
      shift
      ;;
    *)
      if [ -z "$BUILD_TYPE" ]; then
        BUILD_TYPE="$1"
      elif [ -z "$COMPILER" ]; then
        COMPILER="$1"
      else
        break
      fi
      shift
      ;;
  esac
done

if [ -z "$BUILD_TYPE" ]; then
  echo "No build type argument, using default BUILD_TYPE=Debug"
  BUILD_TYPE="Debug"
fi
if [ -z "$COMPILER" ]; then
  echo "No compiler argument, using default COMPILER=clang++"
  COMPILER="clang++"
fi

echo ""
echo "Usage: ./cmake_linux_clang_gcc.sh [BUILD_TYPE] [COMPILER] [--shared] [--postfix-debug] [--env-script=...]"
echo "Build type options: Debug, Release, Distribution, ReleaseUBSAN, ReleaseASAN, ReleaseTSAN, ReleaseCoverage"
echo "Compiler options: clang++, clang++-XX, g++, g++-XX where XX is the version"
echo ""

if [ -n "$ENV_SCRIPT" ]; then
  . "$ENV_SCRIPT"
fi

BUILD_DIR=Linux_$BUILD_TYPE
POSTFIX_ARG=""
SHARED_ARG=""
if [ "$BUILD_TYPE" = "Debug" ] && [ "$POSTFIX_DEBUG" = true ]; then
  POSTFIX_ARG="-DCMAKE_DEBUG_POSTFIX=-d"
fi
if [ "$SHARED_LIB" = true ]; then
  BUILD_DIR=${BUILD_DIR}_shared
  SHARED_ARG="-DBUILD_SHARED_LIBS=ON"
fi

echo Generating Makefile for build type \"$BUILD_TYPE\" and compiler \"$COMPILER\" in folder \"$BUILD_DIR\"
cmake -S . -B $BUILD_DIR -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_CXX_COMPILER=$COMPILER \
  "$@" $POSTFIX_ARG $SHARED_ARG

  echo ""
  echo Compile by running:
  echo "cd ${BUILD_DIR} && make -j$(nproc)"
  echo Run tests: ./UnitTests
  echo ""