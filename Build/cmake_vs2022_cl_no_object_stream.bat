@echo off
cmake -S . -B VS2022_CL_NO_OBJ_STR -G "Visual Studio 17 2022" -A x64 -DENABLE_OBJECT_STREAM=OFF %*
echo Open VS2022_CL_NO_OBJ_STR\JoltPhysics.sln to build the project.
