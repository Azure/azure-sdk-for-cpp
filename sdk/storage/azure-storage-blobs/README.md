# Azure Storage Blobs client library for C++

Azure Blob Storage is Microsoft's object storage solution for the cloud. It is optimized for
storing massive amounts of unstructured data, such as text, binary data, images, documents,
streaming media, and application data.

[Source code][source_code] | [Package (vcpkg)][vcpkg_package] | [API reference documentation][api_reference] | [Product documentation][product_documentation] | [Samples][samples]

## Getting started

### Prerequisites

- [vcpkg](https://learn.microsoft.com/vcpkg/get_started/overview) for package acquisition and dependency management
- [CMake](https://cmake.org/download/) for project build
- An [Azure subscription][azure_sub]
- An existing [Azure Storage account][storage_account_overview]

If you need to create a Storage account, you can use the Azure portal or the [Azure CLI][azure_cli].
When using the Azure CLI, replace `<your-resource-group-name>` and
`<your-storage-account-name>` with your own values. Storage account names must be globally unique.

```PowerShell
az login
az storage account create `
  --resource-group <your-resource-group-name> `
  --name <your-storage-account-name> `
  --sku Standard_LRS
```

### Install the package

The easiest way to acquire the C++ SDK is with the vcpkg package manager and CMake. See the
[Azure SDK for C++ installation instructions][azsdk_vcpkg_install] for more information.
The following commands use vcpkg in manifest mode.

Create a vcpkg manifest in the root of your project:

```batch
vcpkg new --application
```

Add Azure Storage Blobs and Azure Identity to the manifest:

```batch
vcpkg add port azure-storage-blobs-cpp azure-identity-cpp
```

Then add the following to your `CMakeLists.txt` file:

```CMake
find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)

target_link_libraries(
  <your project name>
  PRIVATE
    Azure::azure-identity
    Azure::azure-storage-blobs)
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

`BlobServiceClient` operates on the Blob service at the Storage account level. From it, you can
create a `BlobContainerClient` for container operations and specialized blob clients for block,
append, and page blobs.

The following example uses `DefaultAzureCredential`, which supports multiple credential types and
uses credentials from your development environment. After signing in with `az login`, set
`AZURE_STORAGE_ACCOUNT_URL` to a URL such as
`https://<your-storage-account-name>.blob.core.windows.net`.

```cpp
#include <azure/identity.hpp>
#include <azure/storage/blobs.hpp>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Azure::Storage::Blobs;

int main()
{
  const char* accountUrl = std::getenv("AZURE_STORAGE_ACCOUNT_URL");
  if (accountUrl == nullptr)
  {
    throw std::runtime_error("AZURE_STORAGE_ACCOUNT_URL is not set.");
  }

  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  BlobServiceClient serviceClient(accountUrl, credential);

  BlobContainerClient containerClient
      = serviceClient.GetBlobContainerClient("sample-container");
  BlockBlobClient blockBlobClient = containerClient.GetBlockBlobClient("sample-blob");
}
```

For more information about credential selection and configuration, see
[DefaultAzureCredential][default_azure_credential].

## Key concepts

Blob Storage is designed for:

- Serving images or documents directly to a browser
- Storing files for distributed access
- Streaming video and audio
- Writing to log files
- Storing data for backup, restore, disaster recovery, and archiving
- Storing data for analysis by an on-premises or Azure-hosted service

Blob Storage offers three types of resources:

- The Storage account, accessed through `BlobServiceClient`
- A container in the Storage account, accessed through `BlobContainerClient`
- A blob in a container, accessed through `BlobClient` or a specialized `BlockBlobClient`,
  `AppendBlobClient`, or `PageBlobClient`

### Authentication

The library supports Microsoft Entra ID credentials, connection strings, shared key credentials,
shared access signatures, and anonymous public access where enabled. Microsoft Entra ID with
`DefaultAzureCredential` is recommended for getting started. See the [samples][samples] for other
authentication options.

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

### Upload a blob

Create the container if it does not exist, and then upload data to a block blob:

```cpp
const std::string blobContent = "Hello Azure!";

containerClient.CreateIfNotExists();

std::vector<uint8_t> buffer(blobContent.begin(), blobContent.end());
blockBlobClient.UploadFrom(buffer.data(), buffer.size());
```

### Download a blob

Download the blob into a buffer:

```cpp
auto properties = blockBlobClient.GetProperties().Value;
std::vector<uint8_t> buffer(static_cast<std::size_t>(properties.BlobSize));

blockBlobClient.DownloadTo(buffer.data(), buffer.size());
```

### List blobs

List the blobs in a container:

```cpp
for (auto blobPage = containerClient.ListBlobs(); blobPage.HasPage();
     blobPage.MoveToNextPage())
{
  for (const auto& blob : blobPage.Blobs)
  {
    std::cout << "blob: " << blob.Name << std::endl;
  }
}
```

## Troubleshooting

Blob service operations throw an
[`Azure::Storage::StorageException`](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure. The exception includes the HTTP status code, service error code, request ID, and other
details that can help diagnose the failure. See the
[Blob service error codes](https://learn.microsoft.com/rest/api/storageservices/blob-service-error-codes)
for service-specific errors.

```cpp
try
{
  containerClient.Delete();
}
catch (const Azure::Storage::StorageException& exception)
{
  if (exception.ErrorCode == "ContainerNotFound")
  {
    // The container has already been deleted.
  }
  else
  {
    throw;
  }
}
```

## Next steps

The following samples demonstrate common Blob Storage scenarios:

- [Upload and download blobs](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/blob_getting_started.cpp)
- [List containers and blobs](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/blob_list_operation.cpp)
- [Set a timeout for list operations](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/blob_list_operation_with_timeout.cpp)
- [Query blob contents](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/blob_query.cpp)
- [Create shared access signatures](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/blob_sas.cpp)
- [Use transactional checksums](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/samples/transactional_checksum.cpp)

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
[api_reference]: https://learn.microsoft.com/cpp/api/overview/azure/storage-blobs-readme?view=azure-cpp
[azure_cli]: https://learn.microsoft.com/cli/azure
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sub]: https://azure.microsoft.com/free/
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#getting-started
[default_azure_credential]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#defaultazurecredential
[product_documentation]: https://learn.microsoft.com/azure/storage/blobs/
[project_set_up_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration
[samples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-blobs/samples
[source_code]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-blobs
[storage_account_overview]: https://learn.microsoft.com/azure/storage/common/storage-account-overview
[vcpkg_package]: https://vcpkg.io/en/package/azure-storage-blobs-cpp
