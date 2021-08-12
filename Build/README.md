# Building Jolt Physics

## Windows 10+ (CL - Default compiler)

- Download Visual Studio 2019+ (Community or other edition)
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2019_cl.bat
- Open the resulting project file VS2019_CL\JoltPhysics.sln
- Compile and run either 'Samples' or 'UnitTests'

## Windows 10+ (Clang compiler)

- Download Visual Studio 2019+ (Community or other edition)
- Make sure to install "C++ Clang Compiler for Windows 11.0.0+" and "C++ Clang-cl for v142+ build tools (x64/x86)" using the Visual Studio Installer
- Download CMake 3.15+ (https://cmake.org/download/)
- Run cmake_vs2019_clang.bat
- Open the resulting project file VS2019_Clang\JoltPhysics.sln
- Compile and run either 'Samples' or 'UnitTests'

## Linux (Debian flavor, x64 or ARM64)

- Install clang (apt-get install clang)
- Install cmake (apt-get install cmake)
- Run: ./cmake_vs2019_cl.bat
- Go to the Linux_Debug folder
- Run: make -j 8 && ./UnitTests

## Android

- Install Android Studio 2020.3.1+ (https://developer.android.com/studio/)
- Open the 'Android' folder in Android Studio and wait until gradle finishes
- Select 'Run' / 'Run...' and 'UnitTests'
- If the screen turns green after a while the unit tests succeeded, when red they failed (see the android log for details)