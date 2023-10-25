# Frequently-asked Questions, Common Mistakes and Best Practices

## How to use list-operations? Why doesn't list-operations return all blobs in a container?

C++ SDK applied a different design for pageable operations from other languages.
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

Yes, each client options takes an `ApiVersion` as optional parameter, with which you can specify an API-version used for all the HTTP requests from this client.
Client spawned from another client instance will inherit the settings.

```C++
// serviceClient sends HTTP requests with default API-version, which will change as version evolves.
auto serviceClient = BlobServiceClient::CreateFromConnectionString(GetConnectionString());

// serviceClient sends HTTP requests with specified API-version.
BlobClientOptions clientOptions;
clientOptions.ApiVersion = "2019-12-12";
auto serviceClient = BlobServiceClient::CreateFromConnectionString(GetConnectionString, clientOptions);
// containerClient inherits this setting.
auto containerClient = serviceClient.GetBlobContainerClient(containerName);
```

Please note that multiple-version is not supported yet by Storage C++ SDK (This may change in the future).
Which means, the code above only changes the `x-ms-version` request header and doesn't change SDK behavior.
SDK will still set headers and query parameters that are added between the cutomized API version (which is usually older) and latest API version,
and still expect some headers or fields in HTTP response.
Furthermore, this scenario is not covered by testing, although most of the APIs should be able to work. You should do thorough testing before your code goes to production.

## How can I set a custom UserAgent string?

We recommend you set an application ID with code below, so that it can be identified from which application, SDK, platform the request was sent. The information could be useful for troubleshooting and telemetry purposes.

```C++
BlobClientOptions clientOptions;
clientOptions.Telemetry.ApplicationId = "SomeApplication v1.2.3";

// The User-Agent string will be something like:
// SomeApplication v1.2.3 azsdk-cpp-storage-blobs/12.9.0-beta.3 (Windows 10 Enterprise 6.3 19045 19041.1.amd64fre.vb_release.191206-1406)
```

## What encoding is used for input and output?

All URLs should be URL-encoded, other resource names should be in UTF-8 encoding.
This applies to both input variables and output.
If your code runs in an environment where the default locale and encoding is not UTF-8, you should encode before passing variables into SDK and decode after reading a variable from SDK.

```C++
// containerUrl should be URL-encoded
auto containerClient = BlobContainerClient(containerUrl);
// The return value is URL-encoded
auto url = containerClient.Geturl();
// blobName should be UTF-8 encoded
auto blobClient = containerClient.GetBlobClinet(blobName);

for (auto page = blobContainerClient.ListBlobs(); page.HasPage(); page.MoveToNextPage()) {
  for (auto& blob : page.Blobs) {
    // blob.Name is UTF-8 encoded
    std::cout << blob.Name << std::endl;
  }
}
```

## How can I check if a blob exists? Is there a function `Exists()` like in legacy SDK?

There's no equivalent in the new C++ SDK. This function is usually just a convenient method on top of getting blob properties. You could easily write one like below.

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

1. Use `CreateIfNotExists()`, `DeleteIfExists()` functions whenever possible. These functions internally use access conditions and can help us catch unexpected exceptions caused by resending PUT/DELETE requests on network errors.
1. Use access conditions for other operations. Code below only sends one HTTP request, check-and-write is performed atomically. It only succeeds if the blob doesn't exist.
   ```C++
   UploadBlockBlobOptions options;
   options.AccessConditions.IfNoneMatch = Azure::ETag::Any();
   blobClient.Upload(stream, options);
   ```
1. Use lease for more complex scenarios that invole multiple operations on the same resource.

## Thread safety guarantees

All storage client APIs are thread-safe.
You can call the same API or different APIs on the same client instance from multiple threads without additional synchronization.
It's guaranteed to either return a successful response or throw an exception.

However, this doesn't mean access to underlying storage service has no race condition.
It is still possible that you'll get a 409 error if you read a file that's being written by antoher party. (For example, one thread reading an append blob while another thread appending data to it.)

## Will the SDK retry failed requests? What is the default retry logic? How can I customize retry behavior?

Requests faield due to network errors or HTTP status code 408, 500, 502, 503, 504 will be retried at most 3 times (4 attemps in total) using exponential backup with jitter.
These parameters can be customized with `RetryOptions`. Below is an example.

```C++
BlobClientOptions options;
options.Retry.RetryDelay = std::chrono::milliseconds(800);
options.Retry.MaxRetryDelay = std::chrono::seconds(60);
options.Retry.MaxRetries = 3;
```

## Http pipeline and policies

An HTTP pipeline consists of a sequence of steps executed for each HTTP request-response roundtrip.
Each policy has a dedicated purpose and acts on a request or a response or sometimes both.
When you send a request, the policies are executed in the order that they're added to the pipeline.
When you receive a response from the service, the policies are executed in reverse order.
All policies added to the pipeline execute before you send the request and after you receive a response.
The policy has to decide whether to act on the request, the response, or both.
For example, a logging policy logs the request and response but the authentication policy is only interested in modifying the request.

The Azure Core framework provides the policy with the necessary request and response data along with any necessary context to execute the policy.
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
These policies are automatically added into HTTP pipeline.

#### Custom HTTP pipeline policy

The HTTP pipeline policy provides a convenient mechanism to modify or decorate the request and response.
You can add custom policies to the pipeline that the user or the client library developer created.
When adding the policy to the pipeline, you can specify whether this policy should be executed per-call or per-retry.

