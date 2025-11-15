@echo off
cmake -S . -B VS2026_CL -G "Visual Studio 18 2026" -A x64 %*
echo Open VS2026_CL\JoltPhysics.slnx to build the project.
