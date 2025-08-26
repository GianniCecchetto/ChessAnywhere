@echo off
REM ===== Build board_com.dll with MinGW =====
REM Usage: build.bat

set OUTDIR=build
set DLL=%OUTDIR%\board_com.dll

if not exist %OUTDIR% mkdir %OUTDIR%

gcc -O2 -Wall -shared -o %DLL% board_com.c

if errorlevel 1 (
  echo [ERROR] Build failed
  exit /b 1
)

echo [OK] Built %DLL%
