@echo off
setlocal

:: CTR Native Build Script (Windows)
:: Requires MSYS2 MinGW32 (i686-w64-mingw32-gcc) in PATH

where i686-w64-mingw32-gcc >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: i686-w64-mingw32-gcc not found in PATH
    echo.
    echo Install MSYS2, then run:
    echo   pacman -S mingw-w64-i686-gcc mingw-w64-i686-tools-git
    echo Then add C:\msys64\mingw32\bin to your PATH
    exit /b 1
)

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configure failed
    exit /b 1
)

cmake --build build -j
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo Build succeeded: build\ctr_native.exe
