# Azure SDK for C++ Build Script with VS Environment Setup
# This script sets up the Visual Studio environment and builds the SDK

param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Release",
    
    [string]$InstallPrefix = "C:\Users\kraman\azure-sdk-local",
    
    [switch]$CleanBuild
)

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "Azure SDK for C++ Build Script" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host ""

# Find Visual Studio installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
    Write-Host "Visual Studio not found! Please install Visual Studio 2022 or later." -ForegroundColor Red
    exit 1
}

$vsPath = & $vsWhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    Write-Host "Visual Studio C++ tools not found! Please install C++ workload." -ForegroundColor Red
    exit 1
}

Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green

# Import Visual Studio environment (x64 Native Tools)
$vcVarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcVarsPath)) {
    Write-Host "vcvars64.bat not found at: $vcVarsPath" -ForegroundColor Red
    exit 1
}

Write-Host "Setting up Visual Studio environment..." -ForegroundColor Yellow

# Run vcvars64.bat and capture environment variables
$cmd = "`"$vcVarsPath`" && set"
$envVars = cmd /c $cmd

# Parse and set environment variables
foreach ($line in $envVars) {
    if ($line -match "^([^=]+)=(.*)$") {
        $name = $matches[1]
        $value = $matches[2]
        Set-Item -Path "env:$name" -Value $value -ErrorAction SilentlyContinue
    }
}

Write-Host "Visual Studio environment configured." -ForegroundColor Green
Write-Host ""

# Now proceed with build
$RepoRoot = $PSScriptRoot
$BuildDir = Join-Path $RepoRoot "build"

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

# Configure CMake
Write-Host "Step 1: Configuring CMake..." -ForegroundColor Green
Write-Host "  Generator: NMake Makefiles" -ForegroundColor Gray
Write-Host "  Build Type: $Config" -ForegroundColor Gray
Write-Host "  Install Prefix: $InstallPrefix" -ForegroundColor Gray
Write-Host ""

$vcpkgPath = "C:\Users\kraman\source\repos\vcpkg\scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $vcpkgPath)) {
    Write-Host "Warning: vcpkg toolchain not found at $vcpkgPath" -ForegroundColor Yellow
    Write-Host "Trying to build without vcpkg..." -ForegroundColor Yellow
    $vcpkgPath = $null
}

$cmakeConfigArgs = @(
    "-B", $BuildDir,
    "-S", $RepoRoot,
    "-G", "NMake Makefiles",
    "-DCMAKE_BUILD_TYPE=$Config"
)

if ($vcpkgPath) {
    $cmakeConfigArgs += "-DCMAKE_TOOLCHAIN_FILE=$vcpkgPath"
    $cmakeConfigArgs += "-DVCPKG_TARGET_TRIPLET=x64-windows"
}

$cmakeConfigArgs += @(
    "-DBUILD_TESTING=OFF",
    "-DBUILD_SAMPLES=OFF",
    "-DBUILD_TRANSPORT_CURL=ON",
    "-DBUILD_TRANSPORT_WINHTTP=ON",
    "-DWARNINGS_AS_ERRORS=OFF",
    "-DBUILD_RTTI=ON",
    "-DMSVC_USE_STATIC_CRT=ON",
    "-DCMAKE_CXX_FLAGS=/DCURL_STATICLIB /DWIL_ENABLE_EXCEPTIONS",
    "-DDISABLE_RUST_IN_BUILD=ON",
    "-DDISABLE_AMQP=ON",
    "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
)

& cmake @cmakeConfigArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host ""
Write-Host "Step 2: Building Azure Core library..." -ForegroundColor Green

& cmake --build $BuildDir --target azure-core
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Install only azure-core component (ignore errors from other components)
Write-Host ""
Write-Host "Step 3: Installing azure-core to $InstallPrefix..." -ForegroundColor Green

# Install just the azure-core target by navigating to its build directory
Push-Location "$BuildDir\sdk\core\azure-core"
& cmake --install . --prefix $InstallPrefix --component azure-core 2>&1 | Out-Null
$installExitCode = $LASTEXITCODE
Pop-Location

# Also try the main install (will partially succeed for azure-core)
& cmake --install $BuildDir --prefix $InstallPrefix 2>&1 | Out-Null

# Verify azure-core was installed successfully
if (-not (Test-Path "$InstallPrefix\lib\azure-core.lib")) {
    Write-Host "Installation failed! azure-core.lib not found." -ForegroundColor Red
    exit 1
}

Write-Host "Azure Core installed successfully (other component errors ignored)." -ForegroundColor Yellow

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
