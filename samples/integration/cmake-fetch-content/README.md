# Integrating the Azure SDK for C++ into your application using CMake fetch content

This application shows how to integrate the Azure SDK for C++ in your application. It uses CMake fetch content functionality to clone the Azure SDK for C++ repo into your `build` directory. This approach is useful when using CMake to build your application.

## Pre-requisites

Install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#third-party-dependencies).

- CMake project (min version 3.13).
- C++ version 14 or greater.

## Build

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
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

## Run application

Review source code header for `environment variables` that must be set up before running the app.

```bash
#
# Running the Application
# Instructions from inside the build directory.
#

# Run binary (.exe on Windows)
./application
```