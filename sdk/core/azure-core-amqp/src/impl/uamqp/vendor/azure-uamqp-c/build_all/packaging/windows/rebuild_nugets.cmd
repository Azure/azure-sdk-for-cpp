@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

rem -----------------------------------------------------------------------------
rem -- setup path information
rem -----------------------------------------------------------------------------

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

echo Current Path: %current-path%

set build-root=%current-path%\..\..\..
rem // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

set client-root=%current-path%\..\..\..
for %%i in ("%client-root%") do set client-root=%%~fi

rem -----------------------------------------------------------------------------
rem -- check prerequisites and clean directories
rem -----------------------------------------------------------------------------

where /q nuget.exe
if not !errorlevel! == 0 (
@Echo Azure Amqp needs to download nuget.exe from https://www.nuget.org/nuget.exe 
@Echo https://www.nuget.org 
choice /C yn /M "Do you want to download and run nuget.exe?" 
if not !errorlevel!==1 goto :eof
rem if nuget.exe is not found, then ask user
Powershell.exe wget -outf nuget.exe https://nuget.org/nuget.exe
	if not exist .\nuget.exe (
		echo nuget does not exist
		exit /b 1
	)
)

set build-path=%build-root%\cmake

if exist %build-path%\azure_amqp_output (
	rmdir /s/q %build-path%\azure_amqp_output
	rem no error checking
)

rem -----------------------------------------------------------------------------
rem -- build project
rem -----------------------------------------------------------------------------

call %build-root%\build_all\windows\build.cmd --make_nuget yes

rem -----------------------------------------------------------------------------
rem -- Copy Win32 binaries
rem -----------------------------------------------------------------------------

rem -- Copy all Win32 files from cmake build directory to the repo directory
echo copying %build-path%\uamqp_win32\win32\debug

rem -- Copy all Win32 files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_win32\Debug\*.* %build-path%\azure_amqp_output\win32\debug\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

rem -- Copy all Win32 Release files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_win32\Release\*.* %build-path%\azure_amqp_output\win32\Release\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

rem -----------------------------------------------------------------------------
rem -- build with CMAKE x64
rem -----------------------------------------------------------------------------

rem -- Copy all x64 files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_x64\Debug\*.* %build-path%\azure_amqp_output\x64\debug\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

rem -- Copy all x64 Release files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_x64\Release\*.* %build-path%\azure_amqp_output\x64\Release\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

rem -----------------------------------------------------------------------------
rem -- build with CMAKE ARM
rem -----------------------------------------------------------------------------

rem -- Copy all ARM files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_arm\Debug\*.* %build-path%\azure_amqp_output\arm\debug\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

rem -- Copy all x64 Release files from cmake build directory to the repo directory
xcopy /q /y /R %build-path%\uamqp_arm\Release\*.* %build-path%\azure_amqp_output\arm\Release\*.*
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

if exist *.nupkg (
	del *.nupkg
)

rem -- Package Nuget
nuget pack %build-root%\build_all\packaging\windows\Microsoft.Azure.uamqp.nuspec -OutputDirectory %build-root%\build_all\packaging\windows

rmdir /s/q %build-path%\azure_amqp_output

popd
goto :eof

echo done