@echo off
cmake -S . -B VS2026_Clang_Double -G "Visual Studio 18 2026" -A x64 -T ClangCL -DDOUBLE_PRECISION=YES %*
echo:
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Make sure to install:
echo - C++ Clang Compiler for Windows 20+
echo - MSBuild support for LLVM (clang-cl) toolset
echo Using the Visual Studio Installer
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Open VS2026_Clang_Double/JoltPhysics.sln to build the project.
