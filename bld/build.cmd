@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off


set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set build-root=%current-path%\..
rem // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

set repo_root=%build-root%\..
rem // resolve to fully qualified path
for %%i in ("%repo_root%") do set repo_root=%%~fi

echo Build Root: %build-root%
echo Repo Root: %repo_root%

set cmake-root=%build-root%

rem ----------------------------------------------------------------------------
rem -- parse script arguments
rem ----------------------------------------------------------------------------

rem // default build options
set build-config=Debug
set build-platform=Win32

set CMAKE_skip_unittests=OFF
set CMAKE_use_openssl=OFF
set CMAKE_use_zlog=OFF
set CMAKE_use_lws=OFF

:args-loop
if "%1" equ "" goto args-done
if "%1" equ "--config" goto arg-build-config
if "%1" equ "--platform" goto arg-build-platform
if "%1" equ "--skip-unittests" goto arg-skip-unittests
if "%1" equ "--use-zlog" goto arg-use-zlog
if "%1" equ "--use-libwebsockets" goto arg-use-libwebsockets
if "%1" equ "--use-openssl" goto arg-use-openssl
call :usage && exit /b 1

:arg-build-config
shift
if "%1" equ "" call :usage && exit /b 1
set build-config=%1
goto args-continue

:arg-build-platform
shift
if "%1" equ "" call :usage && exit /b 1
set build-platform=%1
if %build-platform% == x64 (
    set CMAKE_DIR=x64
) else if %build-platform% == arm (
    set CMAKE_DIR=arm
)
goto args-continue

:arg-skip-unittests
set CMAKE_skip_unittests=ON
goto args-continue

:arg-use-libwebsockets
set CMAKE_use_lws=ON
goto args-continue

:arg-use-openssl
set CMAKE_use_openssl=ON
goto args-continue

:arg-use-zlog
set CMAKE_use_zlog=ON
goto args-continue

:arg-cmake-root
shift
if "%1" equ "" call :usage && exit /b 1
set cmake-root=%1

goto args-continue
:args-continue
shift
goto args-loop

:args-done

rem -----------------------------------------------------------------------------
rem -- build with CMAKE and run tests
rem -----------------------------------------------------------------------------

rem this is setting the cmake path in a quoted way
set cmake-root="%build-root%"\build

if EXIST %cmake-root%\cmake\%CMAKE_DIR% (
    rmdir /s/q %cmake-root%\cmake\%CMAKE_DIR%
    rem no error checking
)

echo CMAKE Output Path: %cmake-root%\cmake\%CMAKE_DIR%
mkdir %cmake-root%\cmake\%CMAKE_DIR%
rem no error checking
pushd %cmake-root%\cmake\%CMAKE_DIR%

if %build-platform% == x64 (
	echo ***Running CMAKE for Win64***
	cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %build-root%  -G "Visual Studio 14 Win64"
	if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
) else if %build-platform% == arm (
	echo ***Running CMAKE for ARM***
	cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %build-root%  -G "Visual Studio 14 ARM"
	if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
) else (
	echo ***Running CMAKE for Win32***
	cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %build-root% 
	if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
)

msbuild /m azure-iot-proxy.sln
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

if %build-platform% neq arm (
    ctest -C "debug" -V
    if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
)

popd
goto :eof

rem -----------------------------------------------------------------------------
rem -- subroutines
rem -----------------------------------------------------------------------------

:usage
echo build.cmd [options]
echo options:
echo  --config ^<value^>         [Debug] build configuration (e.g. Debug, Release)
echo  --platform ^<value^>       [Win32] build platform (e.g. Win32, x64, ...)
echo  --use-zlog                 Uses zlog instead of xlogging
echo  --use-openssl              Uses openssl instead of schannel
echo  --use-libwebsockets        Uses libwebsockets instead of winhttp on windows
echo  --cmake-root			     Directory to place the cmake files used for building the project
echo  --skip-unittests           Skips building and executing unit tests (not advisable)
goto :eof

