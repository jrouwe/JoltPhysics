@echo off
cmake -S . -B VS2026_CL_Double -G "Visual Studio 18 2026" -A x64 -DDOUBLE_PRECISION=ON %*
echo Open VS2026_CL_Double\JoltPhysics.slnx to build the project.
