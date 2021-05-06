# Integrating the Azure SDK for C++ into your application using VCPKG

This application shows how to integrate the Azure SDK for C++ in your application. It uses VCPKG to adquire and build the Azure SDK for C++ client libraries. Your CMake project needs to link the libraries from VCPKG by setting the toolchain file to VCPKG (shown below).

## Pre-requisites

There are two options to set up the development environment:

### Manual installation

Install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#third-party-dependencies).

- CMake project (min version 3.13).
- C++ version 14 or greater.

### Container

The sample provides a .devcontainer folder which can be used by VSCode to build and run a docker container with the required C++ build tools and with VCPKG installed.

This method requires VSCode + [Remote Container](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) to be installed in the system. Also make sure to have Docker installed and running. This method works for any operating system where Docker and VSCode is supported like Windows, Linux and MacOS. The development environment will be Linux debian 10.

- Open vcpkg folder in VSCode.
- VSCode will detect the `devcontainer` configuration and ask you if you would like to re-open the folder in a container. Click Yes.
- If VSCode did not ask, you can press F1 and type `Reopen in container` option.

Once VSCode builds and run the container, open the terminal and continue to build step.

> Note: The container is set up to automatically link VCPKG to CMake projects by setting env variables that the CMake sample project will use to set the toolchain.

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
# This code assumes that the SDK dependencies were installed with VCPKG
# When using docker provided container, the TOOLCHAIN option is not required (cmake ..).
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

### Windows VS

If you are using Visual Studio, the toolchain to link VCPKG is set with `CMakeSettings.json`. Upate this file and set the VCPKG toolchain file for VCPKG (VCPKG_ROO\scripts\buildsystems\vcpkg.cmake). After setting the toolchain, VS can generate and build the sample. Use VS to open the sample folder only.

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
