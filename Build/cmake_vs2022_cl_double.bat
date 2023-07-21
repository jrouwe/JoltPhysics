@echo off
cmake -S . -B VS2022_CL_Double -G "Visual Studio 17 2022" -A x64 -DDOUBLE_PRECISION=ON %*
echo Open VS2022_CL_Double\JoltPhysics.sln to build the project.
