# Azure Storage Files Data Lake client library for C++

Azure Data Lake Storage is a scalable and secure data lake for high-performance analytics
workloads. It combines the scale and cost benefits of Azure Blob Storage with a hierarchical file
system that supports directory operations and POSIX access control lists.

[Source code][source_code] | [Package (vcpkg)][vcpkg_package] | [API reference documentation][api_reference] | [Product documentation][product_documentation] | [Samples][samples]

## Getting started

### Prerequisites

- [vcpkg](https://learn.microsoft.com/vcpkg/get_started/overview) for package acquisition and dependency management
- [CMake](https://cmake.org/download/) for project build
- An [Azure subscription][azure_sub]
- An Azure Storage account with hierarchical namespace enabled

If you need to create a Storage account, you can use the Azure portal or the [Azure CLI][azure_cli].
When using the Azure CLI, replace `<your-resource-group-name>` and
`<your-storage-account-name>` with your own values. Storage account names must be globally unique.

```PowerShell
az login
az storage account create `
  --resource-group <your-resource-group-name> `
  --name <your-storage-account-name> `
  --sku Standard_LRS `
  --enable-hierarchical-namespace true
```

### Install the package

The easiest way to acquire the C++ SDK is with the vcpkg package manager and CMake. See the
[Azure SDK for C++ installation instructions][azsdk_vcpkg_install] for more information.
The following commands use vcpkg in manifest mode.

Create a vcpkg manifest in the root of your project:

```batch
vcpkg new --application
```

Add Azure Storage Files Data Lake and Azure Identity to the manifest:

```batch
vcpkg add port azure-storage-files-datalake-cpp azure-identity-cpp
```

Then add the following to your `CMakeLists.txt` file:

```CMake
find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-storage-files-datalake-cpp CONFIG REQUIRED)

target_link_libraries(
  <your project name>
  PRIVATE
    Azure::azure-identity
    Azure::azure-storage-files-datalake)
```

Set `CMAKE_TOOLCHAIN_FILE` to the path to `vcpkg.cmake` before the `project()` statement in your
`CMakeLists.txt` file:

```CMake
set(CMAKE_TOOLCHAIN_FILE "vcpkg-root/scripts/buildsystems/vcpkg.cmake")
```

You can instead pass the path with the `-DCMAKE_TOOLCHAIN_FILE` argument when configuring CMake.
For other ways to acquire and install the library, see the
[Azure C++ project setup samples][project_set_up_examples].

### Create and authenticate clients

`DataLakeServiceClient` operates on the Data Lake service at the Storage account level. From it,
you can create clients for file systems, directories, and files.

The following example uses `DefaultAzureCredential`, which supports multiple credential types and
uses credentials from your development environment. After signing in with `az login`, set
`AZURE_STORAGE_DATALAKE_ACCOUNT_URL` to a URL such as
`https://<your-storage-account-name>.dfs.core.windows.net`.

```cpp
#include <azure/identity.hpp>
#include <azure/storage/files/datalake.hpp>

#include <cstdlib>
#include <memory>
#include <stdexcept>

using namespace Azure::Storage::Files::DataLake;

int main()
{
  const char* accountUrl = std::getenv("AZURE_STORAGE_DATALAKE_ACCOUNT_URL");
  if (accountUrl == nullptr)
  {
    throw std::runtime_error("AZURE_STORAGE_DATALAKE_ACCOUNT_URL is not set.");
  }

  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  DataLakeServiceClient serviceClient(accountUrl, credential);

  DataLakeFileSystemClient fileSystemClient
      = serviceClient.GetFileSystemClient("sample-file-system");
  DataLakeDirectoryClient directoryClient
      = fileSystemClient.GetDirectoryClient("sample-directory");
  DataLakeFileClient fileClient = directoryClient.GetFileClient("sample-file");
}
```

For more information about credential selection and configuration, see
[DefaultAzureCredential][default_azure_credential].

## Key concepts

Data Lake Storage is designed to:

- Store and analyze data at petabyte scale
- Provide Hadoop-compatible access
- Support POSIX-style permissions and access control lists
- Improve directory management performance with a hierarchical namespace
- Provide cost-effective storage for batch, streaming, and interactive analytics

Data Lake Storage offers the following resource hierarchy:

- The Storage account, accessed through `DataLakeServiceClient`
- A file system, accessed through `DataLakeFileSystemClient`
- A directory, accessed through `DataLakeDirectoryClient`
- A file, accessed through `DataLakeFileClient`

A Data Lake file system corresponds to a Blob container, and a Data Lake path corresponds to a
Blob. This client library requires a Storage account with hierarchical namespace enabled.

### Authentication

The library supports Microsoft Entra ID credentials, connection strings, shared key credentials,
and shared access signatures. Microsoft Entra ID with `DefaultAzureCredential` is recommended for
getting started. See the [sample][samples] for other authentication options.

### Thread safety

All client instance methods are thread-safe and independent of each other
([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). Reusing client
instances is safe, even across threads.

### Additional concepts

<!-- CLIENT COMMON BAR -->
[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#http-transport-adapter) |
[Response model types](https://github.com/Azure/azure-sdk-for-cpp#response-t-model-types) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/azure-core#long-running-operations)
<!-- CLIENT COMMON BAR -->

## Examples

### Create a directory and file

```cpp
fileSystemClient.CreateIfNotExists();
directoryClient.CreateIfNotExists();
fileClient.CreateIfNotExists();
```

### Append data to a file

Data Lake files are updated by appending data and then flushing it:

```cpp
const std::string fileContent = "Hello Azure!";
std::vector<uint8_t> buffer(fileContent.begin(), fileContent.end());
Azure::Core::IO::MemoryBodyStream stream(buffer);

fileClient.Append(stream, 0);
fileClient.Flush(buffer.size());
```

### Download a file

```cpp
auto downloadResponse = fileClient.Download();
Azure::Core::Context context;
std::vector<uint8_t> downloaded = downloadResponse.Value.Body->ReadToEnd(context);
```

### List paths

```cpp
for (auto pathPage = fileSystemClient.ListPaths(false); pathPage.HasPage();
     pathPage.MoveToNextPage())
{
  for (const auto& path : pathPage.Paths)
  {
    std::cout << "path: " << path.Name << std::endl;
  }
}
```

## Troubleshooting

Data Lake service operations throw an
[`Azure::Storage::StorageException`](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure. The exception includes the HTTP status code, service error code, request ID, and other
details that can help diagnose the failure. See the
[Blob service error codes](https://learn.microsoft.com/rest/api/storageservices/blob-service-error-codes)
for service-specific errors.

```cpp
try
{
  fileSystemClient.Delete();
}
catch (const Azure::Storage::StorageException& exception)
{
  if (exception.ErrorCode == "ContainerNotFound")
  {
    // The file system has already been deleted.
  }
  else
  {
    throw;
  }
}
```

## Next steps

The [Data Lake getting-started sample](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-datalake/samples/datalake_getting_started.cpp)
demonstrates how to create a file system, directory, and file, append data, and download the file.

## Contributing

For details on contributing to this repository, see the
[contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions. Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the
[Contributor License Agreement](https://cla.microsoft.com).

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately. Follow the instructions provided by the bot. You only need
to do this once across all repositories using the CLA.

This project has adopted the
[Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information, see the
[Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact
[opencode@microsoft.com](mailto:opencode@microsoft.com) with any questions or comments.

### Additional helpful links for contributors

- [Good first issues for new contributors](https://github.com/Azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
- [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
- [How to submit a pull request][azure_sdk_for_cpp_contributing_pull_requests]
- [Azure SDK for C++ wiki](https://github.com/Azure/azure-sdk-for-cpp/wiki)

### Reporting security issues and security bugs

Security issues and bugs should be reported privately to the Microsoft Security Response Center
(MSRC) at <secure@microsoft.com>. You should receive a response within 24 hours. If you do not,
follow up by email to confirm that your original message was received. For more information,
including the MSRC PGP key, see the
[Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the
[MIT license](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt).

<!-- LINKS -->
[api_reference]: https://learn.microsoft.com/cpp/api/overview/azure/storage-files-datalake-readme?view=azure-cpp
[azure_cli]: https://learn.microsoft.com/cli/azure
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sub]: https://azure.microsoft.com/free/
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#getting-started
[default_azure_credential]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#defaultazurecredential
[product_documentation]: https://learn.microsoft.com/azure/storage/blobs/data-lake-storage-introduction
[project_set_up_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration
[samples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-datalake/samples
[source_code]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-datalake
[vcpkg_package]: https://vcpkg.io/en/package/azure-storage-files-datalake-cpp
