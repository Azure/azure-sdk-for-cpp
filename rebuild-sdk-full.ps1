# Azure SDK for C++ Full Rebuild Script
# This script rebuilds ALL Azure SDK components

param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Release",
    
    [string]$InstallPrefix = "C:\Users\kraman\azure-sdk-local",
    
    [switch]$CleanBuild,
    
    [string[]]$Components = @()  # Empty = build all, or specify like: "azure-core", "azure-storage-blobs"
)

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "Azure SDK for C++ Full Rebuild Script" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host ""

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
Write-Host "  Generator: Visual Studio 17 2022" -ForegroundColor Gray
Write-Host "  Platform: x64" -ForegroundColor Gray
Write-Host "  Build Type: $Config" -ForegroundColor Gray
Write-Host "  Install Prefix: $InstallPrefix" -ForegroundColor Gray
Write-Host ""

$cmakeConfigArgs = @(
    "-B", $BuildDir,
    "-S", $RepoRoot,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
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

# Build
Write-Host ""
if ($Components.Count -eq 0) {
    Write-Host "Step 2: Building ALL Azure SDK components ($Config)..." -ForegroundColor Green
    & cmake --build $BuildDir --config $Config
} else {
    Write-Host "Step 2: Building specified components ($Config)..." -ForegroundColor Green
    foreach ($component in $Components) {
        Write-Host "  Building: $component" -ForegroundColor Yellow
        & cmake --build $BuildDir --config $Config --target $component
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Build failed for $component!" -ForegroundColor Red
            exit 1
        }
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Install
Write-Host ""
Write-Host "Step 3: Installing to $InstallPrefix..." -ForegroundColor Green

& cmake --install $BuildDir --prefix $InstallPrefix --config $Config
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
Write-Host "Available packages:" -ForegroundColor White
Write-Host "  - azure-core-cpp" -ForegroundColor Gray
Write-Host "  - azure-storage-blobs-cpp" -ForegroundColor Gray
Write-Host "  - azure-storage-files-datalake-cpp" -ForegroundColor Gray
Write-Host "  - azure-storage-files-shares-cpp" -ForegroundColor Gray
Write-Host "  - azure-identity-cpp" -ForegroundColor Gray
Write-Host "  - azure-keyvault-keys-cpp" -ForegroundColor Gray
Write-Host "  - and more..." -ForegroundColor Gray
Write-Host ""
Write-Host "To use in your project:" -ForegroundColor White
Write-Host "  1. Set CMAKE_PREFIX_PATH to: $InstallPrefix" -ForegroundColor Gray
Write-Host "  2. Use find_package(<package-name> CONFIG REQUIRED)" -ForegroundColor Gray
Write-Host "  3. Link with Azure::<target>" -ForegroundColor Gray
Write-Host ""
