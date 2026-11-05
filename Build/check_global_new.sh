#!/bin/sh

# Checks that no code in the Jolt folder uses the global operator new, which would bypass the allocation hooks in Core/Memory.h.
# Every new expression in Jolt must resolve to a class operator new (e.g. through JPH_OVERRIDE_NEW_DELETE) or be a placement new.
# Usage: ./check_global_new.sh [BUILD_DIR] [CLANG_QUERY]
# BUILD_DIR must contain a compile_commands.json (configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON, default: Linux_Debug).
# CLANG_QUERY is the clang-query executable to use (default: clang-query).

BUILD_DIR=${1:-Linux_Debug}
CLANG_QUERY=${2:-clang-query}

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
	echo "Cannot find $BUILD_DIR/compile_commands.json, configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
	exit 1
fi

# Collect all source files of the Jolt library
FILES=$(grep -o '"file": *"[^"]*"' "$BUILD_DIR/compile_commands.json" | sed 's/"file": *"\(.*\)"/\1/' | grep -E '[/\\]Jolt[/\\]' | sort -u)
if [ -z "$FILES" ]; then
	echo "No Jolt source files found in $BUILD_DIR/compile_commands.json"
	exit 1
fi
echo "Checking $(echo "$FILES" | wc -l) files for use of the global operator new"

# Find new expressions in the Jolt folder that are not placement new and don't resolve to a class operator new
export QUERY='match cxxNewExpr(isExpansionInFileMatching("[/\\\\]Jolt[/\\\\]"), unless(hasAnyPlacementArg(anything())), unless(hasDeclaration(cxxMethodDecl())))'
export BUILD_DIR
export OUT_DIR=$(mktemp -d)
trap 'rm -rf "$OUT_DIR"' EXIT

# Run in parallel batches, each batch writes to its own file so that the output doesn't get interleaved
echo "$FILES" | tr '\n' '\0' | xargs -0 -P "$(nproc)" -n 16 sh -c \
	'"$0" -p "$BUILD_DIR" -c "set output diag" -c "$QUERY" "$@" > "$OUT_DIR/out.$$" 2>&1' "$CLANG_QUERY"

# Matches in headers are reported once per translation unit, so remove duplicates
MATCHES=$(cat "$OUT_DIR"/out.* | grep -E 'note: "root" binds here|error' | sort -u)

if [ -n "$MATCHES" ]; then
	echo "$MATCHES"
	echo ""
	echo "The new expressions above use the global operator new. Add JPH_OVERRIDE_NEW_DELETE to the class."
	exit 1
fi

echo "No use of the global operator new found"
