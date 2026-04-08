@echo off
title Image Uploader Build Tool By Warwick

echo ================================
echo  Image Uploader Build Tool By Warwick
echo ================================
echo.

REM ===== Python check =====
python --version >nul 2>&1
if errorlevel 1 (
    echo [!] Python not found
    pause
    exit /b
)

REM ===== Dependency check =====
echo [*] Checking Python dependencies...

python -c "import serial" >nul 2>&1 || (
    echo [!] PySerial not installed
    echo     Run: python -m pip install pyserial
    pause
    exit /b
)

python -c "import PyInstaller" >nul 2>&1 || (
    echo [!] PyInstaller not installed
    echo     Run: python -m pip install pyinstaller
    pause
    exit /b
)

python -c "import PyQt5" >nul 2>&1 || (
    echo [!] PyQt5 not installed
    echo     Run: python -m pip install pyqt5
    pause
    exit /b
)

echo [+] All dependencies OK
echo.

REM ===== Select Target =====
echo Select build target:
echo.
echo  [1] ESP32-C5
echo  [2] BW16 (RTL8720DN)
echo.

set /p TARGET=Enter choice (1 or 2): 

if "%TARGET%"=="1" goto BUILD_ESP32C5
if "%TARGET%"=="2" goto BUILD_BW16

echo.
echo [!] Invalid selection
pause
exit /b


REM ================================
REM ESP32-C5 BUILD
REM ================================
:BUILD_ESP32C5
echo.
echo [*] Target: ESP32-C5

set NAME=image_uploader_esp32c5
set ENTRY=esp32main.py
set ADDDATA=--add-data "firmware;firmware"
set DIST=dist_esp32c5

goto BUILD_COMMON


REM ================================
REM BW16 BUILD
REM ================================
:BUILD_BW16
echo.
echo [*] Target: BW16 (RTL8720DN)

set NAME=image_uploader_bw16
set ENTRY=bw16main.py
set ADDDATA=--add-data "realtek_tools;realtek_tools"
set DIST=dist_bw16

goto BUILD_COMMON


REM ================================
REM COMMON BUILD
REM ================================
:BUILD_COMMON
echo.
echo [*] Cleaning old build...

rmdir /s /q build >nul 2>&1
rmdir /s /q "%DIST%" >nul 2>&1
del /q "%NAME%.spec" >nul 2>&1

echo.
echo [*] Building EXE...

python -m PyInstaller ^
 --onefile ^
 --noconsole ^
 --name "%NAME%" ^
 --icon input.ico ^
 --distpath "%DIST%" ^
 %ADDDATA% ^
 "%ENTRY%"

if errorlevel 1 (
    echo.
    echo [!] Build failed
    pause
    exit /b
)

echo.
echo [+] Build success!
echo Output: %DIST%\%NAME%.exe
echo.
pause
