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

if not exist %build-root%/version.txt (
		echo Version.txt does not exist
		exit /b 1
)

set /p Version=<%build-root%/version.txt

git tag -a "v%Version%" -m "Release of v%Version%"
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!

git push --tags
if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!