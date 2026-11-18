@echo off
setlocal

cd /d "%~dp0"

echo [1/2] Compiling resources...
windres mfile.rc -O coff -o mfile.res
if errorlevel 1 (
    echo ERROR: windres failed. Make sure MinGW bin is in PATH.
    exit /b 1
)

echo [2/2] Compiling mfile.exe...
g++ -std=c++11 -mwindows -O2 -Wall -static-libgcc -static-libstdc++ mfile.cpp mfile.res -o mfile.exe -lcomctl32 -lshell32 -lcomdlg32 -lole32 -loleaut32 -luuid -lgdi32 -luser32 -luxtheme
if errorlevel 1 (
    echo ERROR: g++ failed.
    del /q mfile.res >nul 2>&1
    exit /b 1
)

del /q mfile.res >nul 2>&1
echo Build successful: %cd%\mfile.exe
exit /b 0
