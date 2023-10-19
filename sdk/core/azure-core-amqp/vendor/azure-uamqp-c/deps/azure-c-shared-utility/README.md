This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# Azure C Shared Utility

azure-c-shared-utility is a C library providing common functionality for basic tasks (like string, list manipulation, IO, etc.).

## Dependencies

azure-c-shared-utility provides 3 tlsio implementations:
- tlsio_schannel - runs only on Windows
- tlsio_openssl - depends on OpenSSL being installed
- tlsio_wolfssl - depends on WolfSSL being installed 

azure-c-shared-utility depends on curl for HTTPAPI for Linux.

azure-c-shared-utility uses cmake for configuring build files.

## Setup

1. Clone **azure-c-shared-utility** using the recursive option:

```
git clone --recursive https://github.com/Azure/azure-c-shared-utility.git
```

2. Create a folder called *cmake* under *azure-c-shared-utility*

3. Switch to the *cmake* folder and run
```
cmake ..
```

4. Build

```
cmake --build .
```

### Installation and Use
Optionally, you may choose to install azure-c-shared-utility on your machine:

1. Switch to the *cmake* folder and run
    ```
    cmake -Duse_installed_dependencies=ON ../
    ```
    ```
    cmake --build . --target install
    ```

    or install using the follow commands for each platform:

    On Linux:
    ```
    sudo make install
    ```

    On Windows:
    ```
    msbuild /m INSTALL.vcxproj
    ```

2. Use it in your project (if installed)
    ```
    find_package(azure_c_shared_utility REQUIRED CONFIG)
    target_link_library(yourlib aziotsharedutil)
    ```

_If running tests, this requires that umock-c, azure-ctest, and azure-c-testrunnerswitcher are installed (through CMake) on your machine._

### Building the tests

In order to build the tests use:

```
cmake .. -Drun_unittests:bool=ON
```

## Configuration options

In order to turn on/off the tlsio implementations use the following CMAKE options:

* `-Duse_schannel:bool={ON/OFF}` - turns on/off the SChannel support
* `-Duse_openssl:bool={ON/OFF}` - turns on/off the OpenSSL support. If this option is used, an environment variable named OpenSSLDir should be set to point to the OpenSSL folder.
* `-Duse_wolfssl:bool={ON/OFF}` - turns on/off the WolfSSL support. If this option is used, an environment variable named WolfSSLDir should be set to point to the WolfSSL folder.
* `-Duse_http:bool={ON/OFF}` - turns on/off the HTTP API support. 
* `-Duse_installed_dependencies:bool={ON/OFF}` - turns on/off building azure-c-shared-utility using installed dependencies. This package may only be installed if this flag is ON.
* `-Drun_unittests:bool={ON/OFF}` - enables building of unit tests. Default is OFF.


## Porting to new devices

Instructions for porting the Azure IoT C SDK to new devices are located
[here](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/porting_guide.md).
