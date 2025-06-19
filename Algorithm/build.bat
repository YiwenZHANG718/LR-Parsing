@echo off
REM Build script for LR Grammar Analyzer (Windows)
REM Author: B22040310
REM Features: Cross-platform compilation with automatic cleanup

setlocal EnableDelayedExpansion

REM Define variables
set CXX=g++
set CXXFLAGS=-std=c++11 -Wall -Wextra -O2
set TARGET=lr_analyzer.exe
set CLI_TARGET=lr_cli.exe
set SOURCES=main.cpp grammar.cpp lr_analyzer.cpp lr_item.cpp
set CLI_SOURCES=lr_cli.cpp grammar.cpp lr_analyzer.cpp lr_item.cpp

REM Check command line argument
if "%1"=="" goto build_all
if "%1"=="help" goto show_help
if "%1"=="clean" goto clean_all
if "%1"=="rebuild" goto rebuild_all
if "%1"=="debug" goto build_debug
if "%1"=="release" goto build_release
if "%1"=="quick" goto build_quick
if "%1"=="test" goto test_build
if "%1"=="status" goto show_status
if "%1"=="run" goto run_main
if "%1"=="run-cli" goto run_cli
goto show_help

:build_all
echo === LR Grammar Analyzer Build ===
echo Platform: Windows
echo Compiler: %CXX%
echo Flags: %CXXFLAGS%
echo ==============================
call :build_targets
call :cleanup_objects
echo === Build Complete ===
echo Platform: Windows
echo Executables created:
echo   - %TARGET%
echo   - %CLI_TARGET%
goto end

:build_debug
echo === Debug Build ===
set CXXFLAGS=%CXXFLAGS% -g -DDEBUG
call :build_targets
call :cleanup_objects
echo === Debug Build Complete ===
goto end

:build_release
echo === Release Build ===
set CXXFLAGS=%CXXFLAGS% -O3 -DNDEBUG
call :build_targets
call :cleanup_objects
echo === Release Build Complete ===
goto end

:build_quick
echo === Quick Build (no cleanup) ===
call :build_targets
echo === Quick Build Complete ===
goto end

:build_targets
echo Building object files...
for %%f in (%SOURCES% %CLI_SOURCES%) do (
    echo Compiling %%f...
    %CXX% %CXXFLAGS% -c %%f -o %%~nf.o
    if errorlevel 1 (
        echo Error compiling %%f
        exit /b 1
    )
)

echo Linking %TARGET%...
%CXX% %CXXFLAGS% -o %TARGET% main.o grammar.o lr_analyzer.o lr_item.o
if errorlevel 1 (
    echo Error linking %TARGET%
    exit /b 1
)
echo ✓ Build successful: %TARGET%

echo Linking %CLI_TARGET%...
%CXX% %CXXFLAGS% -o %CLI_TARGET% lr_cli.o grammar.o lr_analyzer.o lr_item.o
if errorlevel 1 (
    echo Error linking %CLI_TARGET%
    exit /b 1
)
echo ✓ Build successful: %CLI_TARGET%
goto :eof

:cleanup_objects
echo Cleaning up object files...
if exist *.o del /Q *.o >nul 2>&1
echo ✓ Cleanup completed
goto :eof

:clean_all
echo Cleaning all build files...
if exist *.o del /Q *.o >nul 2>&1
if exist *.exe del /Q *.exe >nul 2>&1
if exist lr_analyzer del /Q lr_analyzer >nul 2>&1
if exist lr_cli del /Q lr_cli >nul 2>&1
echo ✓ Clean completed
goto end

:rebuild_all
call :clean_all
call :build_all
goto end

:test_build
call :build_targets
echo === Running Tests ===
echo Testing CLI executable...
if exist ..\TestGrammar\example_grammar.txt (
    %CLI_TARGET% ..\TestGrammar\example_grammar.txt --help
) else (
    echo No test grammar found
)
echo ✓ Test completed
goto end

:show_status
echo === Build Status ===
echo Platform: Windows
if exist %TARGET% (echo ✓ %TARGET% exists) else (echo ✗ %TARGET% missing)
if exist %CLI_TARGET% (echo ✓ %CLI_TARGET% exists) else (echo ✗ %CLI_TARGET% missing)
if exist *.o (echo ⚠ Object files present) else (echo ✓ No object files)
echo ===================
goto end

:run_main
call :build_targets
echo Running %TARGET%...
%TARGET%
goto end

:run_cli
call :build_targets
echo Running %CLI_TARGET% with help...
%CLI_TARGET% --help
goto end

:show_help
echo === LR Grammar Analyzer Build Script Help ===
echo.
echo Platform: Windows
echo Executables will have .exe extension
echo.
echo Main Targets:
echo   build.bat         - Build both executables and auto-cleanup (default)
echo   build.bat clean   - Remove all build files including executables
echo   build.bat rebuild - Clean and build everything
echo.
echo Build Variants:
echo   build.bat debug   - Build with debug symbols
echo   build.bat release - Build with maximum optimization
echo   build.bat quick   - Build without cleanup (for development)
echo.
echo Utilities:
echo   build.bat test    - Test the built executables
echo   build.bat run     - Build and run main program
echo   build.bat run-cli - Build and run CLI program
echo   build.bat status  - Show current build status
echo   build.bat help    - Show this help message
echo.
echo Examples:
echo   build.bat         - Standard build with cleanup
echo   build.bat debug   - Debug build
echo   build.bat quick   - Fast build for development
echo   build.bat rebuild - Complete rebuild
echo ============================================
goto end

:end
endlocal
