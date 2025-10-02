@echo off
setlocal enabledelayedexpansion

:: Configuration
set RAYLIB_PATH=C:\raylib
set RAYLIB_INCLUDE_PATH=%RAYLIB_PATH%\raylib\src
set RAYLIB_LIB_PATH=%RAYLIB_PATH%\raylib\src
set MINGW_PATH=%RAYLIB_PATH%\w64devkit
set PROJECT_NAME=chip8
set BUILD_MODE=RELEASE

:: Set up paths for MinGW (includes gcc)
set PATH=%MINGW_PATH%\bin;%PATH%

echo Building %PROJECT_NAME%...
echo.

:: Create bin directory if it doesn't exist
if not exist "bin" mkdir bin

:: Check if gcc is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo Error: gcc not found!
    echo Expected path: %MINGW_PATH%\bin
    pause
    exit /b 1
)

:: Check if raylib exists
if not exist "%RAYLIB_INCLUDE_PATH%\raylib.h" (
    echo Error: raylib.h not found at %RAYLIB_INCLUDE_PATH%!
    pause
    exit /b 1
)

:: Check if main.c exists
if not exist "src\main.c" (
    echo Error: src\main.c not found!
    pause
    exit /b 1
)

:: Collect all .c files in src directory
set SOURCE_FILES=
for %%f in (src\*.c) do (
    set SOURCE_FILES=!SOURCE_FILES! "%%f"
)

:: Compiler flags
set CFLAGS=-std=c99 -Wall -Wextra -Wno-missing-braces
set INCLUDE_FLAGS=-I"%RAYLIB_INCLUDE_PATH%" -I"src"
set DEFINE_FLAGS=-DPLATFORM_DESKTOP

:: Debug vs Release flags
if /i "%BUILD_MODE%"=="DEBUG" (
    set CFLAGS=%CFLAGS% -g -O0 -DDEBUG
    set OUTPUT_DIR=bin\debug
    echo Building in DEBUG mode...
) else (
    set CFLAGS=%CFLAGS% -O2 -DNDEBUG
    set OUTPUT_DIR=bin\release
    echo Building in RELEASE mode...
)

:: Create output directory
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Windows-specific linker flags for raylib
set LDFLAGS=-L"%RAYLIB_LIB_PATH%"
set LDLIBS=-lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lole32

:: Check if we need to build raylib first
set RAYLIB_LIB=%RAYLIB_LIB_PATH%\libraylib.a
if not exist "%RAYLIB_LIB%" (
    echo libraylib.a not found. Building raylib...
    pushd "%RAYLIB_PATH%\raylib\src"
    mingw32-make PLATFORM=PLATFORM_DESKTOP CC=gcc >nul 2>&1
    if errorlevel 1 (
        echo Failed to build raylib automatically.
        popd
        pause
        exit /b 1
    )
    popd
    echo raylib built successfully!
)

echo.
echo Compiling project...
echo Sources: %SOURCE_FILES%
echo Output: %OUTPUT_DIR%\%PROJECT_NAME%.exe
echo.

:: Compile command
gcc %CFLAGS% %DEFINE_FLAGS% %INCLUDE_FLAGS% %SOURCE_FILES% -o "%OUTPUT_DIR%\%PROJECT_NAME%.exe" %LDFLAGS% %LDLIBS%

:: Check compilation result
if errorlevel 1 (
    echo.
    echo Compilation failed!
    echo.
    pause
    exit /b 1
) else (
    echo.
    echo Compilation successful!
    echo Running: %OUTPUT_DIR%\%PROJECT_NAME%.exe %*
    echo.
    "%OUTPUT_DIR%\%PROJECT_NAME%.exe" %*
)