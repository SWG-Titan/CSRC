@echo off
REM Sync client/datasources/ui to exe UI folders for parity.
REM Run from titan root or client folder.

set SRC=%~dp0..\datasources\ui
set EXE1=%~dp0..\..\exe\win32_rel\ui
set EXE2=%~dp0..\..\exe\tre\ux\ui

if not exist "%SRC%" (
  echo ERROR: Source not found: %SRC%
  exit /b 1
)

echo Syncing UI from %SRC%
if exist "%EXE1%" (
  xcopy /Y /Q "%SRC%\*.*" "%EXE1%\" >nul 2>&1
  echo   -> %EXE1%
)
if exist "%EXE2%" (
  xcopy /Y /Q "%SRC%\*.*" "%EXE2%\" >nul 2>&1
  echo   -> %EXE2%
)
echo Done.