To create a custom HTTP pipeline policy, you just inherit from a base policy class and implement some virtual functions.
You can then plug the policy into the pipeline.

#### Custom headers in HTTP requests

Below is an example of adding a custom header into each HTTP request.
The header value is static and doesn't change over time, so we make it a per-operation policy.
If you want to add some time-variant headers like authentication, you should use per-retry policy.

```C++
class NewPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
public:
  ~NewPolicy() override {}

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

## What is the difference between `BlockBlobClient::Upload()` and `BlockBlobClient::UploadFrom()`? When should I use one vs the other?

`BlockBlobClient::Upload()` takes a stream as parameter and uploads the stream as a block blob with exact one HTTP request.
The blob created with this method doesn't have any blocks, which means functions like `StageBlock()`, `CommitBlockList()` or `GetBlockList()` don't apply here.
You cannot append blocks to such blobs (You'll have to overwrite it with a new one).
You want to use this one if you need precise control over SDK behavior at HTTP level.

`BlockBlobClient::UploadFrom` takes a memory buffer or file name as parameter, splits the data in the buffer or file into smaller chunks intelligently or according to some parameters if provided,
then upload the chunks with multiple threads.
This one suits in most cases. You can expect higher throughput because the chunks are transferred concurrently. It's especially recommended if you need to tansfer large blobs efficiently.

## How to efficiently upload large amount of small blobs?

Unfortunately this SDK doesn't provide a convenient way to upload many blobs or directory contents (files and sub-directories) with just one function call.
You have to create multiple threads, traverse the directories by yourself and upload blobs one by one in each thread to speed up the transfer.
Or you can use tools like [AzCopy](https://learn.microsoft.com/en-us/azure/storage/common/storage-ref-azcopy) or Data Movement Library.

## How to ensure data integrity with transactional checksum?

Generally speaking, TLS protocol includes checksum that's strong enough to detect accidental corruption or deliberate tampering.
Another layer of checksum is usually not necessary if you're using HTTPS.

If you really want it for whatever reason, for example, to detect corruptions before the data is written to the socket,
you can leverage transactional checksum feature in the SDK.
With this feature, you provide a pre-calculated MD5 or CRC64 checksum when calling an upload API,
storage service will calculate checksum after it receives the data and compare with the one you provide,
and fail the request if they don't match.
Make sure you calculate the checksum early enough to cover the time when the corruption may happen.

This functionality also works for download operations.
Below is a code sample to use this feature.

```C++
// upload data with pre-calculated checksum
Blobs::UploadBlockBlobOptions options;
options.TransactionalContentHash = ContentHash();
options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
options.TransactionalContentHash.Value().Value = crc64;  // CRC64 checksum of the data in Base64 encoding
blobClient.Upload(stream, options);

// download and verify checksum
Blobs::DownloadBlobOptions options;
options.RangeHashAlgorithm = HashAlgorithm::Crc64;
auto response = blobClient.Download(options);
auto crc64 = response.Value.TransactionalContentHash.Value();
// Now you can verify checksum of the downloaded data
```

## Why do I see 503 Server Busy errors? What should I do in this case?

Azure storage service has scalability and performance target, which varies for different types of accounts.
When your application accesses storage too aggressively and reaches the limit, storage starts returning 503 Server Busy errors.
You may refer to [this page](https://learn.microsoft.com/azure/storage/common/scalability-targets-standard-account) for the limits for different types of storage accounts in different regions.

Here are a few things that turn out to be effective in practice to minimize the impact of this kind of error.

1. Use exponential backoff policy and jitter for retries.
   Exponential backoff allows the load on storage service to decrease over the time. Jitter helps ease out spikes and ensures the retry traffic is well-distributed over a time window.
1. Increase retry count.
1. Identify the throttling type and reduce the traffic sent from client side.
   You can check the exception thrown from storage function calls with below code. The error message will indicate which scalability target was exceeded.
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
1. Use CDN for static resources.

Throttling error could also happen at subscription level or tenant level. You can still try above approaches or contact Azure Support in such cases.

## How to troubleshoot 403 errors?

403 error means your request to access Azure Storage is not correctly authorized.

If you're using shared key authentication, you should

1. Check the storage account name and account key match the resource you're trying to access, and the key is not rotated.
1. If you need to add extra HTTP request headers or overwrite existing headers, do it with a HTTP pipeline policy, so that the new values can be signed with shared key.
1. If you use a customized HTTP client, do not make any changes to HTTP request in the client.

If you're using SAS authentication,

1. Check the values of query parameter `st` and `se`, which specify a time range the SAS token is valid. Make sure the token is not expired.
   If it's a user delegation SAS, also check `ske` which indicates the expiry of user delegation key.
1. Make sure the shared key used to generate the SAS token is not rotated.
1. If the SAS token is created with a stored access policy, make sure the access policy is still valid.
1. SAS token has its own scope, for example it may be scoped to a storage account, a container or a blob/file. Make sure you don't access a resource out of the SAS token's scope.
1. Check the message in exception.
   You could print the information in exception with code below.
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

You shouldn't write your own function to generate signatures or sign the requests. It's not a trivial task and there are many edge cases to cover. Use Official tools or SDK instead.

If you still cannot figure out the problem with above steps, you can open a GitHub issue with request ID or client request ID of the failed request.
