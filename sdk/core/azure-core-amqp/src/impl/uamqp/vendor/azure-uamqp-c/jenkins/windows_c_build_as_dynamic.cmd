@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal
@echo off

set cmake_dir=dynamic_build
set build-root=%~dp0..
@REM // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

if EXIST %build-root%\cmake\%cmake_dir% (
    rmdir /s/q %build-root%\cmake\%cmake_dir%
)
mkdir %build-root%\cmake\%cmake_dir%
if errorlevel 1 goto :eof

cd %build-root%\cmake\%cmake_dir%

cmake -Dskip_samples=ON -Dbuild_as_dynamic=ON ..\..
if errorlevel 1 goto :eof

cmake --build . -- /m
if errorlevel 1 goto :eof
