@echo off
setlocal
cd /d "%~dp0"

where cl >nul 2>&1
if errorlevel 1 (
  echo Run this from "x64 Native Tools Command Prompt for VS" or after vcvars64.bat
  exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /I public /Fe:dist\test_game.exe ^
  private\test_game.cpp private\game.cpp private\i18n.cpp private\levels.cpp
if errorlevel 1 exit /b 1

del /q *.obj 2>nul
dist\test_game.exe
set ERR=%ERRORLEVEL%
echo.
if %ERR%==0 (echo TESTS_OK) else (echo TESTS_FAILED)
exit /b %ERR%
