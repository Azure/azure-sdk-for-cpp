# Azure Storage Client Library for C++

The Azure Storage Client Library for C++ allows you to build applications against Microsoft Azure Storage. For an overview of Azure Storage, see [Introduction to Microsoft Azure Storage](https://docs.microsoft.com/azure/storage/common/storage-introduction).

# Features

- Blobs
    - Create/Delete/List Containers
    - Create/Read/Update/Delete/List Blobs
- DataLake Gen 2
    - Create/Delete File Systems
    - Create/Delete Directories
    - Create/Read/Append/Flush/Delete Files
- File Shares
    - Create/Delete Shares
    - Create/Delete Directories
    - Create/Read/Delete Files

# Getting started

For the best development experience, we recommend that developers use the [CMake projects in Visual Studio](https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio?view=vs-2019) to view and build the source code together with its dependencies.

## Requirements

To call Azure services, you must first have an Azure subscription. Sign up for a [free trial](https://azure.microsoft.com/pricing/free-trial/) or use your [MSDN subscriber benefits](https://azure.microsoft.com/pricing/member-offers/msdn-benefits-details/).

## Need Help?

Be sure to check out the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/home?forum=windowsazuredata) on MSDN if you need help, or use [StackOverflow](https://stackoverflow.com/questions/tagged/azure).

## Collaborate & Contribute

We gladly accept community contributions.

- **Issues:** Report bugs on the [Issues page](https://github.com/Azure/azure-sdk-for-cpp/issues) in GitHub. Ideally, please add an "[Storage]" prefix to the title for easier categorizing.
- **Forums:** Communicate with the Azure Storage development team on the [Azure Storage Forum](https://social.msdn.microsoft.com/Forums/azure/home?forum=windowsazuredata) or [StackOverflow](https://stackoverflow.com/questions/tagged/azure).
- **Source Code Contributions:** Please follow the [contribution guidelines for Azure open source](https://azure.github.io/azure-sdk/cpp_introduction.html) for instructions about contributing to the source project.

For general suggestions about Azure, use our [Azure feedback forum](https://feedback.azure.com/forums/34192--general-feedback).

## Download & Install

### Install Dependencies

#### Windows

On Windows, dependencies are managed by [vcpkg](https://github.com/microsoft/vcpkg). You can reference the [Quick Start](https://github.com/microsoft/vcpkg#quick-start-windows) to quickly set yourself up.
After Vcpkg is initialized and bootstrapped, you can install the dependencies:
```BatchFile
vcpkg.exe install libxml2:x64-windows curl:x64-windows
```

#### Unix Platforms

You can use the package manager on different Unix platforms to install the dependencies. The dependencies to be installed are:

  - CMake 3.13.0 or higher.
  - libxml2.
  - OpenSSL.
  - libcurl.

### Build from Source

First, download the repository to your local folder:
```BatchFile
git clone https://github.com/Azure/azure-sdk-for-cpp.git
```

#### Windows

##### Use CMake to generate the solution file

In a new folder you created under the root directory:
```BatchFile
cmake .. -A x64 -DCMAKE_TOOLCHAIN_FILE=<YOUR_VCPKG_INSTALL_DIR>/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

The built library will be in `.\sdk\<ProjectDir>\<Configuration>\` respectively for Azure Core and Azure Storage. e.g. `azure_core.lib` will be in `.\sdk\core\azure-core\Debug` for debug configuration.

##### Use Visual Studio's Open by folder feature
Open the root folder of the library with Visual Studio's Open folder feature.

If Vcpkg is not globally integrated, then you need to open CMakeSettings.json and change the `Make toolchain file to be <YOUR_VCPKG_INSTALL_DIR>/scripts/buildsystems/vcpkg.cmake` and save.
Then you can build Azure Storage libraries by selecting the target in Visual Studio, or simply build all.
The libraries will be in `<ProjectRoot>\out\build\<Configuration>\sdk\<LibraryName>` respectively.

#### Unix Platforms

You can run the following command in a new folder created under the downloaded code's root folder to build the code.

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```
Then you can consume the built library with the header files.
make/ninja install is work in progress.


### Via NuGet
WIP
TODO when ready.

### Via Vcpkg
WIP
TODO when ready.

## Dependencies

  - [Azure Core SDK](https://github.com/Azure/azure-sdk-for-cpp/blob/master/README.md)
  - [nlohmann/json](https://github.com/nlohmann/json)
  - [libxml2](https://xmlsoft.org/)

## Code Samples

To get started with the coding, please visit the following code samples:
- [How to use Blob Storage from C++](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/storage/azure-storage-blobs/sample/blob_getting_started.cpp)
- [How to use DataLake Gen 2 Storage from C++](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/storage/azure-storage-files-datalake/sample/datalake_getting_started.cpp)
- [How to use File Storage from C++](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/storage/azure-storage-files-shares/sample/file_share_getting_started.cpp)
