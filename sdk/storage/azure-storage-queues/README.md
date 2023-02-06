# Azure Storage Queues Client Library for C++

Azure Queue storage is a service for storing large numbers of messages that can be accessed from anywhere in the world via authenticated calls using HTTP or HTTPS. A single queue message can be up to 64 KB in size, and a queue can contain millions of messages, up to the total capacity limit of a storage account.

## Getting started

### Install the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Storage packages via vcpkg:

```batch
vcpkg install azure-storage-queues-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-storage-queues-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-storage-queues)
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
cmake --build . --target azure-storage-queues
```

or Unix:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target azure-storage-queues
```

## Key concepts

Common uses of Queue storage include:

- Creating a backlog of work to process asynchronously
- Passing messages between different parts of a distributed application

Learn more about options for authentication (including Connection Strings, Shared Key, Shared Key Signatures, Active Directory, and anonymous public access) in our [samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-queues/samples).

### Thread safety

We guarantee that all client instance methods are thread-safe and independent of each other ([guideline](https://azure.github.io/azure-sdk/cpp_introduction.html#thread-safety)). This ensures that the recommendation of reusing client instances is always safe, even across threads.

### Additional concepts

Client Options | [Accessing the response](https://github.com/Azure/azure-sdk-for-cpp#response-t-model-types) | [Long-running operations](https://github.com/Azure/azure-sdk-for-cpp#long-running-operations) | Handling failures

## Examples

### Send messages

```C++
const std::string connectionString = "<connection_string>";
const std::string queueName = "sample-queue";

// Get a reference to a queue and then create it
QueueClient queueClient = QueueClient(connectionString, queueName);
queueClient.Create();

// Send a message to our queue
queueClient.EnqueueMessage("Hello, Azure1!");
queueClient.EnqueueMessage("Hello, Azure2!");
queueClient.EnqueueMessage("Hello, Azure3!");
```
### Receive messages
```C++
ReceiveMessagesOptions receiveOptions;
receiveOptions.MaxMessages = 3;
auto receiveMessagesResult = queueClient.ReceiveMessages(receiveOptions).Value;
for (auto& msg : receiveMessagesResult.Messages)
{
  std::cout << msg.MessageText << std::endl;
  queueClient.DeleteMessage(msg.MessageId, updateResponse.Value.PopReceipt);
}
```

## Troubleshooting

All Azure Storage Queue  service operations will throw a [StorageException](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/inc/azure/storage/common/storage_exception.hpp)
on failure with helpful [ErrorCode](https://learn.microsoft.com/rest/api/storageservices/queue-service-error-codes)s.
Many of these errors are recoverable.

```C++
try
{
  queueClient.Delete();
}
catch (Azure::Storage::StorageException& e)
{
  if (e.ErrorCode == "QueueNotFound")
  {
    // ignore the error if the queue does not exist.
  }
  else
  {
    // handle other errors here
  }
}
```

## Next steps

Get started with our [Queue samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-queues/samples):

1. [Send and receive messages](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/samples/queue_getting_started.cpp)
2. [Encode and decode messages](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/samples/queue_encode_message.cpp)

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