# _Integrating the Azure SDK for C++ into your application using CMake fetch content

This application shows how to integrate the Azure SDK for C++ in your application. It uses CMake fetch content functionality to clone the Azure SDK for C++ repo into your `build` directory. This approach is useful when using CMake to build your application.

## _Pre-requisites

Install the [Azure SDK for C++ dependencies](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#third-party-dependencies).

- CMake project (min version 3.13).
- C++ version 14 or greater.

## _Build

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
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake -build .
```

## _Run application

Review source code header for `environment variables` that must be set up before running the app.

```bash
#
# _Running the Application
# _Instructions from inside the build directory.
#

# _Run binary (.exe on Windows)
./application
```
