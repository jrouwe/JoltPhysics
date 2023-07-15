# Building and Using Jolt Physics

## Build Types

Each platform supports multiple build targets

- Debug - Debug version of the library, turns on asserts
- Release - Release version of the library, no asserts but includes profiling support and can draw the world and simulation properties
- ReleaseASAN - As Release but turns on Address Sanitizer (clang only) to find bugs
- ReleaseUBSAN - As Release but turns on Undefined Behavior Sanitizer (clang only) to find bugs
- ReleaseCoverage - As Release but turns on Coverage reporting (clang only) to find which areas of the code are not executed
- Distribution - Shippable version of the library, turns off all debugging support

## Includes

The Jolt headers don't include Jolt/Jolt.h. Always include Jolt/Jolt.h before including any other Jolt header.
You can use Jolt/Jolt.h in your precompiled header to speed up compilation.

## Defines

There are a number of user configurable defines that turn on/off certain features:

- JPH_PROFILE_ENABLED - Turns on the internal profiler.
- JPH_EXTERNAL_PROFILE - Turns on the internal profiler but forwards the information to a user defined external system (see Profiler.h).
- JPH_DEBUG_RENDERER - Adds support to draw lines and triangles, used to be able to debug draw the state of the world.
- JPH_DISABLE_TEMP_ALLOCATOR - Disables the temporary memory allocator, used mainly to allow ASAN to do its job.
- JPH_DISABLE_CUSTOM_ALLOCATOR - Disables the ability to override the memory allocator.
- JPH_FLOATING_POINT_EXCEPTIONS_ENABLED - Turns on division by zero and invalid floating point exception support in order to detect bugs (Windows only).
- JPH_CROSS_PLATFORM_DETERMINISTIC - Turns on behavior to attempt cross platform determinism. If this is set, JPH_USE_FMADD is ignored.
- JPH_DOUBLE_PRECISION - Compiles the library so that all positions are stored in doubles instead of floats. This makes larger worlds possible.
- JPH_USE_SSE4_1 - Enable SSE4.1 CPU instructions (x86/x64 only)
- JPH_USE_SSE4_2 - Enable SSE4.2 CPU instructions (x86/x64 only)
- JPH_USE_F16C - Enable half float CPU instructions (x86/x64 only)
- JPH_USE_LZCNT - Enable the lzcnt CPU instruction (x86/x64 only)
- JPH_USE_TZCNT - Enable the tzcnt CPU instruction (x86/x64 only)
- JPH_USE_AVX - Enable AVX CPU instructions (x86/x64 only)
- JPH_USE_AVX2 - Enable AVX2 CPU instructions (x86/x64 only)
- JPH_USE_AVX512 - Enable AVX512F+AVX512VL CPU instructions (x86/x64 only)
- JPH_USE_FMADD - Enable fused multiply add CPU instructions (x86/x64 only)

## Logging & Asserting

To override the default trace and assert mechanism install your own custom handlers in Trace and AssertFailed (see IssueReporting.h).

## Custom Memory Allocator

To implement your custom memory allocator override Allocate, Free, AlignedAllocate and AlignedFree (see Memory.h).

## Building

### Windows 10+ (CL - Default compiler)

- Download Visual Studio 2022 (Community or other edition)
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2022_cl.bat
- Open the resulting project file VS2022_CL\JoltPhysics.sln
- Compile and run either 'Samples' or 'UnitTests'

### Windows 10+ (CL - 32 bit)

- Download Visual Studio 2022 (Community or other edition)
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2022_cl_32bit.bat
- Open the resulting project file VS2022_CL_32BIT\JoltPhysics.sln
- Compile and run either 'Samples' or 'UnitTests'

### Windows 10+ (Clang compiler)

- Download Visual Studio 2022 (Community or other edition)
- Make sure to install "C++ Clang Compiler for Windows 11.0.0+" and "C++ Clang-cl for v142+ build tools (x64/x86)" using the Visual Studio Installer
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2022_clang.bat
- Open the resulting project file VS2022_Clang\JoltPhysics.sln
- Compile and run either 'Samples' or 'UnitTests'

### Windows 10+ (Universal Windows Platform)

- Download Visual Studio 2022+ (Community or other edition)
- Make sure to install "Universal Windows Platform development" using the Visual Studio Installer
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2022_uwp.bat
- Open the resulting project file VS2022_UWP\JoltPhysics.sln
- Compile and run 'UnitTests'

### Windows 10+ (MinGW)

- Follow download instructions for MSYS2 (https://www.msys2.org/)
- From the MSYS2 MSYS app run: pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
- From the MSYS2 MINGW x64 app, in the Build folder run: ./cmake_windows_mingw.sh
- Run: cmake --build MinGW_Debug
- Run: MinGW_Debug/UnitTests.exe

### Linux (Debian flavor, x64 or ARM64)

- Install clang (apt-get install clang)
- Install cmake (apt-get install cmake)
- Run: ./cmake_linux_clang_gcc.sh
- Go to the Linux_Debug folder
- Run: make -j$(nproc) && ./UnitTests

### Linux (Debian flavor, MinGW Cross Compile)

- This setup can be used to run samples on Linux using wine and vkd3d. Tested on Ubuntu 22.04
- Graphics card must support Vulkan and related drivers must be installed
- Install mingw-w64 (apt-get install mingw-w64)
- Run: update-alternatives --config x86_64-w64-mingw32-g++ (Select /usr/bin/x86_64-w64-mingw32-g++-posix)
- Install cmake (apt-get install cmake)
- Install wine64 (apt-get install wine64)
- Run: export WINEPATH="/usr/x86_64-w64-mingw32/lib;/usr/lib/gcc/x86_64-w64-mingw32/10-posix" (change it based on your environment)
- Run: ./cmake_linux_mingw.sh Release (Debug doesn't work)
- Go to the MinGW_Release folder
- Run: make -j$(nproc) && wine UnitTests.exe
- Run: wine Samples.exe

### Android

- Install Android Studio 2020.3.1+ (https://developer.android.com/studio/)
- Open the 'Android' folder in Android Studio and wait until gradle finishes
- Select 'Run' / 'Run...' and 'UnitTests'
- If the screen turns green after a while the unit tests succeeded, when red they failed (see the android log for details)

### macOS

- Install XCode
- Download CMake 3.23+ (https://cmake.org/download/)
- Run: ./cmake_xcode_macos.sh
- This will open XCode with a newly generated project
- Build and run the project

Note that you can also follow the steps in the 'Linux' section if you wish to build without XCode.

### iOS

- Install XCode
- Download CMake 3.23+ (https://cmake.org/download/)
- Run: ./cmake_xcode.ios.sh
- This will open XCode with a newly generated project
- Build and run the project (note that this will only work in the simulator as the code signing information is not set up)

## Link Errors

If you receive the following error when linking:

```
/usr/bin/ld: libJolt.a: error adding symbols: file format not recognized
```

Then you have not enabled interprocedural optimizations (link time optimizations) for your own application. See the INTERPROCEDURAL_OPTIMIZATION option in CMakeLists.txt.

## Doxygen on Windows

Documentation can be generated through doxygen:

- Install Doxygen (https://www.doxygen.nl/download.html)
- Run: run_doxygen.bat
