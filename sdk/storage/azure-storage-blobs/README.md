# Azure Storage Blobs Client Library for C++

Azure Blob storage is Microsoft's object storage solution for the cloud. Blob storage is optimized for storing massive amounts of unstructured data. Unstructured data is data that does not adhere to a particular data model or definition, such as text or binary data.

## Getting started

### Install the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```batch
vcpkg install azure-storage-blobs-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-blobs)
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
cmake --build . --target azure-storage-blobs
```

or Unix:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target azure-storage-blobs
```

## Key concepts

Blob storage is designed for:

- Serving images or documents directly to a browser.
- Storing files for distributed access.
- Streaming video and audio.
- Writing to log files.
- Storing data for backup and restore, disaster recovery, and archiving.
- Storing data for analysis by an on-premises or Azure-hosted service.

Blob storage offers three types of resources:

- The storage account used via `BlobServiceClient`
- A container in the storage account used via `BlobContainerClient`
- A blob in a container used via `BlobClient`

Learn more about options for authentication (including Connection Strings, Shared Key, Shared Key Signatures, Active Directory, and anonymous public access) in our [samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-blobs/samples).

### Thread safety

We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

TBD

## Examples

### Uploading a blob

```C++
const std::string connectionString = "<connection_string>";
const std::string containerName = "sample-container";
const std::string blobName = "sample-blob";
const std::string filePath = "sample-file";

auto containerClient = BlobContainerClient::CreateFromConnectionString(connectionString, containerName);
containerClient.CreateIfNotExists();

BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);
blobClient.UploadFrom(filePath);
```

### Downloading a blob

```C++
blobClient.DownloadTo(filePath);
```

### Enumerating blobs

```C++
for (auto blobPage = containerClient.ListBlobs(); blobPage.HasPage(); blobPage.MoveToNextPage()) {
  for (auto& blob : blobPage.Blobs) {
    // Below is what you want to do with each blob
    std::cout << "blob: " << blob.Name << std::endl;
  }
}
```

## Troubleshooting

All Blob service operations will throw a [StorageException](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure with helpful [ErrorCode](https://learn.microsoft.com/en-us/rest/api/storageservices/blob-service-error-codes)s.
Many of these errors are recoverable.

```C++
try
{
  containerClient.Delete();
}
catch (Azure::Storage::StorageException& e)
{
  if (e.ErrorCode == "ContainerNotFound")
  {
    // ignore the error if the container does not exist.
  }
  else
  {
    // handle other errors here
  }
}
```

## Next steps

TBD

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
[storage_account_overview]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-overview
[create_account_with_azure_portal]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-portal
[create_account_with_powershell]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-powershell
[create_account_with_azure_cli]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-cli
[storage_contrib]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com