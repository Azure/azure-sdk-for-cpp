This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# testrunnerswitcher

testrunnerswitcher is a simple library to switch test runners between azure-ctest (available at https://github.com/Azure/azure-ctest.git) and CppUnitTest.

## Setup

- Clone azure-c-testrunnerswitcher by:
```
git clone --recursive https://github.com/Azure/azure-c-testrunnerswitcher.git
```
- Create a cmake folder under azure-c-testrunnerswitcher
- Switch to the cmake folder and run
   cmake ..
- Build the code for your platform (msbuild for Windows, make for Linux, etc.)

### Installation and Use
Optionally, you may choose to install testrunnerswitcher on your machine:

1. Switch to the *cmake* folder and run
    ```
    cmake --build . --target install
    ```
    or

    Linux:
    ```
    sudo make install
    ```

    Windows:
    ```
    msbuild /m INSTALL.vcxproj
    ```

2. Use it in your project (if installed)
    ```
    find_package(testrunnerswitcher REQUIRED CONFIG)
    target_link_library(yourlib testrunnerswitcher)
    ```