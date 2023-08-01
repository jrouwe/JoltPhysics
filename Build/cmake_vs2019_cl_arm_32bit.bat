@echo off
cmake -S . -B VS2019_CL_ARM_32BIT -G "Visual Studio 16 2019" -A ARM %*
echo Open VS2019_CL_ARM_32BIT\JoltPhysics.sln to build the project.
