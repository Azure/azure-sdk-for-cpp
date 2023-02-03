# Azure Storage Common Client Library for C++

Azure Storage is a Microsoft-managed service providing cloud storage that is highly available, secure, durable, scalable, and redundant. Azure Storage includes Azure Blobs (objects), Azure Data Lake Storage Gen2, Azure Files, and Azure Queues.
The Azure Storage Common library provides infrastructure shared by the other Azure Storage client libraries.

## Getting Started

### Install the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```batch
vcpkg install azure-storage-common-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-storage-common-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-common)
```

### Prerequisites

You need an Azure subscription and a [Storage Account][storage_account_overview] to use this package.

To create a new Storage Account, you can use the [Azure Portal][create_account_with_azure_portal], [Azure PowerShell][create_account_with_powershell], or the [Azure CLI][create_account_with_azure_cli].

### Build from Source

First, download the repository to your local folder:

```batch
git clone https://github.com/Azure/azure-sdk-for-cpp.git
```

Create a new folder under the root directory of local cloned repo, switch into this folder and run below commands:

Windows:

```batch
cmake .. -A x64
cmake --build . --target azure-storage-common
```

or Unix:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target azure-storage-common
```

## Key concepts

The Azure Storage Common client library contains shared infrastructure like authentication credentials and exception types.

## Contributing

See the [Storage CONTRIBUTING.md][storage_contrib] for details on building,
testing, and contributing to these libraries.

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit [cla.microsoft.com][cla].

This project has adopted the [Microsoft Open Source Code of Conduct][coc].
For more information see the [Code of Conduct FAQ][coc_faq]
or contact [opencode@microsoft.com][coc_contact] with any
additional questions or comments.

<!-- LINKS -->
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[storage_account_overview]: https://learn.microsoft.com/azure/storage/common/storage-account-overview
[create_account_with_azure_portal]: https://learn.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-portal
[create_account_with_powershell]: https://learn.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-powershell
[create_account_with_azure_cli]: https://learn.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-cli
[storage_contrib]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com