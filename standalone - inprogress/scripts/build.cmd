@echo off
echo Detected Windows
set CC=cl.exe
set CXX=cl.exe

REM Check processor architecture
echo Processor architecture: %PROCESSOR_ARCHITECTURE%
set "vsScript="

echo Running on %PROCESSOR_ARCHITECTURE% architecture
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
   set "vsScript=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if "%PROCESSOR_ARCHITECTURE%" == "ARM64" (
    set "vsScript=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsarm64.bat"
) else (
    echo Unknown architecture: %PROCESSOR_ARCHITECTURE%
    exit /b
)

echo Setting up build environment...
call "%vsScript%"
if errorlevel 1 (
    echo [%vsScript%] failed with errorlevel %errorlevel%
    exit /b %errorlevel%
)

meson setup build --buildtype debug --prefix C:/linux-port/ --reconfigure --wipe
