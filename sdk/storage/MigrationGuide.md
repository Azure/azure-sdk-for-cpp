# Migration Guide: From Azure Storage CPP SDK (v7.5) to Azure Storage CPP SDK

This guide intends to assist customers in migrating from legacy versions of the Azure Storage C++ library for Blobs to version 12.
While this guide is generally applicable to older versions of the SDK, it was written with v7.5 in mind as the starting point.
It focuses on side-by-side comparisons for similar operations between the [v7.5 package](https://github.com/Azure/azure-storage-cpp) and [v12 package](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage).
Familiarity with the v7.5 client library is assumed. For those new to the Azure Storage Blobs client library for C++, please refer to the [Quickstart](https://docs.microsoft.com/azure/storage/blobs/quickstart-blobs-c-plus-plus) for the v12 library rather than this guide.

## Table of contents

- [Migration benefits](#migration-benefits)
- [General changes](#general-changes)
  - [Package and namespaces](#package-and-namespaces)
  - [Authentication](#authentication)
  - [Client structure](#client-structure)
- [Migration samples](#migration-samples)
  - [Creating a Container](#creating-a-container)
  - [Uploading Blobs to a Container](#uploading-blobs-to-a-container)
  - [Downloading Blobs from a Container](#downloading-blobs-from-a-container)
  - [Listing Blobs in a Container](#listing-blobs-in-a-container)
  - [Managing Blob Metadata](#managing-blob-metadata)
  - [Content Hashes](#content-hashes)
  - [Resiliency](#resiliency)
- [Additional information](#additional-information)

## Migration benefits

Refer to the Tech Community blog post, [Announcing the Azure Storage v12 Client Libraries](https://techcommunity.microsoft.com/t5/azure-storage/announcing-the-azure-storage-v12-client-libraries/ba-p/1482394) or [Introducing the New Azure SDKs](https://aka.ms/azsdk/intro), to understand the benefits of switching to the version 12 client libraries.

Included are the following:
- Thread-safe synchronous APIs
- More comprehensive feature parity
- Dropped dependency of Boost and cpprestsdk
- Improved performance
- Consistent and idiomatic code organization, naming, and API structure, aligned with a set of common guidelines
- The learning curve associated with the libraries was reduced

Note: The blog post linked above announces deprecation for previous versions of the library.

## General changes

### Package and namespaces

Version 12 package names and the namespace roots follow the pattern `Azure::[Area]::[Service]` where v7.5 libraries followed the pattern `azure::[area]::[service]`.

Version 12 packages are installed with:
```bash
vcpkg install azure-storage-blobs-cpp
```

Previously the v7.5 package was installed with:
```bash
vcpkg install azure-storage-cpp
```

### Authentication

#### Azure Active Directory

v7.5

The legacy Storage SDK contained a `bearer_token_credential` class that could be used to populate a `storage_credentials` instance. Constructors took a string token for HTTP authorization headers and an optional refresh mechanism for the library to invoke when the token expired.

v12

A `TokenCredential` abstract class (different API surface than v7.5) exists in the [Azure Core](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core) package that all libraries of the new Azure SDK family depend on, and can be used to construct Storage clients. Implementations of this class can be found separately in the [Azure Identity](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity) package.

```C++
BlobServiceClient serviceClient(serviceUrl, std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret));
```

#### SAS

This section regards authenticating a client with an existing SAS

v7.5

In general, SAS tokens can be provided on their own to be applied as needed, or as a complete, self-authenticating URL. The legacy library allowed providing a SAS through `storage_credentials` as well as constructing with a complete URL.

```C++
cloud_blob_client blob_client(storage_uri(blob_url), storage_credentials(sas_token));
```

```C++
cloud_blob_client blob_client(storage_uri(blob_url_with_sas);
```

v12

The new library only supports constructing a client with a fully constructed SAS URI. Note that since client URIs are immutable once created, a new client instance with a new SAS must be created in order to rotate a SAS.

```C++
BlobClient blobClient(blobUrlWithSas);
```

#### Connection string

The following code assumes you have acquired your connection string (you can do so from the Access Keys tab under Settings in your Portal Storage Account blade). It is recommended to store it in an environment variable. Below demonstrates how to parse the connection string in v7.5 vs v12.

v7.5

```C++
cloud_storage_account storage_account = cloud_storage_account::parse(storage_connection_string);
cloud_blob_client service_client = storage_account.create_cloud_blob_client();
```

v12
```C++
BlobServiceClient serviceClient = BlobServiceClient::CreateFromConnectionString(connectionString);
```

You can also directly get a blob client with your connection string, instead of going through a service and container client to get to your desired blob. You just need to provide the container and blob names alongside the connection string.

```C++
BlobClient blobClient = BlobClient::CreateFromConnectionString(connectionString, containerName, blobName);
```

#### Shared Key

Shared key authentication requires the URI to the storage endpoint, the storage account name, and the shared key as a base64 string. The following code assumes you have acquired your shared key (you can do so from the Access Keys tab under Settings in your Portal Storage Account blade). It is recommended to store it in an environment variable.

Note that the URI to your storage account can generally be derived from the account name (though some exceptions exist), and so you can track only the account name and key. These examples will assume that is the case, though you can substitute your specific account URI if you do not follow this pattern.

v7.5
```C++
cloud_blob_client blob_client(storage_uri(blob_service_url), storage_credentials(account_name, account_key));
```

v12
```C++
auto credential = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);
BlobServiceClient serviceClient(blobServiceUrl, credential);
```

If you wish to rotate the key within your `BlobServiceClient` (and any derived clients), you must retain a reference to the `StorageSharedKeyCredential`, which has the instance method `Update(std::string accountKey)`.

### Client Structure

**The legacy SDK used a stateful model.** There were container and blob objects that held state regarding service resources and required the user to manually call their update methods. But blob contents were not a part of this state and had to be uploaded/downloaded whenever they were to be interacted with. This became increasingly confusing over time, and increasingly susceptible to thread safety issues.

The modern SDK has taken a client-based approach. There are no objects designed to be representations of storage resources, but instead clients that act as your mechanism to interact with your storage resources in the cloud. **Clients hold no state of your resources.** (Lease client is an exception.) This is most noticeable when looking at [blob metadata](#managing-blob-metadata).

The hierarchical structure of Azure Blob Storage can be understood by the following diagram:
![Blob Storage Hierarchy](https://docs.microsoft.com/azure/storage/blobs/media/storage-blobs-introduction/blob1.png)

In the interest of simplifying the API surface, v12 uses three top level clients to match this structure that can be used to interact with a majority of your resources: `BlobServiceClient`, `BlobContainerClient`, and `BlobClient`. Note that blob-type-specific operations can still be accessed by their specific clients, as in v7.5.

#### Migrating from cloud_blob_directory

Note the absence of a v12 equivalent for v7.5's `cloud_blob_directory`. Directories were an SDK-only concept that did not exist in Azure Blob Storage, and which were not brought forwards into the modern Storage SDK. As shown by the diagram in [Client Structure](#client-structure), containers only contain a flat list of blobs, but those blobs can be named and listed in ways that imply a folder-like structure. See our [Listing Blobs in a Container](#listing-blobs-in-a-container) migration samples later in this guide for more information.

For those whose workloads revolve around manipulating directories and heavily relied on the legacy SDKs abstraction of this structure, consider the [pros and cons of enabling hierarchical namespace](https://docs.microsoft.com/azure/storage/blobs/data-lake-storage-namespace) on your storage account, which would allow switching to the [Data Lake Gen 2 SDK](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/storage/azure-storage-files-datalake), whose migration is not covered in this document.

#### Class Conversion Reference

The following table lists v7.5 classes and their v12 equivalents for quick reference.

| v7.5 | v12 |
|-------|--------|
| `cloud_blob_client` | `BlobServiceClient` |
| `cloud_blob_container`  | `BlobContainerClient` |
| `cloud_blob_directory` | No equivalent |
| `cloud_blob` | `BlobClient` |
| `cloud_block_blob` | `BlockBlobClient` |
| `cloud_page_blob` | `PageBlobClient` |
| `cloud_append_blob` | `AppendBlobClient` |

## Migration Samples

### Creating a Container

v7.5
```C++
auto container_client = service_client.get_container_reference(container_name);
container_client.create();
```

v12
```C++
auto containerClient = serviceClient.GetBlobContainerClient(containerName);
containerClient.Create();
```

Or you can use the `BlobServiceClient.CreateBlobContainer()` method.

```C++
serviceClient.CreateBlobContainer(containerName);
```

### Uploading Blobs to a Container

#### Uploading from a file

v7.5
```C++
cloud_block_blob block_blob_client = container_client.get_block_blob_reference(blob_name);
block_blob_client.upload_from_file(local_file_path);
```

v12
```C++
BlockBlobClient blockBlobClient = containerClient.GetBlockBlobClient(blobName);
blockBlobClient.UploadFrom(localFilePath);
```

#### Uploading from a stream

v7.5
```C++
block_blob_client.upload_from_stream(stream);
```

v12
```C++
blockBlobClient.Upload(stream);
```

#### Uploading text

v7.5
```C++
block_blob_client.upload_text("Hello Azure!");
```

v12
```C++
uint8_t text[] = "Hello Azure!";
blockBlobClient.UploadFrom(text, sizeof(text) - 1);
```

### Downloading Blobs from a Container

#### Downloading to a file

v7.5
```C++
auto blob_client = container_client.get_blob_reference(blob_name);
blob_client.download_to_file(local_file_path);
```

v12
```C++
auto blobClient = containerClient.GetBlobClient(blobName);
blobClient.DownloadTo(localFilePath);
```

#### Downloading to a stream

v7.5
```C++
blob_client.download_to_stream(stream);
```

v12
```C++
auto response = blobClient.Download();
BodyStream& stream = *response.Value.BodyStream;
```

#### Downloading text

v7.5
```C++
auto text = blob_client.download_text();
```

v12
```C++
auto response = blobClient.Download();
std::vector<uint8_t> blobContent = response.Value.BodyStream->ReadToEnd();
std::string text(blobContent.begin(), blobContent.end());
```

### Listing Blobs in a Container

#### Flat Listing

v7.5
```C++
for (auto iter = container_client.list_blobs(); iter != list_blob_item_iterator(); ++iter) {
    if (iter->is_blob()) {
        auto blob_client = iter->as_blob();
    }
}
```

v12
```C++
for (auto blobPage = containerClient.ListBlobs(); blobPage.HasPage(); blobPage.MoveToNextPage()) {
    for (auto& blob : blobPage.Blobs) {

    }
}
```

#### Hierarchical Listing

See the [list blobs documentation](https://docs.microsoft.com/azure/storage/blobs/storage-blobs-list?tabs=dotnet#flat-listing-versus-hierarchical-listing) for more information on what a hierarchical listing is.

v7.5

`list_blobs()` and `list_blobs_segmented()` that were used in a flat listing contain overloads with a boolean parameter `use_flat_blob_listing`, which results in a flat listing when `true`. Provide `false` to perform a hierarchical listing.

```C++
for (auto iter = container_client.list_blobs(prefix, false, blob_listing_details::none, 0, blob_request_options, operation_context)) {
    if (iter->is_blob()) {
        auto blob_client = iter->as_blob();
    }
    else {
        auto directory_client = iter->as_directory();
    }
}
```

v12

v12 has explicit methods for listing by hierarchy.

```C++
for (auto blobPage = containerClient.ListBlobsByHierarchy("/"); blobPage.HasPage(); blobPage.MoveToNextPage()) {
    for (auto& blob : blobPage.Blobs) {

    }
    for (auto& blobPrefix : blobPage.BlobPrefixes) {

    }
}
```

### Managing Blob Metadata

On the service, blob metadata is overwritten alongside blob data overwrites. If metadata is not provided on a blob content edit, that is interpreted as a metadata clear. Legacy versions of the SDK mitigated this by maintaining blob metadata internally and sending it for you on appropriate requests. This helped in simple cases, but could fall out of sync and required developers to defensively code against metadata changes in a multi-client scenario anyway.

v12 has abandoned this stateful approach, having users manage their own metadata. While this requires additional code for developers, it ensures you always know how your metadata is being managed and avoid silently corrupting metadata due to SDK caching.

v7.5 samples:

The legacy SDK maintained a metadata cache, allowing you to modify metadata on the `cloud_blob` and invoke `upload_metadata()`. Calling `download_attributes()` beforehand refreshed the metadata cache to avoid undoing recent changes.

```C++
blob_client.download_attributes();
blob_client.metadata()["foo"] = "bar";
blob_client.upload_metadata();
```

The legacy SDK maintained internal state for blob content uploads. Calling `download_attributes()` beforehand refreshed the metadata cache to avoid undoing recent changes.

```C++
// download blob content. blob metadata is fetched and cached on download
blob_client.download_to_file(local_file_path);

// modify blob content

// re-upload modified blob content while preserving metadata
blob_client.upload_from_file(local_file_path);
```

v12 samples:

The modern SDK requires you to hold onto metadata and update it appropriately before sending off. You cannot just add a new key-value pair, you must update the collection and send the collection.

```C++
auto metadata = blobClient.GetProperties().Value.Metadata;
metadata["foo"] = "bar";
blobClient.SetMetadata(metadata);
```

Additionally with blob content edits, if your blobs have metadata you need to get the metadata and re-upload with that metadata, telling the service what metadata goes with this new blob state.

```C++
// download blob content and metadata
auto response = blobClient.DownloadTo(localFilePath);
auto metadata = response.Value.Metadata;

// modify blob content

// re-upload modified blob content while preserving metadata
// not adding metadata is a metadata clear
UploadBlockBlobFromOptions uploadOptions;
uploadOptions.Metadata = metadata;
blobClient.UploadFrom(localFilePath, uploadOptions);
```

### Content Hashes

#### Blob Content MD5

v7.5 calculated blob content MD5 for validation on download by default, assuming there was a stored MD5 in the blob properties. Calculation and storage on upload was opt-in. Note that this value is not generated or validated by the service, and is only retained for the client to validate against.

v7.5
```C++
blob_request_options options;
options.set_store_blob_content_md5(false);  // true to calculate content MD5 on upload and store property
options.set_disable_content_md5_validation(false);  // true to disable download content validation
```

v12 does not have an automated mechanism for blob content validation. It must be done per-request by the user.

v12
```C++
// upload with blob content hash property
UploadBlockBlobOptions uploadOptions;
uploadOptions.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
uploadOptions.HttpHeaders.ContentHash.Value = precalculatedContentHash;
blobClient.Upload(stream, uploadOptions);

// download whole blob and get stored content hash property
auto response = blobClient.Download();
auto hashAlgorithm = response.Value.Details.HttpHeaders.ContentHash.Algorithm;  // This is always MD5
auto md5 = response.Value.Details.HttpHeaders.ContentHash.Value;

// validate stream against hash in your workflow
```

#### Transactional MD5 and CRC64

Transactional hashes are not stored and have a lifespan of the request they are calculated for. Transactional hashes are verified by the service on upload.

v7.5 provided transactional hashing on uploads and downloads through opt-in request options. MD5 and Storage's custom CRC64 were supported. The SDK calculated and validated these hashes automatically when enabled. The calculation worked on any upload or download method.

v7.5

```C++
blob_request_options options;
options.set_use_transactional_md5(false);  // true to use MD5 on all blob content transactions.
options.set_use_transactional_crc64(false);  // true to use CRC64 on all blob content transactions.
```

v12 does not currently provide this functionality. Users who manage their own individual upload and download HTTP requests can provide a precalculated MD5 on upload and access the MD5 in the response object. v12 currently offers no API to request a transactional CRC64.

```C++
// upload a block with transactional hash calculated by user
StageBlockOptions stageBlockOptions;
stageBlockOptions.TransactionalContentHash = ContentHash();
stageBlockOptions.TransactionalContentHash.Algorithm = HashAlgorithm::Md5;  // HashAlgorithm::Crc64 to use CRC64.
stageBlockOptions.TransactionalContentHash.Value = precalculatedContentHash;
blobClient.StageBlock(blockId, blockContentStream, stageBlockOptions);

// upload more blocks as needed

// commit block list
blobClient.CommitBlockList(blockList);

// download any range of blob with transactional checksum requested (maximum 4 MB for downloads)
DownloadBlobOptions downloadOptions;
downloadOptions.RangeHashAlgorithm = HashAlgorithm::Md5;  // HashAlgorithm::Crc64 to use CRC64.
auto response = blobClient.Download();
auto hashAlgorithm = response.Value.Details.HttpHeaders.ContentHash.Algorithm;
auto hashValue = response.Value.Details.HttpHeaders.ContentHash.Value;
// validate stream against hash in your workflow
```

### Resiliency

#### Retry policy

v7.5
```C++
blob_request_options options;
options.set_retry_policy(exponential_retry_policy(delta_backoff, max_attempts));
```

v12
```C++
Blobs::BlobClientOptions options;
// The only supported mode is exponential.
options.Retry.RetryDelay = std::chrono::milliseconds(delta_backoff);
options.Retry.MaxRetries = maxAttempts;
```

### Asynchronous API

Unfortunately, we don't support asynchronous interface in v12 SDK. You could wrap synchronous functions into asynchronous with some async framework like `std::async`. But note that I/O operations are still performed synchronously under the hood. There's no performance gain with this method.

v7.5
```C++
auto task = blob_client.download_text_async().then([](utility::string_t blob_content) {
  std::wcout << "blob content:" << blob_content << std::endl;
});
// Do something else
task.wait();
```

v12
```C++
auto task = std::async([blobClient]() {
  auto response = blobClient.Download();
  std::vector<uint8_t> blobContent = response.Value.BodyStream->ReadToEnd();
  std::string text(blobContent.begin(), blobContent.end());
  std::cout << "blob content: " << text << std::endl;
});
// Do something else
task.wait();
```

## Additional information

### Samples
More examples can be found at:
- [Azure Storage samples using v12 C++ Client Libraries](https://docs.microsoft.com/azure/storage/common/storage-samples-c-plus-plus?toc=/azure/storage/blobs/toc.json)

### Links and references
- [Quickstart](https://docs.microsoft.com/azure/storage/blobs/quickstart-blobs-c-plus-plus)
- [Samples](https://docs.microsoft.com/azure/storage/common/storage-samples-c-plus-plus?toc=/azure/storage/blobs/toc.json)
- [C++ SDK reference](https://azure.github.io/azure-sdk-for-cpp/storage.html#azure-storage-blobs)