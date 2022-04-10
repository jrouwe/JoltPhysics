@echo off
del %~dp0%Build\Doxygen /s /q
doxygen
pause