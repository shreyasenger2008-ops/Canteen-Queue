@echo off
setlocal

echo ======================================================================
echo Building Canteen Queue Optimization System (DSA Engine)
echo ======================================================================

set CMAKE_EXE="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist %CMAKE_EXE% (
    set CMAKE_EXE=cmake
)

echo Using CMake: %CMAKE_EXE%

%CMAKE_EXE% -B build
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configure failed!
    exit /b %ERRORLEVEL%
)

%CMAKE_EXE% --build build --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    exit /b %ERRORLEVEL%
)

echo ======================================================================
echo Running Tests...
echo ======================================================================

if exist build\Debug\canteen_tests.exe (
    build\Debug\canteen_tests.exe
) else if exist build\canteen_tests.exe (
    build\canteen_tests.exe
) else (
    %CMAKE_EXE% --build build --target RUN_TESTS --config Debug
)

echo Build and test process finished.
