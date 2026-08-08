# Azure Storage Queues client library for C++

Azure Queue Storage stores large numbers of messages that can be accessed from anywhere through
authenticated HTTP or HTTPS calls. Queues are commonly used to create a backlog of work for
asynchronous processing and to exchange messages between components of distributed applications.

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

Add Azure Storage Queues and Azure Identity to the manifest:

```batch
vcpkg add port azure-storage-queues-cpp azure-identity-cpp
```

Then add the following to your `CMakeLists.txt` file:

```CMake
find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-storage-queues-cpp CONFIG REQUIRED)

target_link_libraries(
  <your project name>
  PRIVATE
    Azure::azure-identity
    Azure::azure-storage-queues)
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

`QueueServiceClient` operates on the Queue service at the Storage account level. From it, you can
create a `QueueClient` to manage a queue and its messages.

The following example uses `DefaultAzureCredential`, which supports multiple credential types and
uses credentials from your development environment. After signing in with `az login`, set
`AZURE_STORAGE_QUEUE_ACCOUNT_URL` to a URL such as
`https://<your-storage-account-name>.queue.core.windows.net`.

```cpp
#include <azure/identity.hpp>
#include <azure/storage/queues.hpp>

#include <cstdlib>
#include <memory>
#include <stdexcept>

using namespace Azure::Storage::Queues;

int main()
{
  const char* accountUrl = std::getenv("AZURE_STORAGE_QUEUE_ACCOUNT_URL");
  if (accountUrl == nullptr)
  {
    throw std::runtime_error("AZURE_STORAGE_QUEUE_ACCOUNT_URL is not set.");
  }

  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  QueueServiceClient serviceClient(accountUrl, credential);
  QueueClient queueClient = serviceClient.GetQueueClient("sample-queue");
}
```

For more information about credential selection and configuration, see
[DefaultAzureCredential][default_azure_credential].

## Key concepts

Queue Storage is designed for:

- Creating a backlog of work to process asynchronously
- Decoupling components of a distributed application
- Scheduling work that can be retried independently
- Scaling producers and consumers separately

Queue Storage offers the following resource hierarchy:

- The Storage account, accessed through `QueueServiceClient`
- A queue, accessed through `QueueClient`
- Messages within a queue, managed through `QueueClient`

A queue message can be up to 64 KiB and can remain in the queue until it is explicitly deleted or
its time-to-live expires. Receiving a message makes it temporarily invisible to other consumers;
the message must be deleted after successful processing.

### Authentication

The library supports Microsoft Entra ID credentials, connection strings, shared key credentials,
and shared access signatures. Microsoft Entra ID with `DefaultAzureCredential` is recommended for
getting started. See the [samples][samples] for other authentication options.

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

### Create a queue and send a message

```cpp
queueClient.Create();
queueClient.EnqueueMessage("Hello, Azure!");
```

### Receive and delete messages

```cpp
ReceiveMessagesOptions options;
options.MaxMessages = 5;

auto response = queueClient.ReceiveMessages(options);
for (const auto& message : response.Value.Messages)
{
  std::cout << message.MessageText << std::endl;
  queueClient.DeleteMessage(message.MessageId, message.PopReceipt);
}
```

### Peek at messages

Peeking returns messages without changing their visibility:

```cpp
PeekMessagesOptions options;
options.MaxMessages = 5;

auto response = queueClient.PeekMessages(options);
for (const auto& message : response.Value.Messages)
{
  std::cout << message.MessageText << std::endl;
}
```

## Troubleshooting

Queue service operations throw an
[`Azure::Storage::StorageException`](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure. The exception includes the HTTP status code, service error code, request ID, and other
details that can help diagnose the failure. See the
[Queue service error codes](https://learn.microsoft.com/rest/api/storageservices/queue-service-error-codes)
for service-specific errors.

```cpp
try
{
  queueClient.Delete();
}
catch (const Azure::Storage::StorageException& exception)
{
  if (exception.ErrorCode == "QueueNotFound")
  {
    // The queue has already been deleted.
  }
  else
  {
    throw;
  }
}
```

## Next steps

The following samples demonstrate common Queue Storage scenarios:

- [Send and receive messages](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/samples/queue_getting_started.cpp)
- [Encode and decode messages](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/samples/queue_encode_message.cpp)

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
[api_reference]: https://learn.microsoft.com/cpp/api/overview/azure/storage-queues-readme?view=azure-cpp
[azure_cli]: https://learn.microsoft.com/cli/azure
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sub]: https://azure.microsoft.com/free/
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#getting-started
[default_azure_credential]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#defaultazurecredential
[product_documentation]: https://learn.microsoft.com/azure/storage/queues/storage-queues-introduction
[project_set_up_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/samples/integration
[samples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-queues/samples
[source_code]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-queues
[storage_account_overview]: https://learn.microsoft.com/azure/storage/common/storage-account-overview
[vcpkg_package]: https://vcpkg.io/en/package/azure-storage-queues-cpp
