@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

set me=%~n0

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set repo-root=%current-path%\..\..\..
rem // resolve to fully qualified path
for %%i in ("%repo-root%") do set repo-root=%%~fi

rem ----------------------------------------------------------------------------
rem -- parse script arguments
rem ----------------------------------------------------------------------------

rem // default build options
set share_root=root
set build_config=Release

set fuzzer_index=
set fuzzer_jobs=%NUMBER_OF_PROCESSORS%
set fuzzer_interval=3h
set fuzzer_do_tmin=1
set fuzzer_use_asan=
set fuzzer_no_push=

set role=0
set worker_flags=
set git_uri=
set git_sync=

rem manager instance name
set instance_name=%COMPUTERNAME%

rem project target and target mode
set executable=protofuzz
set mode=json

:args-loop 
if "%1" equ "" goto :args-done
if "%1" equ  "-x" goto :arg-trace
if "%1" equ "--xtrace" goto :arg-trace
if "%1" equ "--name" goto :arg-name
if "%1" equ "--repo-root" goto :arg-repo-root
if "%1" equ  "-r" goto :arg-repo-root
if "%1" equ "--mode" goto :arg-mode
if "%1" equ  "-m" goto :arg-mode
if "%1" equ "--executable" goto :arg-executable
if "%1" equ  "-e" goto :arg-executable
if "%1" equ "--destroy" goto :arg-destroy
if "%1" equ "--config" goto :arg-build-config
if "%1" equ  "-C" goto :arg-build-config
if "%1" equ "--jobs" goto :arg-jobs
if "%1" equ  "-j" goto :arg-jobs
if "%1" equ "--interval" goto :arg-interval
if "%1" equ  "-i" goto :arg-interval
if "%1" equ "--use-asan" goto :arg-use-asan
if "%1" equ "--no-push" goto :arg-no-push
if "%1" equ "--lightweight" goto :arg-lightweight
if "%1" equ  "-l" goto :arg-lightweight
call :usage && exit /b 1
:arg-trace 
set worker_flags=%worker_flags% %1
echo on
goto :args-continue
:arg-destroy
set role=3
goto :args-continue
:arg-use-asan
set worker_flags=%worker_flags% %1
set fuzzer_use_asan=1
goto :args-continue
:arg-lightweight
set worker_flags=%worker_flags% %1
set fuzzer_do_tmin=
goto :args-continue
:arg-no-push
set worker_flags=%worker_flags% %1
set fuzzer_no_push=1
goto :args-continue
:arg-name
shift
if "%1" equ "" call :usage && exit /b 1
set instance_name=%1
goto :args-continue
:arg-repo-root
shift
if "%1" equ "" call :usage && exit /b 1
set share_root=%1
goto :args-continue
:arg-mode
shift
if "%1" equ "" call :usage && exit /b 1
set mode=%1
goto :args-continue
:arg-executable
shift
if "%1" equ "" call :usage && exit /b 1
set executable=%1
goto :args-continue
:arg-build-config 
shift
if "%1" equ "" call :usage && exit /b 1
set build_config=%1 
goto :args-continue
:arg-jobs 
shift
if "%1" equ "" call :usage && exit /b 1
set fuzzer_jobs=%1
goto :args-continue
:arg-interval
shift
if "%1" equ "" call :usage && exit /b 1
set fuzzer_interval=%1
goto :args-continue
:args-continue 
shift
goto :args-loop
:args-done 

rem check docker installed
call docker --version
if not !ERRORLEVEL! == 0 call :docker-install-prompt && exit /b 1
set project=%executable%-%json%
echo == Fuzz project %project% ==

rem -----------------------------------------------------------------------------
rem -- Stop and cleanup existing fuzzing environment
rem -----------------------------------------------------------------------------
:fuzzer-cleanup
rem if not "%build-clean%" == "" docker rmi -f afl-%project%
for /f %%i in ('docker ps -aqf "label=%project%"') do docker stop %%i
for /f %%i in ('docker ps -aqf "label=%project%"') do docker rm -f %%i
if not "%role%" == "3" goto :fuzzer-build
echo ... stopped!
goto :eof

rem -----------------------------------------------------------------------------
rem -- Build fuzzing environment
rem -----------------------------------------------------------------------------
:fuzzer-build
echo building %build_config% container images ...
rem build afl environment
pushd %repo-root%\bld\docker
docker build -f Dockerfile.ubuntu-xenial.afl -t afl-env .
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
rem build clone into afl environment
pushd %repo-root%
echo FROM afl-env > tmp.Dockerfile
echo COPY / /repo >> tmp.Dockerfile
if not "%fuzzer_use_asan%" == "1" echo ENV AFL_HARDEN 1 >> tmp.Dockerfile
if "%fuzzer_use_asan%" == "1" echo ENV AFL_USE_ASAN 1 >> tmp.Dockerfile
echo ENV AFL_NO_BUILTIN 1 >> tmp.Dockerfile
echo RUN ./repo/bld/docker/docker-build.sh --cl -ggdb -C %build_config% --skip-unittests >> tmp.Dockerfile
docker build -f tmp.Dockerfile -t afl-%project% .
if exist tmp.Dockerfile del /f tmp.Dockerfile
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!

