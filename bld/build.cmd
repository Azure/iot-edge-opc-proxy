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
set build-platform=win32
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" set build-platform=x64
set build-os=Windows
set build-use-remote-branch=
set build-skip-dotnet=
set build-pack-only=
set build-clean=
set build-docker-args=
set build-vs-ver=15

set CMAKE_toolset=
set CMAKE_run_unittests=ON
set CMAKE_use_openssl=OFF
set CMAKE_use_zlog=OFF
set CMAKE_use_lws=OFF
set CMAKE_use_dnssd=ON
set CMAKE_prefer_dnssd_embedded_api=OFF
set CMAKE_mem_check=OFF

set build-root="%repo-root%"\build
set build-nuget-output=
set build-context="%repo-root%"
set build-branch=local

:args-loop 
if "%1" equ "" goto :args-done
if "%1" equ  "-x" goto :arg-trace
if "%1" equ "--xtrace" goto :arg-trace
if "%1" equ "--os" goto :arg-build-os
if "%1" equ "--remote" goto :arg-build-remote
if "%1" equ "--clean" goto :arg-build-clean
if "%1" equ  "-c" goto :arg-build-clean
if "%1" equ "--config" goto :arg-build-config
if "%1" equ  "-C" goto :arg-build-config
if "%1" equ "--toolset" goto :arg-build-toolset
if "%1" equ  "-T" goto :arg-build-toolset
if "%1" equ "--vs" goto :arg-build-vs-ver
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
if "%1" equ "--use-dnssd" goto :arg-use-dnssd
if "%1" equ "--with-memcheck" goto :arg-with-memcheck
call :usage && exit /b 1
:arg-trace 
echo on
goto :args-continue
:arg-build-os 
shift
if "%1" equ "" call :usage && exit /b 1
set build-os=%1
goto :args-continue
:arg-build-clean 
set build-clean=1
goto :args-continue
:arg-build-remote 
set build-use-remote-branch=1
goto :args-continue
:arg-build-config 
shift
if "%1" equ "" call :usage && exit /b 1
set build-configs=%build-configs%%1 
goto :args-continue
:arg-build-toolset 
shift
if "%1" equ "" call :usage && exit /b 1
set CMAKE_toolset=-T %1
goto :args-continue
:arg-build-vs-ver 
shift
if "%1" equ "" call :usage && exit /b 1
set build-vs-ver=%1
goto :args-continue
:arg-build-root 
shift
if "%1" equ "" call :usage && exit /b 1
set build-root=%1
goto :args-continue
:arg-build-nuget-folder 
shift
if "%1" equ "" call :usage && exit /b 1
set build-nuget-output=%1
goto :args-continue
:arg-build-platform 
shift
if /I "%1" == "x64" set build-platform=x64 && goto :args-continue
if /I "%1" == "arm" set build-platform=arm && goto :args-continue
if /I "%1" == "x86" set build-platform=win32 && goto :args-continue
if /I not "%1" == "win32" call :usage && exit /b 1
set build-platform=win32
goto :args-continue
:arg-skip-unittests 
set CMAKE_run_unittests=OFF
goto :args-continue
:arg-skip-dotnet 
set build-skip-dotnet=1
goto :args-continue
:arg-pack-only 
set build-pack-only=1
goto :args-continue
:arg-use-dnssd 
shift
if /I "%1" == "No" set CMAKE_use_dnssd=OFF && goto :args-continue
if /I "%1" == "Embedded" set CMAKE_prefer_dnssd_embedded_api=ON && goto :args-continue
if /I not "%1" == "Yes" call :usage && exit /b 1
goto :args-continue
:arg-use-libwebsockets 
set CMAKE_use_lws=ON
echo     ... with libwebsockets
goto :args-continue
:arg-use-zlog 
set CMAKE_use_zlog=ON
echo     ... with zlog
goto :args-continue
:arg-use-openssl 
set CMAKE_use_openssl=ON
echo     ... with openssl
goto :args-continue
:arg-with-memcheck
set CMAKE_mem_check=ON
echo     ... with memory checking
goto :args-continue
:args-continue 
shift
goto :args-loop

