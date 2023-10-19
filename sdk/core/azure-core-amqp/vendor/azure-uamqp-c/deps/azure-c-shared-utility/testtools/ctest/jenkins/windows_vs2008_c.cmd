@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion

set build-root=%~dp0..
rem // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

@echo off

set repo_root=%build-root%\..
rem // resolve to fully qualified path
for %%i in ("%repo_root%") do set repo_root=%%~fi

echo Build Root: %build-root%
echo Repo Root: %repo_root%

set CMAKE_DIR=ctest_win32

rem -----------------------------------------------------------------------------
rem -- build with CMAKE and run tests
rem -----------------------------------------------------------------------------

echo CMAKE Output Path: %build-root%\cmake\%CMAKE_DIR%

if EXIST %build-root%\cmake\%CMAKE_DIR% (
    rmdir /s/q %build-root%\cmake\%CMAKE_DIR%
    rem no error checking
)

echo %build-root%\cmake\%CMAKE_DIR%
mkdir %build-root%\cmake\%CMAKE_DIR%
rem no error checking
pushd %build-root%\cmake\%CMAKE_DIR%

echo ***Running CMAKE for Win32***   
cmake %build-root% -Drun_unittests:BOOL=ON -Duse_cppunittest:bool=OFF
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!

call :_run-msbuild "Build" ctest.sln
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
    
ctest -C "debug" -V
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!

popd
goto :eof

rem -----------------------------------------------------------------------------
rem -- subroutines
rem -----------------------------------------------------------------------------

:_run-msbuild
rem // optionally override configuration|platform
setlocal EnableExtensions
set build-target=
if "%~1" neq "Build" set "build-target=/t:%~1"

msbuild /m %build-target% "/p:Configuration=Debug;Platform=Win32" %2
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
goto :eof

echo done
