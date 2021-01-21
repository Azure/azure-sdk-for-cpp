# Integrating the Azure SDK for C++ with CMake fetch content to your application

This application is an example of integrating the Azure SDK for C++ to your application. It uses the CMake fetch content functionality to automatically clone the Azure SDK for C++ repo to your `build` directory. This approach is useful if you are using CMake to build your application.

## Pre-requisites

Make sure to install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#third-party-dependencies) first.

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
# This code assumes that the SDK dependencies were installed with VCPKG
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

## Run application

Review source code header to learn if there is any `environment variable` that needs to be set up before running the app.

```bash
#
# Running the Application
# Instructions from inside the build directory.
#

# Run binary (.exe on Windows)
./applicationName
```
