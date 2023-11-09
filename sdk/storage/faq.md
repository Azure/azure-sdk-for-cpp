# Frequently Asked Questions, Common Mistakes and Best Practices

**Frequently Asked Questions**

[How to use list-operations? Why don't list-operations return all blobs in a container?](#how-to-use-list-operations-why-dont-list-operations-return-all-blobs-in-a-container)

[Can I specify an API-version other than the default one?](#can-i-specify-an-api-version-other-than-the-default-one)

[How can I set a custom UserAgent string?](#how-can-i-set-a-custom-useragent-string)

[What encoding is used for input and output?](#what-encoding-is-used-for-input-and-output)

[How can I check if a blob exists?](#how-can-i-check-if-a-blob-exists)

[What is the difference between `BlockBlobClient::Upload()` and `BlockBlobClient::UploadFrom()`? When should I use one vs the other?](#what-is-the-difference-between-blockblobclientupload-and-blockblobclientuploadfrom-when-should-i-use-one-vs-the-other)

[How to efficiently upload a large amount of small blobs?](#how-to-efficiently-upload-large-amount-of-small-blobs)

[How to ensure data integrity with transactional checksum?](#how-to-ensure-data-integrity-with-transactional-checksum)


**Design and Concepts**

[Thread safety guarantees](#thread-safety-guarantees)

[Http pipeline and policies](#http-pipeline-and-policies)

[Will the SDK retry failed requests? What is the default retry logic? How can I customize retry behavior?](#will-the-sdk-retry-failed-requests-what-is-the-default-retry-logic-how-can-i-customize-retry-behavior)


**Troubleshooting**

[Why do I see 503 Server Busy errors? What should I do in this case?](#why-do-i-see-503-server-busy-errors-what-should-i-do-in-this-case)

[How to troubleshoot 403 errors?](#how-to-troubleshoot-403-errors)

## How to use list-operations? Why don't list-operations return all blobs in a container?

The C++ SDK applies a different design for pageable operations from other languages.
It involves a nested-loop to iterate over all items in a container.
The outer loop does network I/O operations.
The inner loop gets called for every paged result and doesn't do I/O.

Below is an example of listing all blobs in a blob container.

```C++
for (auto page = blobContainerClient.ListBlobs(); page.HasPage(); page.MoveToNextPage()) {
  for (auto& blob : page.Blobs) {
    std::cout << blob.Name << std::endl;
  }
}
```

Sometimes a paged result may contain multiple collections and you may want to iterate over all of them.

```C++
for (auto page = directoryClient.ListFilesAndDirectories(); page.HasPage(); page.MoveToNextPage())
{
  for (const auto& d : page.Directories)
  {
    std::cout << "Directory: " << d.Name << std::endl;
  }
  for (const auto& f : page.Files)
  {
    std::cout << "File: " << f.Name << std::endl;
  }
}
```

## Can I specify an API-version other than the default one?

Yes, each client options takes an `ApiVersion` as an optional parameter, with which you can specify an API-version used for all the HTTP requests from this client.
Clients spawned from another client instance will inherit the settings.

```C++
// serviceClient sends HTTP requests with default API-version, which will change as version evolves.
auto serviceClient = BlobServiceClient::CreateFromConnectionString(GetConnectionString());

// serviceClient sends HTTP requests with specified API-version.
BlobClientOptions clientOptions;
clientOptions.ApiVersion = "2019-12-12";
auto serviceClient2 = BlobServiceClient::CreateFromConnectionString(GetConnectionString, clientOptions);
// containerClient inherits this setting.
auto containerClient = serviceClient2.GetBlobContainerClient(containerName);
```

Please note that multiple-version is not supported by the Storage C++ SDK (this may change in the future).
Which means, the code above only changes the `x-ms-version` request header and doesn't change SDK behavior.
The SDK will still set headers and query parameters that are added between the customized API version (which is usually older) and latest API version,
and still expect some headers or fields in HTTP response.
Furthermore, this scenario is not covered by testing, although most of the APIs should be able to work. You should do thorough testing before your code goes to production.

## How can I set a custom UserAgent string?

We recommend you set an application ID with the code below, so that it can be identified from which application, SDK, and platform the request was sent. The information could be useful for troubleshooting and telemetry purposes.

```C++
BlobClientOptions clientOptions;
clientOptions.Telemetry.ApplicationId = "SomeApplication v1.2.3";

// The User-Agent string will be something like:
// SomeApplication v1.2.3 azsdk-cpp-storage-blobs/12.9.0-beta.3 (Windows 10 Enterprise 6.3 19045 19041.1.amd64fre.vb_release.191206-1406)
```

## What encoding is used for input and output?

All URLs should be URL-encoded and other resource names should be UTF-8 encoded.
This applies to both input variables and output.
If your code runs in an environment where the default locale and encoding is not UTF-8, you should encode before passing variables into the SDK and decode variables returned from the SDK.

In the blow code snippet, we'd like to create a blob named <code>ol&#225;</code>.
```C++
// If the blob client is created from a container client, the blob name should be UTF-8 encoded.
auto blobClient = containerClient.GetBlobClient("ol\xC3\xA1");
// If the blob client is built from URL, it should be URL-encoded
blobClient = Azure::Storage::Blobs::BlobClient("https://account.blob.windows.core.net/container/ol%C3%A1");

// The return value is URL-encoded
auto blobUrl = blobClient.GetUrl();

for (auto page = blobContainerClient.ListBlobs(); page.HasPage(); page.MoveToNextPage()) {
  for (auto& blob : page.Blobs) {
    // blob.Name is UTF-8 encoded
    std::cout << blob.Name << std::endl;
  }
}
```

## How can I check if a blob exists?

You can check whether a blob exists or not by writing a convenience method on top of getting blob properties, as follows:

```C++
bool BlobExists(const Azure::Storage::Blobs::BlobClient& client) {
  try {
    client.GetProperties();
    return true;
  }
  catch (Azure::Storage::StorageException& e) {
    if (e.ReasonPhrase == "The specified blob does not exist.") {
      return false;
    }
    throw;
  }
}
```

A common use scenario is checking if a blob exists before writing to it.
It is fine if you write from a single thread.
However, it is problematic if the destination might be modified by multiple threads or processes. A subsequent write operation may get overwritten by the previous one.
In this case, it's more recommended to:

1. Use `CreateIfNotExists()`, `DeleteIfExists()` functions whenever possible. These functions internally use access conditions and can help you catch unexpected exceptions caused by resending PUT/DELETE requests on network errors.
1. Use access conditions for other operations. The code below only sends one HTTP request, check-and-write is performed atomically. It only succeeds if the blob doesn't exist.
   ```C++
   UploadBlockBlobOptions options;
   options.AccessConditions.IfNoneMatch = Azure::ETag::Any();
   blobClient.Upload(stream, options);
   ```
1. Use lease for more complex scenarios that invole multiple operations on the same resource.

## What is the difference between `BlockBlobClient::Upload()` and `BlockBlobClient::UploadFrom()`? When should I use one vs the other?

`BlockBlobClient::Upload()` takes a stream as a parameter and uploads the stream as a block blob with exactly one HTTP request.
The blob created with this method doesn't have any blocks, which means functions like `StageBlock()`, `CommitBlockList()` or `GetBlockList()` don't apply here.
You cannot append blocks to such blobs (You'll have to overwrite it with a new one).
You want to use this one if you need precise control over the SDK behavior at an HTTP level.

`BlockBlobClient::UploadFrom` takes a memory buffer or file name as a parameter, splits the data in the buffer or file into smaller chunks intelligently or according to some parameters if provided,
and then upload the chunks with multiple threads.
This one is suitable in most cases. You can expect higher throughput because the chunks are transferred concurrently. It's especially recommended if you need to transfer large blobs efficiently.

## How to efficiently upload large amount of small blobs?

Unfortunately, this SDK doesn't provide a convenient way to upload many blobs or directory contents (files and sub-directories) with just one function call.
You have to create multiple threads, traverse the directories by yourself and upload blobs one by one in each thread to speed up the transfer. Below is a skeleton example.
```C++
const std::vector<std::string> paths; // Files to be uploaded
std::atomic<size_t> curr{0};
auto upload_func = [&]() {
  while (true)
  {
    auto id = curr.fetch_add(1);
    if (id >= paths.size())
    {
      break;
    }
    const std::string path = paths[id];
    try
    {
      blobClient.UploadFrom(path);
    }
    catch (Azure::Storage::StorageException& e)
    {
      // exception handling
    }
  }
};

std::vector<std::thread> thread_handles;
for (size_t i = 0; i < num_threads; ++i)
{
  thread_handles.push_back(std::thread(upload_func));
}
for (auto& i : thread_handles)
{
  i.join();
}
```
Or you can use tools like [AzCopy](https://learn.microsoft.com/azure/storage/common/storage-ref-azcopy) or [Data Movement Library](https://learn.microsoft.com/azure/storage/common/storage-use-data-movement-library).

## How to ensure data integrity with transactional checksum?

Generally speaking, the TLS protocol includes checksum that's strong enough to detect accidental corruption or deliberate tampering.
Another layer of checksum is usually not necessary if you're using HTTPS.

If you really want it for whatever reason, for example, to detect corruptions before the data is written to the socket,
you can leverage the transactional checksum feature in the SDK.
With this feature, you provide a pre-calculated MD5 or CRC64 checksum when calling an upload API
and the storage service will calculate checksum after it receives the data. It will then compare that with the one you provide
and fail the request if they don't match.
Make sure you calculate the checksum as early as possible so that potential corruptions that occur on storage medium, computer memory or during data transfer can be detected.

This functionality also works for download operations.
Below is a code sample to use this feature.

```C++
// upload data with pre-calculated checksum
Blobs::UploadBlockBlobOptions options;
auto checksum = ContentHash();
checksum.Algorithm = HashAlgorithm::Crc64;
checksum.Value = crc64;  // CRC64 checksum of the data in Base64 encoding
options.TransactionalContentHash = checksum;
blobClient.Upload(stream, options);

// download and verify checksum
Blobs::DownloadBlobOptions options;
options.RangeHashAlgorithm = HashAlgorithm::Crc64;
auto response = blobClient.Download(options);
auto crc64 = response.Value.TransactionalContentHash.Value();
// Now you can verify checksum of the downloaded data
```

## Thread safety guarantees

All storage client APIs are thread-safe.
You can call the same API or different APIs on the same client instance from multiple threads without additional synchronization.
It's guaranteed to either return a successful response or throw an exception.

However, this doesn't mean access to the underlying storage service has no race conditions.
It is still possible that you'll get a 409 error if you read a file that's being written to by another party. (For example, one thread reading an append blob while another thread appending data to it.)

## Http pipeline and policies

An HTTP pipeline consists of a sequence of steps executed for each HTTP request-response roundtrip.
Each policy has a dedicated purpose and acts on the request or the response or sometimes both.
When you send a request, the policies are executed in the order that they're added to the pipeline.
When you receive a response from the service, the policies are executed in reverse order.
All policies added to the pipeline execute before you send the request and after you receive a response.
Each policy has to decide whether to act on the request, the response, or both.
For example, a logging policy logs the request and response but the authentication policy is only interested in modifying the request.

The Azure Core library as part of this SDK provides the policy with the necessary request and response data along with any necessary context to execute the policy.
The policy can then perform its operation with the given data and pass the control along to the next policy in the pipeline.

#### HTTP pipeline policy position

When you make HTTP requests to cloud services, it's important to handle transient failures and to retry failed attempts.
Because this functionality is a common requirement, Azure Core provides a retry policy that can watch for transient failures and automatically retry the request.

This retry policy, therefore, splits the whole pipeline into two parts:
policies that execute before the retry policy and policies that execute after the retry policy.
Policies added before the retry policy execute only once per API operation, and policies added after the retry policy execute as many times as the retries.

So, when building the HTTP pipeline, you should understand whether to execute a policy for each request retry or once per API operation.

![](https://learn.microsoft.com/en-us/azure/developer/java/sdk/media/http-pipeline.svg)

#### Common HTTP pipeline policies

HTTP pipelines for REST-based services have configurations with policies for authentication, retries, logging, telemetry, and specifying the request ID in the header.
These policies are automatically added into the HTTP pipeline.

#### Custom HTTP pipeline policy

Policies within an HTTP pipeline provide a convenient mechanism to modify or decorate the request and response.
You can add custom policies to the pipeline that the user or the client library developer created.
When adding a policy to the pipeline, you can specify whether this policy should be executed per-call or per-retry.

To create a custom HTTP pipeline policy, you just inherit from the base `Azure::Core::Http::Policies::HttpPolicy` class and implement a couple of virtual functions.
You can then plug the policy into the pipeline.

#### Custom headers in HTTP requests

Below is an example of adding a custom header into each HTTP request.
The header value is static and doesn't change over time, so we make it a per-operation policy.
If you want to add some time-variant headers like authentication, you should use a per-retry policy.

```C++
class NewPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
public:
  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<NewPolicy>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
      Azure::Core::Context const& context) const override
  {
    request.SetHeader("x-ms-custom-hearder-name", "header-value");
    return nextPolicy.Send(request, context);
  }
};

BlobClientOptions options;
options.PerOperationPolicies.push_back(std::make_unique<NewPolicy>());
options.PerRetryPolicies.push_back(std::make_unique<NewPolicy>());
```

## Will the SDK retry failed requests? What is the default retry logic? How can I customize retry behavior?

Requests failed due to network errors or HTTP status code 408, 500, 502, 503, 504 will be retried at most 3 times (4 attempts in total) using exponential backoff with jitter.
These parameters can be customized with `RetryOptions`. Below is an example.

```C++
BlobClientOptions options;
options.Retry.RetryDelay = std::chrono::milliseconds(800);
options.Retry.MaxRetryDelay = std::chrono::seconds(60);
options.Retry.MaxRetries = 3;
```

## Why do I see 503 Server Busy errors? What should I do in this case?

The Azure storage service has scalability and performance target, which varies for different types of accounts.
When your application accesses storage too aggressively and reaches the limit, storage starts returning 503 Server Busy errors.
You may refer to [this page](https://learn.microsoft.com/azure/storage/common/scalability-targets-standard-account) for the limits of different types of storage accounts in different regions.

Here are a few things you can do to minimize the impact of this kind of error.

1. Use exponential backoff policy and jitter for retries.
   Exponential backoff allows the load on storage service to decrease over time. Jitter helps ease out spikes and ensures the retry traffic is well-distributed over a time window.
1. Increase the retry count.
1. Identify the throttling type and reduce the traffic sent from client side.
   You can check the exception thrown from storage function calls with the below code. The error message will indicate which scalability target was exceeded.
   ```C++
   try
   {
     // storage function goes here
   }
   catch (Azure::Storage::StorageException& e)
   {
     if (e.StatusCode == Azure::Core::Http::HttpStatusCode::ServiceUnavailable)
     {
       std::cout << e.Message << std::endl;
     }
   }
   ```
1. Store your data in multiple storage accounts for load balancing.
1. Use a CDN for static resources.

Throttling errors could also happen at a subscription level or tenant level. You can still try the above approaches or contact Azure Support in such cases.

## How to troubleshoot 403 errors?

A 403 error means your request to access Azure Storage is not correctly authorized.

If you're using shared key authentication, you should:

1. Check that the storage account name and account key match the resource you're trying to access, and the key is not rotated.
1. If you need to add extra HTTP request headers or overwrite existing headers, do it with an HTTP pipeline policy, so that the new values can be signed with the shared key.
1. If you use a customized HTTP client, do not make any changes to the HTTP request in the client.

If you're using SAS authentication, you should:

1. Check the values of query parameter `st` and `se`, which specify a time range the SAS token is valid. Make sure the token is not expired.
   If it's a user delegation SAS, also check `ske` which indicates the expiry of the user delegation key.
1. Make sure the shared key used to generate the SAS token is not rotated.
1. If the SAS token is created with a stored access policy, make sure the access policy is still valid.
1. SAS token has its own scope, for example it may be scoped to a storage account, a container or a blob/file. Make sure you don't access a resource out of the SAS token's scope.
1. Check the message in the exception.
   You could print the information in the exception with the code below.
   ```C++
   try
   {
     // storage function goes here
   }
   catch (Azure::Storage::StorageException& e)
   {
     if (e.StatusCode == Azure::Core::Http::HttpStatusCode::Forbidden)
     {
       std::cout << e.RequestId << std::endl;
       std::cout << e.ClientRequestId << std::endl;
       std::cout << e.ErrorCode << std::endl;
       std::cout << e.Message << std::endl;
       for (const auto& i : e.AdditionalInformation)
       {
         std::cout << i.first << ":" << i.second << std::endl;
       }
     }
   }
   ```
   For example, a common error code is `AuthorizationPermissionMismatch`, which means the SAS token doesn't have permission to perform this operation (like writing to a blob with a SAS token that only permits read).

You shouldn't write your own functions to generate signatures or sign the requests. It's not a trivial task and there are many edge cases to cover. Use official tools or the SDK instead.

If you still cannot figure out the problem with the above steps, you can open [a GitHub issue](https://github.com/Azure/azure-sdk-for-cpp/issues/new/choose) with the request ID or client request ID of the failed request.
