# Microsoft Azure IoT SDKs - macro utils for C

This repository contains the following:
* **a generation tool for macro_utils.h** to regenerate the file macro_utils.h. Pressing F5 after opening .sln file will regenerate macro_utils.h
* **macro_utils.h** a C header file that contains a multitude of very useful C macros

## How to clone the repository

```
git clone https://github.com/Azure/azure-macro-utils-c.git 
```

## How to use the Azure IoT SDKs - macro utils for C

In a C source file: `#include "azure_macro_utils/macro_utils.h"`

### Installation

To install the Azure IoT SDKs - macro utils for C on your machine, we recommend you use cmake 3.0.0 or higher.

- Create a "build" directory and run `cmake ..` from that directory to generate the cmake files.
- Run `cmake --build . --target install` to install on your machine

#### CMake 3.0.0+
In your project, simply:
- `find_package(azure_macro_utils_c REQUIRED CONFIG)`
- `target_link_libraries(<your_target> azure_macro_utils_c)`

#### CMake 2.\*.\*
In your project, simply:
- `target_include_directories(<your_target> PUBLIC ${MACRO_UTILS_INC_FOLDER})`

## Supported compilers

The following compilers are supported: gcc 4.4 (and onwards), Visual Studio 2015 and others.

## CMake integration

The project will export a global variable MACRO_UTILS_INC_FOLDER that points to the folder needed to be included so that `#include "azure_macro_utils/macro_utils.h"` resolves properly.