@echo off
setlocal
cd /d "%~dp0"

if "%~1"=="" (
  echo Usage: debug_level.bat ^<1^|2^|3^> [zh^|en]
  echo Example: debug_level.bat 2
  echo Example: debug_level.bat 3 en
  exit /b 1
)

if not exist dist\baohu.exe (
  echo dist\baohu.exe missing. Run build.bat first.
  exit /b 1
)

set LANG=zh
if not "%~2"=="" set LANG=%~2

dist\baohu.exe --level %~1 --lang %LANG%
endlocal
