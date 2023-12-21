@echo off
cmake -S . -B VS2022_CL_CPD -G "Visual Studio 17 2022" -A x64 -DCROSS_PLATFORM_DETERMINISTIC=ON %*
echo Open VS2022_CL_CPD\JoltPhysics.sln to build the project.
