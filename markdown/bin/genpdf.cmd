@echo off
setlocal
set SCRIPT_DIR=%~dp0
set PYTHON_SCRIPT=%SCRIPT_DIR%..\src\main.py
set GENPDF_RESOURCE_DIR=%SCRIPT_DIR%..\src\resources

if "%~1"=="" (
  py -3 "%PYTHON_SCRIPT%" --help
) else (
  py -3 "%PYTHON_SCRIPT%" %*
)
