@echo off
cmake -S . -B VS2026_CL_Double_CPD -G "Visual Studio 18 2026" -A x64 -DDOUBLE_PRECISION=ON -DCROSS_PLATFORM_DETERMINISTIC=ON %*
echo Open VS2026_CL_Double_CPD\JoltPhysics.slnx to build the project.
