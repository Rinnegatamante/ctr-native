@echo off
setlocal

:: CTR Native Build Script (Windows)
:: Requires MSYS2 MinGW32 tools in PATH

set "CTR_NATIVE_MSYS2_PACKAGES=git mingw-w64-i686-gcc mingw-w64-i686-cmake mingw-w64-i686-make"

where i686-w64-mingw32-gcc >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: i686-w64-mingw32-gcc not found in PATH
    echo.
    echo Install MSYS2, then run:
    echo   pacman -S --needed %CTR_NATIVE_MSYS2_PACKAGES%
    echo Then add C:\msys64\mingw32\bin to your PATH
    exit /b 1
)

where i686-w64-mingw32-g++ >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: i686-w64-mingw32-g++ not found in PATH
    echo.
    echo Install MSYS2, then run:
    echo   pacman -S --needed %CTR_NATIVE_MSYS2_PACKAGES%
    echo Then add C:\msys64\mingw32\bin to your PATH
    exit /b 1
)

where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: cmake not found in PATH
    echo.
    echo Install MSYS2, then run:
    echo   pacman -S --needed %CTR_NATIVE_MSYS2_PACKAGES%
    echo Then add C:\msys64\mingw32\bin to your PATH
    exit /b 1
)

where mingw32-make >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: mingw32-make not found in PATH
    echo.
    echo Install MSYS2, then run:
    echo   pacman -S --needed %CTR_NATIVE_MSYS2_PACKAGES%
    echo Then add C:\msys64\mingw32\bin to your PATH
    exit /b 1
)

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ -DCMAKE_MAKE_PROGRAM=mingw32-make -DCMAKE_BUILD_TYPE=Release "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
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
