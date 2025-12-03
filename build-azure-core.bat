@echo off
REM Build azure-core library with your CurlOptionsCallback changes
REM Run this from Developer Command Prompt for VS Insiders

echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat"

cd /d %~dp0

echo.
echo Building azure-core library...
echo.

REM Create build directory
if not exist sdk\core\azure-core\mybuild mkdir sdk\core\azure-core\mybuild
cd sdk\core\azure-core\mybuild

REM Configure with minimal dependencies
cmake .. -G "NMake Makefiles" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_TESTING=OFF ^
    -DAZURE_SDK_DISABLE_AUTO_VCPKG=ON ^
    -DBUILD_TRANSPORT_CURL=ON ^
    -DCMAKE_INSTALL_PREFIX=C:\Users\kraman\azure-sdk-local

if errorlevel 1 (
    echo Configuration failed!
    pause
    exit /b 1
)

echo.
echo Building...
nmake

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Installing to C:\Users\kraman\azure-sdk-local...
nmake install

echo.
echo Build complete!
echo Library installed to: C:\Users\kraman\azure-sdk-local
echo.
pause
