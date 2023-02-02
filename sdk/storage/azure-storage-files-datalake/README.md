# Azure Storage Files Data Lake Client Library for C++

Azure Data Lake includes all the capabilities required to make it easy for developers, data scientists, and analysts to store data of any size, shape, and speed, and do all types of processing and analytics across platforms and languages. It removes the complexities of ingesting and storing all of your data while making it faster to get up and running with batch, streaming, and interactive analytics.

## Getting started

### Install the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```batch
vcpkg install azure-storage-files-datalake-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-storage-files-datalake-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-files-datalake)
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
cmake --build . --target azure-storage-files-datalake
```

or Unix:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target azure-storage-files-datalake
```

## Key concepts

DataLake Storage Gen2 was designed to:
- Service multiple petabytes of information while sustaining hundreds of gigabits of throughput
- Allow you to easily manage massive amounts of data

Key Features of DataLake Storage Gen2 include:
- Hadoop compatible access
- A superset of POSIX permissions
- Cost effective in terms of low-cost storage capacity and transactions
- Optimized driver for big data analytics

A fundamental part of Data Lake Storage Gen2 is the addition of a hierarchical namespace to Blob storage. The hierarchical namespace organizes objects/files into a hierarchy of directories for efficient data access.

In the past, cloud-based analytics had to compromise in areas of performance, management, and security. Data Lake Storage Gen2 addresses each of these aspects in the following ways:
- Performance is optimized because you do not need to copy or transform data as a prerequisite for analysis. The hierarchical namespace greatly improves the performance of directory management operations, which improves overall job performance.
- Management is easier because you can organize and manipulate files through directories and subdirectories.
- Security is enforceable because you can define POSIX permissions on directories or individual files.
- Cost effectiveness is made possible as Data Lake Storage Gen2 is built on top of the low-cost Azure Blob storage. The additional features further lower the total cost of ownership for running big data analytics on Azure.

Data Lake Storage Gen2 offers two types of resources:

- The _filesystem_ used via 'DataLakeFileSystemClient'
- The _path_ used via 'DataLakeFileClient' or 'DataLakeDirectoryClient'

|ADLS Gen2 	                | Blob       |
| --------------------------| ---------- |
|Filesystem                 | Container  | 
|Path (File or Directory)   | Blob       |

Note: This client library does not support hierarchical namespace (HNS) disabled storage accounts.

Learn more about options for authentication (including Connection Strings, Shared Key, Shared Key Signatures, Active Directory, and anonymous public access) in our [samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-datalake/samples).

### Thread safety

We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

Client Options | [Accessing the response](https://github.com/Azure/azure-sdk-for-cpp#response-t-model-types) | [Long-running operations](https://github.com/Azure/azure-sdk-for-cpp#long-running-operations) | Handling failures

## Examples

### Appending Data to a DataLake File

```C++
const std::string connectionString = "<connection_string>";
const std::string fileSystemName = "sample-filesystem";
const std::string directoryName = "sample-directory";
const std::string fileName = "sample-file";
const std::string localFilePath = "<path_to_local_file>";

// Create DataLakeServiceClient
DataLakeServiceClient serviceClient = DataLakeServiceClient::CreateFromConnectionString(connectionString);

// Get a reference to a filesystem named "sample-filesystem" and then create it
DataLakeFileSystemClient filesystemClient = serviceClient.GetFileSystemClient(fileSystemName);
filesystemClient.CreateIfNotExists();

// Create a DataLake Directory
DataLakeDirectoryClient directoryClient = filesystemClient.GetDirectoryClient(directoryName);
directoryClient.CreateIfNotExists();

// Create a DataLake File using a DataLake Directory
DataLakeFileClient fileClient = directoryClient.GetFileClient(fileName);
fileClient.CreateIfNotExists();

// Append data to the DataLake File
Azure::Core::IO::FileBodyStream fileStream(localFilePath);
fileClient.Append(fileStream, 0);
fileClient.Flush(fileStream.Length());
```
### Reading Data from a DataLake File
```C++
Response<DownloadFileResult> fileContents = fileClient.Download();
```

### Enumerating DataLake Paths
```C++
for (auto pathPage = client.ListPaths(false); pathPage.HasPage(); pathPage.MoveToNextPage())
{
  for (auto& path : pathPage.Paths)
  {
    // Below is what you want to do with each path
    std::cout << "path: " << path.Name << std::endl;
  }
}
```

## Troubleshooting

All File DataLake service operations will throw a [StorageException](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure with helpful [ErrorCode](https://learn.microsoft.com/en-us/rest/api/storageservices/blob-service-error-codes)s.
Many of these errors are recoverable.

```C++
try
{
  fileSystemClient.Delete();
}
catch (Azure::Storage::StorageException& e)
{
  if (e.ErrorCode == "ContainerNotFound")
  {
    // ignore the error if the file system does not exist.
  }
  else
  {
    // handle other errors here
  }
}
```

## Next steps

Get started with our [DataLake samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-datalake/samples):

1. [Append and read DataLake Files](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-datalake/samples/datalake_getting_started.cpp)

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