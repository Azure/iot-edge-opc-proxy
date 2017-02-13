@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

call ..\bld\build.cmd -c %*
if errorlevel 1 exit /b !errorlevel!
