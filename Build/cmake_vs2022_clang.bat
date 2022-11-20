@echo off
cmake -S . -B VS2022_Clang -G "Visual Studio 17 2022" -A x64 -T ClangCL %*
echo:
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Make sure to install:
echo - C++ Clang Compiler for Windows 12.0.0+
echo - C++ Clang-cl for v143+ build tools (x64/x86)
echo Using the Visual Studio Installer
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Open VS2022_Clang/JoltPhysics.sln to build the project.