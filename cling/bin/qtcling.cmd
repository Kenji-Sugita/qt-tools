@echo off
setlocal

set "QTCLING_DEV_ROOT=%~dp0.."
if not defined QTCLING_CLING_ROOT (
  if exist "%QTCLING_DEV_ROOT%\bin\cling.exe" (
    set "QTCLING_CLING_ROOT=%QTCLING_DEV_ROOT%"
  ) else (
    set "QTCLING_CLING_ROOT=%QTCLING_DEV_ROOT%\qtcling-win-arm64"
  )
)
if not defined QTCLING_QT_ROOT set "QTCLING_QT_ROOT=C:\Qt\6.11.1\msvc2022_arm64"
if not defined QTCLING_STARTUP_FILE set "QTCLING_STARTUP_FILE=%QTCLING_DEV_ROOT%\src\qtguiwin-startup.cpp"

if not exist "%QTCLING_CLING_ROOT%\bin\cling.exe" (
  echo qtcling: cling.exe not found: %QTCLING_CLING_ROOT%\bin\cling.exe 1>&2
  exit /b 1
)

if not exist "%QTCLING_QT_ROOT%\include\QtCore" (
  echo qtcling: Qt ARM64 headers not found under: %QTCLING_QT_ROOT% 1>&2
  exit /b 1
)

set "PATH=%QTCLING_QT_ROOT%\bin;%PATH%"
set "QT_PLUGIN_PATH=%QTCLING_QT_ROOT%\plugins"
if exist "%QTCLING_DEV_ROOT%\share\libedit\termcap" (
  set "LIBEDIT_TERMCAP=%QTCLING_DEV_ROOT%\share\libedit\termcap"
) else (
  set "LIBEDIT_TERMCAP=%QTCLING_DEV_ROOT%\libedit-win-arm64\share\libedit\termcap"
)

"%QTCLING_CLING_ROOT%\bin\cling.exe" -std=c++2c ^
  -I "%QTCLING_QT_ROOT%\include" ^
  -I "%QTCLING_QT_ROOT%\include\QtCore" ^
  -I "%QTCLING_QT_ROOT%\include\QtGui" ^
  -I "%QTCLING_QT_ROOT%\include\QtWidgets" ^
  -L "%QTCLING_QT_ROOT%\bin" ^
  -L "%QTCLING_DEV_ROOT%\bin" ^
  -lqtcling-win.dll -lQt6Core.dll -lQt6Gui.dll -lQt6Widgets.dll %*

exit /b %ERRORLEVEL%
