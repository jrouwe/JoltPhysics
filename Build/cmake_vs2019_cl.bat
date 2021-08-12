@echo off
cmake -S . -B VS2019_CL -G "Visual Studio 16 2019" -A x64
echo Open VS2019_CL\JoltPhysics.sln to build the project.