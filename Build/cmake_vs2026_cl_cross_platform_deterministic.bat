@echo off
cmake -S . -B VS2026_CL_CPD -G "Visual Studio 18 2026" -A x64 -DCROSS_PLATFORM_DETERMINISTIC=ON %*
echo Open VS2026_CL_CPD\JoltPhysics.sln to build the project.