:args-done 
if "%build-configs%" == "" set build-configs=Debug Release 
if "%build-nuget-output%" == "" set build-nuget-output=%build-root%
echo Building %build-configs%...
if not "%build-clean%" == "" (
    if not "%build-pack-only%" == "" call :usage && exit /b 1
    echo Cleaning previous build output...
    call :rmdir-force %build-root%
)
if not exist %build-root% mkdir %build-root%
if /I not "%build-os%" == "Windows" goto :docker-build
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
set CMAKE-version=
for /f "tokens=3,4,5 delims=. " %%i in ('cmake --version') do call :native-build-check-cmake %%i %%j %%k
if "%CMAKE-version%" == "" exit /b 1
:native-configure-all 
pushd %build-root%\cmake\%build-platform%
if %build-platform% == x64 (
    echo ***Running CMAKE for Win64***
    call cmake %CMAKE_toolset% -Drun_unittests:BOOL=%CMAKE_run_unittests% -Dmem_check:BOOL=%CMAKE_mem_check% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% -Duse_dnssd:BOOL=%CMAKE_use_dnssd% -Dprefer_dnssd_embedded_api:BOOL=%CMAKE_prefer_dnssd_embedded_api% %repo-root% -G "Visual Studio %build-vs-ver% Win64"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
) else if %build-platform% == arm (
    echo ***Running CMAKE for ARM***
    call cmake %CMAKE_toolset% -Drun_unittests:BOOL=%CMAKE_run_unittests% -Dmem_check:BOOL=%CMAKE_mem_check% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% -Duse_dnssd:BOOL=%CMAKE_use_dnssd% -Dprefer_dnssd_embedded_api:BOOL=%CMAKE_prefer_dnssd_embedded_api% %repo-root% -G "Visual Studio %build-vs-ver% ARM"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
) else (
    echo ***Running CMAKE for Win32***
    call cmake %CMAKE_toolset% -Drun_unittests:BOOL=%CMAKE_run_unittests% -Dmem_check:BOOL=%CMAKE_mem_check% -Duse_lws:BOOL=%CMAKE_use_lws% -Duse_zlog:BOOL=%CMAKE_use_zlog% -Duse_openssl:BOOL=%CMAKE_use_openssl% -Duse_dnssd:BOOL=%CMAKE_use_dnssd% -Dprefer_dnssd_embedded_api:BOOL=%CMAKE_prefer_dnssd_embedded_api% %repo-root% -G "Visual Studio %build-vs-ver%"
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
)
popd
:native-build-all 
if not "%build-pack-only%" == "" goto :eof
for %%c in (%build-configs%) do (
    pushd %build-root%\cmake\%build-platform%
    call :native-build-and-test %%c
    popd
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
)
goto :eof

:native-build-and-test 
if /I not "%1" == "Release" if /I not "%1" == "Debug" if /I not "%1" == "MinSizeRel" if /I not "%1" == "RelWithDebInfo" goto :eof
call cmake --build . --config %1
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
if %build-platform% equ arm goto :eof
if "%CMAKE_run_unittests%" equ "OFF" goto :eof
call ctest -C "%1" -V
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :eof

:native-build-check-cmake 
if not "%CMAKE-version%" == "" goto :eof
if not "%1" == "3" goto :eof
if "%build-vs-ver%" == "15" if %2 lss 7 echo Install cmake 3.7 or higher to build for Visual Studio 2017... && goto :eof
if "%build-vs-ver%" == "14" if %2 equ 0 echo Install cmake 3.1 or higher to build for Visual Studio 2015... && goto :eof
set CMAKE-version=%1.%2.%3
goto :eof

rem -----------------------------------------------------------------------------
rem -- build dotnet api
rem -----------------------------------------------------------------------------
:dotnet-build 
if not "%build-skip-dotnet%" == "" goto :eof
call dotnet --version
if not !ERRORLEVEL! == 0 echo No dotnet installed, skipping dotnet build. && goto :eof
if not "%build-pack-only%" == "" goto :dotnet-build-test-and-pack-all
pushd %repo-root%\api\csharp
call dotnet restore
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
popd
:dotnet-build-test-and-pack-all 
for %%c in (%build-configs%) do (
    pushd %repo-root%\api\csharp
    call :dotnet-build-test-and-pack %%c
    popd
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
)
goto :eof

