@echo off
del %~dp0%Build\Doxygen /s /q
doxygen
copy Build\Doxygen\JoltPhysics.chm Docs\
pause