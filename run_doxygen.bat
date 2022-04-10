@echo off
del %~dp0%Build\Doxygen /s /q > NUL
doxygen
pause