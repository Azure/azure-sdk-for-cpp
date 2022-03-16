# Smoketest the Azure SDK for C++ into your application using vcpkg

This application instantiates all the sdk clients, and call one API from each in order to check their side by side capabilities

## Pre-requisites

There are two options to set up the development environment:

### Manual installation

Install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#third-party-dependencies).

- CMake project (min version 3.13).
- C++ version 14 or greater.

## Build

### Linux terminal

```bash
#
# Building the application.
# Instructions from application root directory.
#

# Create build directory just the first time.
mkdir build
cd build

# Generate and build
# This code assumes that the SDK dependencies were installed with vcpkg
# When using docker provided container, the TOOLCHAIN option is not required (cmake ..).
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

### Windows VS

If you are using Visual Studio, the toolchain to link vcpkg is set with `CMakeSettings.json`. Upate this file and set the vcpkg toolchain file for vcpkg (VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake). After setting the toolchain, VS can generate and build the sample. Use VS to open the sample folder only.

## Run application

Review source code header for `environment variables` that must be set up before running the app.

```bash
#
# Running the Application
# Instructions from inside the build directory.
#

# Run binary (.exe on Windows)
./smoketest-vcpkg
```
