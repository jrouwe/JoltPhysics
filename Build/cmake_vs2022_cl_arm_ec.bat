@echo off
cmake -S . -B VS2022_CL_ARM_EC -G "Visual Studio 17 2022" -A ARM64EC %*
echo Open VS2022_CL_ARM_EC\JoltPhysics.sln to build the project.