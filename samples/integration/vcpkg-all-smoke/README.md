# _Smoketest the Azure SDK for C++ into your application using vcpkg

This application instantiates all the sdk clients, and call one API from each in order to check their side by side capabilities

## _Pre-requisites

There are two options to set up the development environment:

### _Manual installation

Install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#third-party-dependencies).

- CMake project (min version 3.13).
- C++ version 14 or greater.

## _Build

### _Linux terminal

```bash
#
# _Building the application.
# _Instructions from application root directory.
#

# _Create build directory just the first time.
mkdir build
cd build

# _Generate and build
# _This code assumes that the SDK dependencies were installed with vcpkg
# _When using docker provided container, the TOOLCHAIN option is not required (cmake ..).
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

### _Windows VS

If you are using Visual Studio, the toolchain to link vcpkg is set with `CMakeSettings.json`. Update this file and set the vcpkg toolchain file for vcpkg (VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake). After setting the toolchain, VS can generate and build the sample. Use VS to open the sample folder only.

## _Run application

Review source code header for `environment variables` that must be set up before running the app.

```bash
#
# _Running the Application
# _Instructions from inside the build directory.
#

# _Run binary (.exe on Windows)
./smoketest-vcpkg
```
