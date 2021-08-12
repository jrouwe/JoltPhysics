@echo off
set PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\Llvm\x64\bin\;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\
msbuild VS2019_Clang\JoltPhysics.sln /p:Configuration=ReleaseCoverage
cd VS2019_Clang\ReleaseCoverage
UnitTests.exe
llvm-profdata merge -sparse default.profraw -o default.profdata
cd ..\..
llvm-cov show -format=html -output-dir=CoverageReport .\VS2019_Clang\ReleaseCoverage\UnitTests.exe -instr-profile=.\VS2019_Clang\ReleaseCoverage\default.profdata
CoverageReport\index.html