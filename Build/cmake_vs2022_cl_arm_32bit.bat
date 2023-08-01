@echo off
cmake -S . -B VS2022_CL_ARM_32BIT -G "Visual Studio 17 2022" -A ARM %*
echo Open VS2022_CL_ARM_32BIT\JoltPhysics.sln to build the project.
