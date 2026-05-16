@echo off
cmake -S . -B VS2026_CL_ARM_EC -G "Visual Studio 18 2026" -A ARM64EC %*
echo Open VS2026_CL_ARM_EC\JoltPhysics.slnx to build the project.