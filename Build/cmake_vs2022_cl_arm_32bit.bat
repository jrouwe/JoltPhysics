@echo off
cmake -S . -B VS2022_CL_ARM_32BIT -G "Visual Studio 17 2022" -A ARM -DCMAKE_SYSTEM_VERSION="10.0.22621.0" -DCMAKE_CXX_COMPILER_WORKS=1 %*
echo Note that Windows 11 SDK (10.0.22621.0) is the last SDK to support 32-bit ARM, make sure you have it installed.
echo Open VS2022_CL_ARM_32BIT\JoltPhysics.sln to build the project.
