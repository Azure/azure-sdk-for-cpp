# Frequently-asked Questions and Common Mistakes

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
Client spawned from another class will inherit the settings.

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

We recommend you set an application ID with code below.

```C++
BlobClientOptions clientOptions;
clientOptions.Telemetry.ApplicationId = "SomeApplication v1.2.3";

// The User-Agent string will be something like:
// SomeApplication v1.2.3 azsdk-cpp-storage-blobs/12.9.0-beta.3 (Windows 10 Enterprise 6.3 19045 19041.1.amd64fre.vb_release.191206-1406)
```

## What encoding is used for input and output?

All URL should be URL-encoded, other resource names should be in UTF-8 encoding.
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
However, it is problematic if the destination might be modified from multiple threads or processes. A subsequent write operation may get overwritten by the previous one.
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
When you send a request, the policies execute in the order that they're added to the pipeline.
When you receive a response from the service, the policies execute in the reverse order.
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