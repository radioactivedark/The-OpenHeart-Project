REM ##################################################################################################
REM ##------------------------------------------FILE START------------------------------------------##
REM ##################################################################################################


REM Set project details.
@echo off
setlocal EnableExtensions
title OpenHeart Client Build
echo.
echo ########################################
echo ##     OpenHeart Client Builder      ##
echo ########################################
echo.

REM Set project & dep directories.
set "PROJECT_DIR=%~dp0"
set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"
set "DEPS_DIR=%PROJECT_DIR%\..\.Dependencies"
set "CLIENT_DEPS=%DEPS_DIR%\Client"
set "SHARED_DEPS=%DEPS_DIR%\Shared"
set "MINGW_DIR=%CLIENT_DEPS%\MinGW"
set "MINGW_BIN=%MINGW_DIR%\bin"
set "GCC=%MINGW_BIN%\gcc.exe"
set "GXX=%MINGW_BIN%\mingw32-g++.exe"
set "WINDRES=%MINGW_BIN%\windres.exe"
set "MAKE=%MINGW_BIN%\mingw32-make.exe"
set "BUILD_DIR=%PROJECT_DIR%\build"

REM Echo directories to user.
echo Project:
echo   %PROJECT_DIR%
echo.
echo Dependencies:
echo   %DEPS_DIR%
echo.
echo MinGW:
echo   %MINGW_DIR%
echo.

REM Error for missing deps.
if not exist "%GCC%" (
    echo ERROR: Bundled GCC was not found:
    echo   %GCC%
    echo.
    goto :error
)
if not exist "%GXX%" (
    echo ERROR: Bundled G++ was not found:
    echo   %GXX%
    echo.
    goto :error
)
if not exist "%WINDRES%" (
    echo ERROR: Bundled windres was not found:
    echo   %WINDRES%
    echo.
    goto :error
)
if not exist "%MAKE%" (
    echo ERROR: Bundled MinGW Make was not found:
    echo   %MAKE%
    echo.
    goto :error
)
if not exist "%PROJECT_DIR%\CMakeLists.txt" (
    echo ERROR: CMakeLists.txt was not found:
    echo   %PROJECT_DIR%\CMakeLists.txt
    echo.
    goto :error
)

REM Set CMake executable.
set "CMAKE=%PROJECT_DIR%\..\INCLUDED APPLICATIONS\CMake\bin\cmake.exe"

REM Error if CMake isn't found.
if not exist "%CMAKE%" (
    echo ERROR: Bundled CMake was not found.
    echo.
    echo Expected it at:
    echo   %CMAKE%
    echo.
    goto :error
)

REM Echo CMake directory to user.
echo CMake:
echo   %CMAKE%
echo.

REM Clear pre-existing build dir if existing, makes new one.
if exist "%BUILD_DIR%" (
    echo Cleaning previous build configuration...
    rmdir /s /q "%BUILD_DIR%"
    if errorlevel 1 goto :error
)
mkdir "%BUILD_DIR%"
if errorlevel 1 goto :error

REM Set dir for CMake options.
set "CMAKE_PROJECT_DIR=%PROJECT_DIR:\=/%"
set "CMAKE_BUILD_DIR=%BUILD_DIR:\=/%"
set "CMAKE_GCC=%GCC:\=/%"
set "CMAKE_GXX=%GXX:\=/%"
set "CMAKE_WINDRES=%WINDRES:\=/%"
set "CMAKE_MAKE=%MAKE:\=/%"

REM Echoing configuration status to user.
echo.
echo ########################################
echo ##          Configuring...            ##
echo ########################################
echo.

REM CMake commands.
"%CMAKE%" ^
    -S "%CMAKE_PROJECT_DIR%" ^
    -B "%CMAKE_BUILD_DIR%" ^
    -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER="%CMAKE_GCC%" ^
    -DCMAKE_CXX_COMPILER="%CMAKE_GXX%" ^
    -DCMAKE_RC_COMPILER="%CMAKE_WINDRES%" ^
    -DCMAKE_MAKE_PROGRAM="%CMAKE_MAKE%"

if errorlevel 1 (
    echo.
    echo ########################################
    echo ##     CMake configuration FAILED     ##
    echo ########################################
    echo.
    goto :error
)

REM Echo build status, execute CMake commands.
echo.
echo ########################################
echo ##            Building...             ##
echo ########################################
echo.
"%CMAKE%" --build "%CMAKE_BUILD_DIR%" --parallel
if errorlevel 1 (
    echo.
    echo ########################################
    echo ##           BUILD FAILED            ##
    echo ########################################
    echo.
    goto :error
)

REM Echo build status if successful, and location to user.
echo.
echo ########################################
echo ##          BUILD SUCCESSFUL!         ##
echo ########################################
echo.
echo Executable should be in:
echo   %PROJECT_DIR%\RELEASE
echo.
if exist "%PROJECT_DIR%\RELEASE\OpenHeart.exe" (
    echo Found:
    echo   %PROJECT_DIR%\RELEASE\OpenHeart.exe
    echo.
)
pause
exit /b 0
:error
echo.
echo ########################################
echo ##            BUILD FAILED            ##
echo ########################################
echo.
pause
exit /b 1


REM ##################################################################################################
REM ##-------------------------------------------FILE END-------------------------------------------##
REM ##################################################################################################
REM ##-------------------------------------------MODIFIED-------------------------------------------##
REM ## DATE: 9/25/2026                                                                              ##
REM ## TIME: 3:15 PM (UTC-5:00)                                                                     ##
REM ## BY: radioactivedark                                                                          ##
REM ##################################################################################################