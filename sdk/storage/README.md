# Azure Storage libraries for C++

Azure Storage is a Microsoft-managed service providing cloud storage that is highly available, secure, durable, scalable, and redundant. Azure Storage includes Blobs (objects), Queues, and Files.

- [Azure.Storage.Blobs](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/README.md) is Microsoft's object storage solution for the cloud. Blob storage is optimized for storing massive amounts of unstructured data that does not adhere to a particular data model or definition, such as text or binary data.

- [Azure.Storage.Queues](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/README.md) is a service for storing large numbers of messages.  A queue message can be up to 64 KB in size and a queue may contain millions of messages, up to the total capacity limit of a storage account.

- [Azure.Storage.Files.Shares](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-shares/README.md) offers fully managed file shares in the cloud that are accessible via the industry standard Server Message Block (SMB) protocol.  Azure file shares can be mounted concurrently by cloud or on-premises deployments of Windows, Linux, and macOS.

- [Azure.Storage.Files.DataLake](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-datalake/README.md) includes all the capabilities required to make it easy for developers, data scientists, and analysts to store data of any size, shape, and speed, and do all types of processing and analytics across platforms and languages.

- [Azure.Storage.Common](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/README.md) provides infrastructure shared by the other Azure Storage client libraries like shared key authentication and exceptions.

## Contributing

See the [C++ Contributing Guide][sdk_contrib] for details on building,
testing, and contributing to these libraries.

See the [Storage Testing Guide][storage_testing] for how to set up storage resources running unit tests.

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit [cla.microsoft.com][cla].

This project has adopted the [Microsoft Open Source Code of Conduct][coc].
For more information see the [Code of Conduct FAQ][coc_faq]
or contact [opencode@microsoft.com][coc_contact] with any
additional questions or comments.

<!-- LINKS -->
[sdk_contrib]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[storage_testing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/TestingGuide.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com
 