#!/bin/sh

# Builds the unit tests with code coverage enabled, runs them and generates an HTML coverage report.
# Usage: ./coverage_report_linux.sh [LLVM_VERSION]
# The LLVM version must match the version of clang++ that is used to compile (default: 18).

set -e

LLVM_VERSION=${1:-18}
COMPILER=clang++-$LLVM_VERSION
PROFDATA=llvm-profdata-$LLVM_VERSION
COV=llvm-cov-$LLVM_VERSION
BUILD_DIR=Linux_ReleaseCoverage
REPORT_DIR=Linux_ReleaseCoverage/Report

cd "$(dirname "$0")"

# Generate and build
./cmake_linux_clang_gcc.sh ReleaseCoverage $COMPILER
cmake --build $BUILD_DIR --target UnitTests -- -j$(nproc)

# Run the unit tests, this writes the raw profile
rm -f $BUILD_DIR/UnitTests.profraw $BUILD_DIR/UnitTests.profdata
(cd $BUILD_DIR && LLVM_PROFILE_FILE=UnitTests.profraw ./UnitTests)

# Convert the raw profile and generate the report
$PROFDATA merge -sparse $BUILD_DIR/UnitTests.profraw -o $BUILD_DIR/UnitTests.profdata
rm -rf $REPORT_DIR
$COV show $BUILD_DIR/UnitTests \
	-instr-profile=$BUILD_DIR/UnitTests.profdata \
	-format=html \
	-show-line-counts-or-regions \
	-ignore-filename-regex='(UnitTests|TestFramework)/' \
	-output-dir=$REPORT_DIR
$COV report $BUILD_DIR/UnitTests \
	-instr-profile=$BUILD_DIR/UnitTests.profdata \
	-ignore-filename-regex='(UnitTests|TestFramework)/' \
	| tail -n 1

echo ""
echo "Coverage report written to $(pwd)/$REPORT_DIR/index.html"

xdg-open $(pwd)/$REPORT_DIR/index.html
