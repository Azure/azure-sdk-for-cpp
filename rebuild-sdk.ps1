# Azure SDK for C++ Rebuild Script
# This script rebuilds the Azure SDK with the same configuration used previously

param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Release",
    
    [string]$InstallPrefix = "C:\Users\kraman\azure-sdk-local",
    
    [switch]$CleanBuild
)

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "Azure SDK for C++ Rebuild Script" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan
Write-Host ""

$RepoRoot = $PSScriptRoot
$BuildDir = Join-Path $RepoRoot "build"

# Detect Visual Studio installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    $vsVersion = & $vsWhere -latest -property installationVersion
    Write-Host "Detected Visual Studio: $vsVersion at $vsPath" -ForegroundColor Gray
}

# Use NMake Makefiles generator which works with any VS version via Developer PowerShell
$generator = "NMake Makefiles"

# Clean build if requested
if ($CleanBuild) {
    Write-Host "Cleaning previous build directory..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Ensure build directory exists
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure CMake with the same settings as before
Write-Host "Step 1: Configuring CMake..." -ForegroundColor Green
Write-Host "  Generator: $generator" -ForegroundColor Gray
Write-Host "  Build Type: $Config" -ForegroundColor Gray
Write-Host "  Install Prefix: $InstallPrefix" -ForegroundColor Gray
Write-Host ""

$cmakeConfigArgs = @(
    "-B", $BuildDir,
    "-S", $RepoRoot,
    "-G", $generator,
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DCMAKE_TOOLCHAIN_FILE=C:/Users/kraman/source/repos/vcpkg/scripts/buildsystems/vcpkg.cmake",
    "-DVCPKG_TARGET_TRIPLET=x64-windows",
    "-DBUILD_TESTING=OFF",
    "-DBUILD_SAMPLES=OFF",
    "-DBUILD_TRANSPORT_CURL=ON",
    "-DBUILD_TRANSPORT_WINHTTP=OFF",
    "-DWARNINGS_AS_ERRORS=ON",
    "-DBUILD_RTTI=ON",
    "-DMSVC_USE_STATIC_CRT=OFF",
    "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
)

& cmake @cmakeConfigArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 2: Building Azure Core library ($Config)..." -ForegroundColor Green

& cmake --build $BuildDir --target azure-core
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 3: Installing to $InstallPrefix..." -ForegroundColor Green

& cmake --install $BuildDir --prefix $InstallPrefix
if ($LASTEXITCODE -ne 0) {
    Write-Host "Installation failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "Build Complete!" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "The Azure SDK has been built and installed to:" -ForegroundColor White
Write-Host "  $InstallPrefix" -ForegroundColor Yellow
Write-Host ""
Write-Host "To use this in your project:" -ForegroundColor White
Write-Host "  1. Set CMAKE_PREFIX_PATH to: $InstallPrefix" -ForegroundColor Gray
Write-Host "  2. Use find_package(azure-core-cpp CONFIG REQUIRED)" -ForegroundColor Gray
Write-Host "  3. Link with Azure::azure-core" -ForegroundColor Gray
Write-Host ""
Write-Host "Build artifacts location:" -ForegroundColor White
Write-Host "  $BuildDir\sdk\core\azure-core\" -ForegroundColor Gray
Write-Host ""
