@echo off
cmake -S . -B VS2026_CL_NO_OBJ_STR -G "Visual Studio 18 2026" -A x64 -DENABLE_OBJECT_STREAM=OFF %*
echo Open VS2026_CL_NO_OBJ_STR\JoltPhysics.slnx to build the project.
