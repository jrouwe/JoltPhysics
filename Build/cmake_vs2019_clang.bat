@echo off
cmake -S . -B VS2019_Clang -G "Visual Studio 16 2019" -A x64 -T ClangCL
echo:
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Make sure to install:
echo - C++ Clang Compiler for Windows 11.0.0+
echo - C++ Clang-cl for v142+ build tools (x64/x86)
echo Using the Visual Studio Installer
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Open VS2019_Clang/JoltPhysics.sln to build the project.