rem Create or sync sync dir and if needed seed initial test cases
if not exist %share_root% mkdir %share_root%
for %%i in ("%share_root%") do set share_root=%%~fi
pushd %share_root%
if exist %share_root%\.git git_sync=1
if not "%git_sync%" == "1" goto :fuzzer-seed
git pull
popd
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
:fuzzer-seed
pushd %share_root%
for /f %%i in ('dir /a /b  %project%') do goto :fuzzer-seed-prompt
goto :fuzzer-seed-testcases
:fuzzer-seed-prompt
echo Found previous sync directory %share_root%\%project%!
set /P seed_prompt="Do you want to reseed with initial test cases? Y/n [n]"
if not "%seed_prompt%" == "Y" goto :fuzzer-start
:fuzzer-seed-testcases
echo Seeding %share_root%\testcases
if not exist testcases mkdir testcases
copy %current-path%\json\* %share_root%\testcases\
dir /a /b testcases\* 
echo Creating sync dir...
if not exist %project% mkdir %project%
popd

rem -----------------------------------------------------------------------------
rem -- Start environment
rem -----------------------------------------------------------------------------
rem start workers
:fuzzer-start
set fuzzer_index=0
:fuzzer-start-worker
    if "%fuzzer_index%" == "%fuzzer_jobs%" goto :fuzzer-start-manager
    echo Starting worker s_%instance_name%_%fuzzer_index% mounted to %share_root%:/share...
    docker run -l %project%=worker -itd --restart=on-failure:2 --privileged=true -v %share_root%:/share -h s_%instance_name%_%fuzzer_index% --entrypoint=/repo/bld/test/fuzz/%me%.sh afl-%project% --worker -C %build_config% %worker_flags% --name %instance_name% -e %executable% -m %mode% -r /share -n %fuzzer_index%
    if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
    set /A fuzzer_index=%fuzzer_index%+1
    goto :fuzzer-start-worker
rem start manager
:fuzzer-start-manager
echo Starting manager m_%instance_name% mounted to %share_root%:/share...
docker run -l %project%=manager -it --restart=always --privileged=true -v /var/run/docker.sock:/var/run/docker.sock -v %share_root%:/share -h m_%instance_name% --entrypoint=/repo/bld/test/fuzz/%me%.sh afl-%project% --manager -C %build_config% %worker_flags% --name %instance_name% -e %executable% -m %mode% -r /share -i %fuzzer_interval% 
if not !ERRORLEVEL! == 0 exit /b !ERRORLEVEL!
echo ... started!
goto :eof

rem -----------------------------------------------------------------------------
rem -- subroutines
rem -----------------------------------------------------------------------------

:docker-install-prompt 
echo Install docker from https://docs.docker.com/docker-for-windows !
call :usage
goto :eof

:usage 
echo %me%.cmd [options]
echo options:
echo  -x --xtrace              print a trace of each command.
echo  -e --executable ^<value^>  [%executable%] Name of the executable to fuzz.
echo  -m --mode ^<value^>        [%mode%] Name of mode, doubles as subfolder with seed tests and passed as
echo                           first arg to the executable. Project name is (executable)-(mode)
echo  -r --repo-root ^<value^>   Performs a sync at interval times with a git repo @ ^<value^>
echo                           If not .git repo, uses folder as sync root. Defaults to [%share_root%]
echo     --destroy             Clean fuzzing environment - stops and deletes everything.
echo     --name ^<value^>        [%instance_name%] Name of the instance.
echo  -C --config ^<value^>      [%build_config%] Build configuration (e.g. Debug, Release) to fuzz.
echo  -j --jobs ^<value^>        [%fuzzer_jobs%] Number of docker images to start. 0 == auto.
echo  -i --interval ^<value^>    [%fuzzer_interval%] Amount of time between resyncing (e.g. 2m, 24h, etc.).
echo  -l --lightweight         Perform lightweight minimization of corpus skipping tmin.
echo     --use-asan            Uses address sanitizer and not hardening and libdislocator. 
echo     --no-push             Does not perform a git push during sync.
echo     --destroy             Clean fuzzing environment - stops and deletes everything.
goto :eof

