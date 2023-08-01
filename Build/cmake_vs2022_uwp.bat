@echo off
cmake -S . -B VS2022_UWP -G "Visual Studio 17 2022" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 %*
echo If cmake failed then be sure to check the "Universal Windows Platform development" checkbox in the Visual Studio Installer
echo Open VS2022_UWP\JoltPhysics.sln to build the project.
echo Note that none of the sample applications are available for the Universal Windows Platform (use cmake_vs2022_cl.bat instead).
