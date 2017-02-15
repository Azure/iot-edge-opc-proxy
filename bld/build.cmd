@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set repo-root=%current-path%\..
rem // resolve to fully qualified path
for %%i in ("%repo-root%") do set repo-root=%%~fi

echo Building %repo-root% ...

rem ----------------------------------------------------------------------------
rem -- parse script arguments
rem ----------------------------------------------------------------------------

rem // default build options
set build-configs=
set build-platform=Win32
set build-os=Windows
set build-skip-dotnet=
set build-pack-only=
set build-clean=

set CMAKE_skip_unittests=OFF
set CMAKE_use_openssl=OFF
set CMAKE_use_zlog=OFF
set CMAKE_use_lws=OFF

set build-root="%repo-root%"\build
set build-nuget-output=%build-root%

:args-loop
if "%1" equ "" goto :args-done
if "%1" equ "--os" goto :arg-build-os
if "%1" equ "--clean" goto :arg-build-clean
if "%1" equ  "-c" goto :arg-build-clean
if "%1" equ "--config" goto :arg-build-config
if "%1" equ  "-C" goto :arg-build-config
if "%1" equ "--build-root" goto :arg-build-root
if "%1" equ  "-o" goto :arg-build-root
if "%1" equ "--nuget-folder" goto :arg-build-nuget-folder
if "%1" equ  "-n" goto :arg-build-nuget-folder
if "%1" equ "--platform" goto :arg-build-platform
if "%1" equ  "-p" goto :arg-build-platform
if "%1" equ "--skip-unittests" goto :arg-skip-unittests
if "%1" equ "--skip-dotnet" goto :arg-skip-dotnet
if "%1" equ "--pack-only" goto :arg-pack-only
if "%1" equ "--use-zlog" goto :arg-use-zlog
if "%1" equ "--use-libwebsockets" goto :arg-use-libwebsockets
if "%1" equ "--use-openssl" goto :arg-use-openssl
call :usage && exit /b 1

:arg-build-clean
set build-clean=1
goto :args-continue

:arg-build-config
shift
if "%1" equ "" call :usage && exit /b 1
set build-configs=%build-configs%%1 
goto :args-continue

:arg-build-platform
shift
if "%1" equ "" call :usage && exit /b 1
set build-platform=%1
if %build-platform% == x64 (
    set CMAKE_DIR=x64
) else if %build-platform% == arm (
    set CMAKE_DIR=arm
)
goto :args-continue

:arg-pack-only
set build-pack-only=1
goto :args-continue

:arg-skip-dotnet
set build-skip-dotnet=1
goto :args-continue

:arg-skip-unittests
set CMAKE_skip_unittests=ON
goto :args-continue

:arg-build-os
shift
if "%1" equ "" call :usage && exit /b 1
set build-os=%1
if /I "%build-os%" == "Windows" goto :args-continue
if /I "%build-os%" == "Linux" set build-os=
if not exist %current-path%\docker\%build-os% call :usage && exit /b 1
call :docker-build
if not !ERRORLEVEL! == 0 echo ERROR during docker build... && exit /b !ERRORLEVEL!
goto :build-done

:arg-build-nuget-folder
shift
if "%1" equ "" call :usage && exit /b 1
set build-nuget-output=%1
goto :args-continue

:arg-use-libwebsockets
set CMAKE_use_lws=ON
echo     ... with libwebsockets
goto :args-continue

:arg-use-openssl
set CMAKE_use_openssl=ON
echo     ... with openssl
goto :args-continue

:arg-use-zlog
set CMAKE_use_zlog=ON
echo     ... with zlog
goto :args-continue

:arg-build-root
shift
if "%1" equ "" call :usage && exit /b 1
set build-root=%1

goto :args-continue
:args-continue
shift
goto :args-loop

:args-done

if "%build-configs%" == "" set build-configs=Debug Release 
echo Building %build-configs%...
if not "%build-clean%" == "" (
    if not "%build-pack-only%" == "" call :usage && exit /b 1
    echo Cleaning previous build output...
    if exist %build-root% rmdir /s /q %build-root%
)
if not exist %build-root% mkdir %build-root%

call :native-build
if not !ERRORLEVEL! == 0 echo Failures during native build... && exit /b !ERRORLEVEL!
call :dotnet-build
if not !ERRORLEVEL! == 0 echo Failures during dotnet build... && exit /b !ERRORLEVEL!
goto :build-done

rem -----------------------------------------------------------------------------
rem -- build natively with CMAKE and run tests
rem -----------------------------------------------------------------------------

:native-build
if not exist %build-root%\cmake\%build-platform% mkdir %build-root%\cmake\%build-platform%
pushd %build-root%\cmake\%build-platform%
if %build-platform% == x64 (
    echo ***Running CMAKE for Win64***
    cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %repo-root% -G "Visual Studio 14 Win64"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
) else if %build-platform% == arm (
    echo ***Running CMAKE for ARM***
    cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %repo-root% -G "Visual Studio 14 ARM"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
) else (
    echo ***Running CMAKE for Win32***
    cmake -Dskip_unittests:BOOL=%CMAKE_skip_unittests% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% %repo-root% -G "Visual Studio 14"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
)
popd
if not "%build-pack-only%" == "" goto :eof
for %%c in (%build-configs%) do call :native-build-and-test %%c

