@echo off
cmake -S . -B VS2019_CL_ARM -G "Visual Studio 16 2019" -A ARM64 %*
echo Open VS2019_CL_ARM\JoltPhysics.sln to build the project.
