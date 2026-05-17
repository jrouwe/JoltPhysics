@echo off
cmake -S . -B VS2026_CL_ARM -G "Visual Studio 18 2026" -A ARM64 %*
echo Open VS2026_CL_ARM\JoltPhysics.slnx to build the project.
