@echo off
setlocal
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
cd /d "%~dp0"
cmake --preset default
if errorlevel 1 exit /b %errorlevel%
cmake --build build\intermediate --config %CONFIG% --parallel
if errorlevel 1 exit /b %errorlevel%
echo.
echo Built: %~dp0build\bin\%CONFIG%\loader.gg.exe
