@echo off
cmake -S . -B VS2022_CL_ARM -G "Visual Studio 17 2022" -A ARM64 %*
echo Open VS2022_CL_ARM\JoltPhysics.sln to build the project.