:native-build-and-test
if /I not "%1" == "Release" if /I not "%1" == "Debug" if /I not "%1" == "MinSizeRel" if /I not "%1" == "RelWithDebInfo" goto :eof
pushd %build-root%\cmake\%build-platform%
msbuild /m azure-iot-proxy.sln /p:Configuration=%1 /p:Platform=%build-platform%
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
if %build-platform% equ arm goto :eof
if "%CMAKE_skip_unittests%" equ "ON" goto :eof
pushd %build-root%\cmake\%build-platform%
ctest -C "%1" -V
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof

rem -----------------------------------------------------------------------------
rem -- build dotnet api
rem -----------------------------------------------------------------------------
:dotnet-build
if not "%build-skip-dotnet%" == "" goto :eof
call dotnet --version
if not !ERRORLEVEL! == 0 echo No dotnet installed, skipping dotnet build. && goto :eof
pushd %repo-root%\api\csharp
if "%build-pack-only%" == "" call dotnet restore
for %%c in (%build-configs%) do call :dotnet-build-test-and-pack %%c
popd
goto :eof

:dotnet-build-test-and-pack
if /I not "%1" == "Release" if /I not "%1" == "Debug" if /I not "%1" == "Signed" goto :eof
for /f %%i in ('dir /b /s project.json') do call :dotnet-project-build %1 "%%i"
for /f %%i in ('dir /b /s project.json') do call :dotnet-project-pack %1 "%%i"
goto :eof
:dotnet-project-build
if not "%build-pack-only%" == "" goto :eof
if not exist "%build-root%\managed\%1" mkdir "%build-root%\managed\%1"
pushd "%~dp2"
echo.
if not "%build-clean%" == "" rmdir /s /q "bin\%1"
call dotnet build -c %1
xcopy /e /y /i /q "bin\%1" "%build-root%\managed\%1"
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof
:dotnet-project-pack
if not exist "%build-nuget-output%\%1" mkdir "%build-nuget-output%\%1"
echo.
call dotnet pack --no-build -c %1 -o "%build-nuget-output%\%1" "%2"
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof


rem -----------------------------------------------------------------------------
rem -- build all in Docker container
rem -----------------------------------------------------------------------------
:docker-build
call docker --version
if not !errorlevel!==0 call :docker-install-prompt && exit /b 1
rem filter build args to pass to build.sh
call :docker-build-args %*
echo     ... azure-iot-proxy:build-%build-os% %docker-run-args%
rem make image and run
call :docker-build-image
call :docker-build-run
set docker-run-args=
goto :eof

:docker-build-args
:docker-build-args-loop
if "%1" equ "" goto :eof
if "%1" equ "--os" shift && shift && goto :docker-build-args-loop
set docker-run-args=%docker-run-args% %1
shift
goto :docker-build-args-loop
:docker-build-image
set docker-file-path=%current-path%\docker\%build-os%
for %%i in ("%docker-file-path%") do set docker-file-path=%%~fi

if "%build-os%" equ "" set build-os=ubuntu
pushd %current-path%\docker
docker build -f %docker-file-path%\Dockerfile -t azure-iot-proxy:build-%build-os% .
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof
:docker-build-run
if "%build-os%" equ "" set build-os=ubuntu
for /f "delims=" %%r in ('git rev-parse HEAD') do set build-commit-env=-e COMMIT_ID=%%r
docker run -ti %build-commit-env% azure-iot-proxy:build-%build-os% %docker-run-args%
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof
:docker-install-prompt
echo Install docker from https://docs.docker.com/docker-for-windows for option --os %build-os%
call :usage
goto :eof

rem -----------------------------------------------------------------------------
rem -- subroutines
rem -----------------------------------------------------------------------------

:build-done
echo ... Success!
goto :eof

:usage
echo build.cmd [options]
echo options:
echo    --os ^<value^>             [Windows] OS to build on (needs Docker if Linux Flavor).
echo -c --clean                  Build clean (Removes previous build output).
echo -C --config ^<value^>         [Debug] build configuration (e.g. Debug, Release).
echo -o --build-root ^<value^>     [/build] Directory in which to place all files during build.
echo    --use-zlog               Use zlog as logging framework instead of xlogging.
echo    --use-openssl            Uses openssl instead of schannel.
echo    --use-libwebsockets      Uses libwebsockets instead of winhttp on Windows.
echo    --skip-unittests         Skips building and executing unit tests.
echo    --skip-dotnet            Skips building dotnet API and packages.
echo    --pack-only              Only creates packages. (Cannot be combined with --clean)
echo -n --nuget-folder ^<value^>   [/build] Folder to use when outputting nuget packages.
echo -p --platform ^<value^>       [Win32] build platform (e.g. Win32, x64, ...).
goto :eof

