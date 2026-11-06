#!/bin/sh

# Checks that Jolt never uses the global operator new, which would bypass the allocation hooks in Core/Memory.h.
# Every new expression in the Jolt folder, and every new expression anywhere that allocates a class declared in the Jolt folder,
# must resolve to a class operator new (e.g. through JPH_OVERRIDE_NEW_DELETE) or be a placement new.
# Usage: [JOBS=N] ./check_global_new.sh [BUILD_DIR] [CLANG_QUERY]
# BUILD_DIR must contain a compile_commands.json (configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON, default: Linux_Debug).
# CLANG_QUERY is the clang-query executable to use (default: clang-query).
# JOBS is the number of files that are processed in parallel (default: number of CPUs), lower it if you run out of memory.

BUILD_DIR=${1:-Linux_Debug}
CLANG_QUERY=${2:-clang-query}
JOBS=${JOBS:-$(nproc)}

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
	echo "Cannot find $BUILD_DIR/compile_commands.json, configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
	exit 1
fi

# Collect all source files, also the ones outside the Jolt folder as they can allocate Jolt classes
FILES=$(grep -o '"file": *"[^"]*"' "$BUILD_DIR/compile_commands.json" | sed 's/"file": *"\(.*\)"/\1/' | sort -u)
if [ -z "$FILES" ]; then
	echo "No source files found in $BUILD_DIR/compile_commands.json"
	exit 1
fi
echo "Checking $(echo "$FILES" | wc -l) files for use of the global operator new"

# Find new expressions that are not placement new and don't resolve to a class operator new, and that are either in the Jolt folder or allocate a class declared in the Jolt folder
# (expressions in uninstantiated templates are skipped as their operator new is not known yet, the template instantiations are checked instead)
IN_JOLT='isExpansionInFileMatching("[/\\\\]Jolt[/\\\\]")'
export QUERY="match cxxNewExpr(unless(isInstantiationDependent()), unless(hasAnyPlacementArg(anything())), unless(hasDeclaration(cxxMethodDecl())), anyOf($IN_JOLT, hasType(pointsTo(qualType(hasUnqualifiedDesugaredType(recordType(hasDeclaration(cxxRecordDecl($IN_JOLT)))))))))"
export BUILD_DIR
export OUT_DIR=$(mktemp -d)
trap 'rm -rf "$OUT_DIR"' EXIT

# Run one file per clang-query process (clang-query keeps the AST of all files it is given in memory),
# each process appends to its own file so that the output doesn't get interleaved
echo "$FILES" | tr '\n' '\0' | xargs -0 -P "$JOBS" -n 1 sh -c \
	'echo "$1"; "$0" -p "$BUILD_DIR" -c "set output diag" -c "$QUERY" "$1" >> "$OUT_DIR/out.$$" 2>&1' "$CLANG_QUERY"

# Matches in headers are reported once per translation unit, so remove duplicates
MATCHES=$(cat "$OUT_DIR"/out.* | grep -E 'note: "root" binds here|error' | sort -u)

if [ -n "$MATCHES" ]; then
	echo "$MATCHES"
	echo ""
	echo "The new expressions above use the global operator new. Add JPH_OVERRIDE_NEW_DELETE to the class."
	exit 1
fi

echo "No use of the global operator new found"
