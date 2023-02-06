# Azure Storage Files Shares Client Library for C++

Azure File Shares offers fully managed file shares in the cloud that are accessible via the industry standard Server Message Block (SMB) protocol. Azure file shares can be mounted concurrently by cloud or on-premises deployments of Windows, Linux, and macOS. Additionally, Azure file shares can be cached on Windows Servers with Azure File Sync for fast access near where the data is being used.

## Getting started

### Install the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```batch
vcpkg install azure-storage-files-shares-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-storage-files-shares-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-files-shares)
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
cmake --build . --target azure-storage-files-shares
```

or Unix:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target azure-storage-files-shares
```

## Key concepts

Azure file shares can be used to:

- Completely replace or supplement traditional on-premises file servers or NAS devices.
- "Lift and shift" applications to the cloud that expect a file share to store file application or user data.
- Simplify new cloud development projects with shared application settings, diagnostic shares, and Dev/Test/Debug tool file shares.

Learn more about options for authentication (including Connection Strings, Shared Key, Shared Key Signatures, Active Directory, and anonymous public access) in our [samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-shares/samples).

### Thread safety

We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

Client Options | [Accessing the response](https://github.com/Azure/azure-sdk-for-cpp#response-t-model-types) | [Long-running operations](https://github.com/Azure/azure-sdk-for-cpp#long-running-operations) | Handling failures

## Examples

### Create a share and upload a file

```C++
const std::string shareName = "sample-share";
const std::string directoryName = "sample-directory";
const std::string fileName = "sample-file";
const std::string localFilePath = "<path_to_local_file>";

// Get a reference to a share and then create it
ShareClient shareClient = ShareClient::CreateFromConnectionString(connectionString, shareName);
shareClient.CreateIfNotExists();

// Get a reference to a directory and create it
ShareDirectoryClient directoryClient = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);;
directoryClient.CreateIfNotExists();

// Get a reference to a file and upload it
ShareFileClient fileClient = directoryClient.GetFileClient(fileName);
// upload from local file
fileClient.UploadFrom(localFilePath);
// or upload from memory buffer
fileClient.UploadFrom(bufferPtr, bufferLength);
```

### Download a file

```C++
// download to local file
fileClient.DownloadTo(localFilePath);
// or download to memory buffer
fileClient.DownloadTo(bufferPtr, bufferLength);
```

### Traverse a share

```C++
std::vector<ShareDirectoryClient> remaining;
remaining.push_back(shareClient.GetRootDirectoryClient());
while (remaining.size() > 0)
{
  auto& directoryClient = remaining.back();
  remaining.pop_back();
  for (auto page = directoryClient.ListFilesAndDirectories(); page.HasPage();
       page.MoveToNextPage())
  {
    for (auto& file : page.Files)
    {
          std::cout << "file: " << file.Name << std::endl;
    }
    for (auto& directory : page.Directories)
    {
      std::cout << "directory: " << directory.Name << std::endl;
      remaining.push_back(directoryClient.GetSubdirectoryClient(directory.Name));
    }
  }
}
```

## Troubleshooting

All Azure Storage File Shares service operations will throw a [StorageException](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure with helpful [ErrorCode](https://learn.microsoft.com/rest/api/storageservices/file-service-error-codes)s.
Many of these errors are recoverable.

```C++
try
{
  shareClient.Delete();
}
catch (Azure::Storage::StorageException& e)
{
  if (e.ErrorCode == "ShareNotFound")
  {
    // ignore the error if the file share does not exist.
  }
  else
  {
    // handle other errors here
  }
}
```

## Next steps

Get started with our [File samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-shares/samples):

1. [Upload and download files](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-shares/samples/file_share_getting_started.cpp)

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