@echo off
echo Detected Windows
set CC=cl.exe
set CXX=cl.exe
meson setup build --buildtype debug --prefix C:/linux-port/ --reconfigure --wipe