:dotnet-build-test-and-pack 
if /I not "%1" == "Release" if /I not "%1" == "Debug" if /I not "%1" == "Signed" goto :eof
for /f %%i in ('dir /b /s *.csproj') do call :dotnet-project-build %1 "%%i"
for /f %%i in ('dir /b /s *.csproj') do call :dotnet-project-pack %1 "%%i"
goto :eof
:dotnet-project-build 
if not "%build-pack-only%" == "" goto :eof
if not exist "%build-root%\managed\%1" mkdir "%build-root%\managed\%1"
pushd "%~dp2"
echo.
if not "%build-clean%" == "" call :rmdir-force "bin\%1"
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
if not !ERRORLEVEL! == 0 call :docker-install-prompt && exit /b 1
if /I "%build-os%" == "Linux" set build-os=
pushd %current-path%\docker
dir /b /s Dockerfile.%build-os%* > nul 2> nul
popd
if not !ERRORLEVEL! == 0 call :docker-build-os-not-found && exit /b 1
call :docker-build-args %*
:docker-build-get-git-tag 
rem set tracking branch and remote
set build-remote=
if "%build-use-remote-branch%" == "" goto :docker-build-and-run-all 
set build-branch=
set build-context=.
for /f "tokens=2* delims=/" %%r in ('git symbolic-ref HEAD') do set build-branch=%%s
if not !ERRORLEVEL! == 0 goto :docker-build-and-run-all
for /f "delims=" %%r in ('git config "branch.%build-branch%.remote"') do set build-remote=%%r
if not !ERRORLEVEL! == 0 goto :docker-build-and-run-all
for /f "tokens=2* delims=/" %%r in ('git config "branch.%build-branch%.merge"') do set build-branch=%%s
if not !ERRORLEVEL! == 0 goto :docker-build-and-run-all
for /f "delims=" %%r in ('git remote get-url %build-remote%') do set build-remote=%%r
if not !ERRORLEVEL! == 0 goto :docker-build-and-run-all
if "%build-remote%" == "" set build-remote=https://github.com/Azure/iot-edge-opc-proxy
if "%build-branch%" == "" set build-branch=master
:docker-build-and-run-all 
pushd %current-path%\docker
for /f %%i in ('dir /b /s Dockerfile.%build-os%*') do (
    call :docker-build-and-run %%i
    if not !ERRORLEVEL! == 0 popd 
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
)
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
goto :build-done
:docker-build-and-run 
set docker-build-os=
for /f "tokens=1* delims=." %%i in ("%~nx1") do set docker-build-os=%%j
echo     ... Building azure-iot-proxy:%docker-build-os%-env
docker build -f Dockerfile.%docker-build-os% -t azure-iot-proxy:%docker-build-os%-env .
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
echo     ... Building azure-iot-proxy:%docker-build-os%-%build-branch% %build-docker-args%
rem create proxy docker file
    echo FROM azure-iot-proxy:%docker-build-os%-env > tmp.Dockerfile
if "%build-use-remote-branch%" == "" (
    echo COPY / /repo >> tmp.Dockerfile
    echo RUN ./repo/bld/docker/docker-build.sh %build-docker-args% >> tmp.Dockerfile
) else (
    echo ENV PROXY_REPO=%build-remote% >> tmp.Dockerfile
    echo ENV COMMIT_ID=%build-branch% >> tmp.Dockerfile
    echo COPY docker-build.sh / >> tmp.Dockerfile
    echo RUN ./docker-build.sh %build-docker-args% >> tmp.Dockerfile
)
if not "%build-clean%" == "" docker rmi -f azure-iot-proxy:%docker-build-os%-%build-branch%
docker build -f tmp.Dockerfile -t azure-iot-proxy:%docker-build-os%-%build-branch% %build-context%
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
if exist tmp.Dockerfile del /f tmp.Dockerfile
goto :eof
:docker-build-args 
if "%1" equ "" goto :eof
if "%1" equ "--os" shift && shift && goto :docker-build-args
if "%1" equ "--remote" shift && goto :docker-build-args
set build-docker-args=%build-docker-args% %1
shift
goto :docker-build-args
:docker-install-prompt 
echo Install docker from https://docs.docker.com/docker-for-windows for option --os %build-os%
call :usage
goto :eof
:docker-build-os-not-found 
echo No OS image definitions for %build-os% found!
call :usage 
goto :eof

rem -----------------------------------------------------------------------------
rem -- subroutines
rem -----------------------------------------------------------------------------

:rmdir-force 
set _attempt=0
:try-rmdir 
if not exist %1 goto :done-rmdir
set /a _attempt+=1
if _attempt == 30 goto :done-rmdir
echo Removing %~1 (%_attempt%)...
rmdir /s /q %1
goto :try-rmdir
:done-rmdir 
set _attempt=
goto :eof

:build-done 
echo ... Success!
goto :eof

:usage 
echo build.cmd [options]
echo options:
echo -x --xtrace                 print a trace of each command.
echo    --os ^<value^>             [Windows] OS to build on (needs Docker if Linux Flavor).
echo    --remote                 Build current branch on remote rather than use local build context.
echo -c --clean                  Build clean (Removes previous build output).
echo -C --config ^<value^>         [%build-configs%] build configuration (e.g. Debug, Release).
echo -T --toolset ^<value^>        An optional toolset to use, e.g. v140 or clang.
echo -o --build-root ^<value^>     [/build] Directory in which to place all files during build.
echo    --use-zlog               Use zlog as logging framework instead of xlogging.
echo    --use-openssl            Uses openssl instead of schannel.
echo    --use-libwebsockets      Uses libwebsockets instead of winhttp on Windows.
echo    --use-dnssd ^<value^>      [Yes] Sets the dnssd build option (Yes, No, Embedded).
echo    --with-memcheck          Compile in memory checks.
echo    --skip-unittests         Skips building and executing unit tests.
echo    --skip-dotnet            Skips building dotnet API and packages.
echo    --pack-only              Only creates packages. (Cannot be combined with --clean)
echo -n --nuget-folder ^<value^>   [/build] Folder to use when outputting nuget packages.
echo -p --platform ^<value^>       [%build-platform%] build platform (e.g. Win32, x64, ...).
echo    --vs ^<value^>               [%build-vs-ver%] Visual Studio version to use (e.g. 14 for Visual Studio 2015).
goto :eof

