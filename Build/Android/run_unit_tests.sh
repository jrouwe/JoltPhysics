#!/bin/bash
# Installs the unit test APK on the connected device / emulator, runs it and fails (exit code 1) if the unit tests failed.
# Usage: run_unit_tests.sh <apk or directory to search for UnitTests-debug.apk> [timeout in seconds]

APK="$1"
TIMEOUT="${2:-900}"

# If a directory is given, search it for the debug APK (its location differs between Gradle tasks / versions)
if [ -d "$APK" ]; then
	APK=$(find "$APK" -name "UnitTests-debug.apk" | head -n 1)
fi

if [ -z "$APK" ] || [ ! -f "$APK" ]; then
	echo "APK not found. Usage: $0 <apk or directory> [timeout in seconds]"
	exit 2
fi

adb install -t -r "$APK" || exit 2
adb logcat -c
adb shell am start -n com.joltphysics.unittests/android.app.NativeActivity || exit 2

# Wait for the app to report its result
RESULT=""
for ((i = 0; i < TIMEOUT; i++)); do
	RESULT=$(adb logcat -d -s Jolt:I | grep -F "UNIT_TESTS_RESULT:" | tail -n 1)
	[ -n "$RESULT" ] && break
	sleep 1
done

# Show the full log of the test run
adb logcat -d -s Jolt:I

if [ -z "$RESULT" ]; then
	echo "Unit tests did not finish within $TIMEOUT seconds (or crashed)"
	adb logcat -d -s AndroidRuntime:E DEBUG:I libc:F
	exit 1
fi

if [[ "$RESULT" == *"PASSED"* ]]; then
	echo "Unit tests passed"
	exit 0
fi

echo "::error::Android unit tests failed"
exit 1
