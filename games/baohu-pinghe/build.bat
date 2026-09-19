@echo off
setlocal
cd /d "%~dp0"

where cl >nul 2>&1
if errorlevel 1 (
  echo Run this from "x64 Native Tools Command Prompt for VS" or after vcvars64.bat
  exit /b 1
)

if not exist dist mkdir dist

cl /nologo /EHsc /std:c++17 /utf-8 /O2 /MT /I public /Fe:dist\baohu.exe ^
  private\main.cpp private\game.cpp private\i18n.cpp private\levels.cpp user32.lib
if errorlevel 1 exit /b 1

del /q *.obj 2>nul

echo.
echo Built: %cd%\dist\baohu.exe
endlocal
