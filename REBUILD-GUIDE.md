# Rebuild Guide for Azure SDK for C++

This guide explains how to rebuild your forked version of the Azure SDK for C++ so it can be used from other projects.

## Quick Start

### Option 1: Rebuild Just Azure Core (Fastest)

```powershell
.\rebuild-sdk.ps1 -Config Release
```

This builds only the `azure-core` library, which is the foundation that most other projects need.

### Option 2: Rebuild All SDK Components

```powershell
.\rebuild-sdk-full.ps1 -Config Release
```

This builds all Azure SDK components (Storage, KeyVault, Identity, etc.).

### Option 3: Rebuild Specific Components

```powershell
.\rebuild-sdk-full.ps1 -Config Release -Components "azure-core", "azure-storage-blobs"
```

## Script Parameters

Both scripts support these parameters:

- **`-Config`**: Build configuration (Debug, Release, RelWithDebInfo)
  - Default: `Release`
  - Example: `-Config Debug`

- **`-InstallPrefix`**: Where to install the SDK
  - Default: `C:\Users\kraman\azure-sdk-local`
  - Example: `-InstallPrefix "C:\MyCustomPath"`

- **`-CleanBuild`**: Remove previous build files first
  - Example: `-CleanBuild`

## Build Configuration Summary

The SDK is configured with these settings (matching your previous build):

| Setting | Value |
|---------|-------|
| Generator | Visual Studio 17 2022 |
| Platform | x64 |
| Toolchain | vcpkg |
| CURL Transport | Enabled |
| WinHTTP Transport | Disabled |
| Testing | Disabled |
| Samples | Disabled |
| RTTI | Enabled |
| Static CRT | Disabled |
| Warnings as Errors | Enabled |

## Using the Built SDK in Your Project

### CMakeLists.txt Example

```cmake
cmake_minimum_required(VERSION 3.13)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Point to your local SDK installation
set(CMAKE_PREFIX_PATH "C:/Users/kraman/azure-sdk-local")

# Find the packages you need
find_package(azure-core-cpp CONFIG REQUIRED)
# find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
# find_package(azure-identity-cpp CONFIG REQUIRED)

add_executable(myapp main.cpp)

target_link_libraries(myapp 
    PRIVATE 
    Azure::azure-core
    # Azure::azure-storage-blobs
    # Azure::azure-identity
)
```

### Building Your Project

```powershell
# Configure
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release

# Run
.\build\Release\myapp.exe
```

## Manual Build Steps

If you prefer to build manually:

### 1. Configure CMake

```powershell
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/Users/kraman/source/repos/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DBUILD_TESTING=OFF `
  -DBUILD_SAMPLES=OFF `
  -DBUILD_TRANSPORT_CURL=ON `
  -DCMAKE_INSTALL_PREFIX=C:\Users\kraman\azure-sdk-local
```

### 2. Build

```powershell
# Just azure-core
cmake --build build --config Release --target azure-core

# Or all components
cmake --build build --config Release
```

### 3. Install

```powershell
cmake --install build --prefix C:\Users\kraman\azure-sdk-local --config Release
```

## Build Artifacts

After building, you'll find:

- **Build files**: `build/sdk/core/azure-core/Release/`
  - `azure-core.lib` - Static library
  - `azure-core.dll` - Shared library (if applicable)

- **Installed files**: `C:\Users\kraman\azure-sdk-local/`
  - `include/` - Header files
  - `lib/` - Library files
  - `share/` - CMake package configuration files

## Troubleshooting

### vcpkg Not Found

If you get vcpkg errors, verify the path:
```powershell
Test-Path C:\Users\kraman\source\repos\vcpkg\scripts\buildsystems\vcpkg.cmake
```

If incorrect, update the path in the scripts or use:
```powershell
$env:VCPKG_ROOT = "C:\path\to\your\vcpkg"
```

### Missing Dependencies

If vcpkg dependencies are missing:
```powershell
cd C:\Users\kraman\source\repos\vcpkg
.\vcpkg install curl openssl --triplet x64-windows
```

### Clean Rebuild

If you encounter issues, try a clean rebuild:
```powershell
.\rebuild-sdk.ps1 -Config Release -CleanBuild
```

## Verifying the Build

To verify your SDK is built correctly:

```powershell
# Check if libraries exist
Test-Path "C:\Users\kraman\azure-sdk-local\lib\azure-core.lib"

# Check if headers exist
Test-Path "C:\Users\kraman\azure-sdk-local\include\azure\core\http\http.hpp"

# Check if CMake config exists
Test-Path "C:\Users\kraman\azure-sdk-local\share\azure-core-cpp\azure-core-cppConfig.cmake"
```

## Next Steps

After rebuilding:

1. **Test the build** - Create a simple test project to verify the SDK works
2. **Update your projects** - Point your dependent projects to the new installation
3. **Document changes** - If you've made custom modifications, document them

## Contact & Support

For issues specific to your modifications, refer to your team's documentation.
For general Azure SDK issues, see: https://github.com/Azure/azure-sdk-for-cpp
