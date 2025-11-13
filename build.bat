@echo off
REM CountLines Build Script for Windows
REM Requires CMake and a C compiler (MSVC, MinGW, etc.)

echo ========================================
echo   CountLines Build Script
echo ========================================
echo.

REM Check if CMake is installed
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

REM Create build directory
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure the project
echo.
echo Configuring project with CMake...
echo.
cmake .. -G "MinGW Makefiles"
if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    echo Trying with Visual Studio generator...
    echo.
    cmake .. -G "Visual Studio 16 2019"
    if errorlevel 1 (
        echo.
        echo ERROR: CMake configuration failed with all generators
        echo Please check your build environment
        echo.
        cd ..
        pause
        exit /b 1
    )
)

REM Build the project
echo.
echo Building project...
echo.
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    echo Please check the error messages above
    echo.
    cd ..
    pause
    exit /b 1
)

REM Check if executable was created
if exist "bin\Release\countlines.exe" (
    set EXECUTABLE=bin\Release\countlines.exe
) else if exist "bin\countlines.exe" (
    set EXECUTABLE=bin\countlines.exe
) else (
    echo.
    echo ERROR: Executable not found
    echo Build may have failed
    echo.
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Build Successful!
echo ========================================
echo.
echo Executable location: build\%EXECUTABLE%
echo.
echo You can now run:
echo   build\%EXECUTABLE% --help
echo   build\%EXECUTABLE% --web
echo   build\%EXECUTABLE% .
echo.
echo To install the web interface, copy the 'web' folder
echo to the same directory as the executable.
echo.

cd ..
pause
