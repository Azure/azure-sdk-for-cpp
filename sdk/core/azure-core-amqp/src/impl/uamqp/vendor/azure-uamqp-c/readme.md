This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# uAMQP

`uAMQP` is a general purpose C library for AMQP 1.0.

The goals are:

- Compliance with the standard
- Optimizing for low RAM footprint
- Be as portable as possible

It is currently mostly a client side implementation only.
Although much of the standard is symmetrical, there are parts that are asymmetrical, like the SASL handshake.

The server side support of `uAMQP` (for example for SASL) is currently work in progress.

## Dependencies

`uAMQP` uses `azure-c-shared-utility`, which is a C library providing common functionality for basic tasks (string manipulation, list manipulation, IO, etc.).
`azure-c-shared-utility` is available here: https://github.com/Azure/azure-c-shared-utility and it is used as a submodule.

Please note that azure-c-shared-utility in turn depends on several libraries (libssl-dev, libuuid-dev, libcurl-dev).

On an Ubuntu distro it is recommended to install all needed packages by running:

```
  sudo apt-get update
  sudo apt-get install -y git cmake build-essential curl libcurl4-openssl-dev libssl-dev uuid-dev
```

azure-c-shared-utility provides several tlsio implementations, some being:
- tlsio_schannel - runs only on Windows
- tlsio_openssl - depends on OpenSSL being installed
- tlsio_wolfssl - depends on WolfSSL being installed
- tlsio_mbedtls
- ...

For more information about configuring `azure-c-shared-utility` see https://github.com/Azure/azure-c-shared-utility.

`uAMQP` uses cmake for configuring build files.

For WebSockets support `uAMQP` depends on the support provided by azure-c-shared-utility.

## Setup

### Build

- Clone `azure-uamqp-c` by:

```
git clone --recursive https://github.com/Azure/azure-uamqp-c.git
```

- Create a folder named `cmake` under `azure-uamqp-c`

- Switch to the `cmake` folder and run

```
cmake ..
```

- Build

```
cmake --build .
```

### Installation and Use

Optionally, you may choose to install azure-uamqp-c on your machine:

1. Switch to the `cmake` folder and run
    ```
    cmake -Duse_installed=ON ../
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
    find_package(uamqp REQUIRED CONFIG)
    target_link_library(yourlib uamqp)
    ```

_This requires that azure-c-shared-utility is installed (through CMake) on your machine._

_If running tests, this requires that umock-c, azure-ctest, and azure-c-testrunnerswitcher are installed (through CMake) on your machine._

### Building the tests

In order to build the unit tests use:

```
cmake .. -Drun_unittests:bool=ON
```

In order to build the end to end tests use:

```
cmake .. -Drun_e2e_tests:bool=ON
```

Please note that some end to end tests (talking to Event Hubs or IoT Hubs) require setup of environment variables so that the tests have the information about the endpoints that they need to connect to.

## Switching branches

After any switch of branches (git checkout for example), one should also update the submodule references by:

```
git submodule update --init --recursive
```

## Samples

Samples are available in the azure-uamqp-c/samples folder:

- Send messages to an Event Hub
- Receive messages from an Event Hub
- Send messages to an IoT Hub using CBS
- Send messages to an IoT Hub using AMQP over WebSockets
- Simple client/server sample using raw TCP
