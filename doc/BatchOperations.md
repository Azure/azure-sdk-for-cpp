# Batched Operations in Azure Core

TL;DR: The batch pattern for storage is highly specific to storage's scenarios and
does not match the patterns used for other Azure APIs. So batch operations should NOT
be a part of the Azure Core API surface.

<<<<<<< Updated upstream
=======
The rest of this document discusses the pattern that the storage team should use to implement
batched operations. It is roughly based on the existing batch pattern proposed by the storage team.

## Proposed batched API design pattern for Azure Storage C++ Client

The following is the proposed solution to the storage batch client.

> ***Note:*** The `_internal` `DeferredResponseFactory` and
 `DeferredResponseProcessor` classes described below are not strictly
required for the design, but are examples of how the "batch" pattern operates.

The only customer facing construct is the `DeferredResponse<T>` type.

```cpp

namespace Azure { namespace Storage { namespace Blob { namespace Batch {
/**
* \brief A DeferredResponse represents an operation which will be deferred
* until after a call to the `SubmitBatch` API.
*
* @details A `DeferredResponse<T>` object represents a service method which has
* not yet completed. The name was chosen * to reflect the .Net and Java 
* implementations, which return `Response<T>` objects, but the `DeferredResponse`
* name reflects that this is NOT an * * actual response, but instead a "proxy object"
* which can be used to retrieve the actual response after the request has completed.
*
* Calling the `GetResponse` API before the `SubmitBatch` method on the *
* `DeferredResponseProcessor` has been called is * an error and SHOULD throw a
* descriptive exception (especially if the alternative is throwing an access violation).
*
* Calling the `GetResponse` API _after_ calling the `SubmitBatch` method may
* throw for the same reasons that any other service method might throw - the
* call to `GetResponse` should behave as close as possible to the original API
* operation.
*
*/
template<typename T>
class DeferredResponse {
public:
    /**
     * @brief Retrieve the Response corresponding to this deferred operation.
     *
     * Note that this MAY throw an exception if the corresponding service method fails.
     */
    virtual Response<T> GetResponse() = 0;
};

namespace _internal {
/**
 * @brief A DeferredResponseFactory creates DeferredResponse objects
 * which are created from an HTTP request to be sent to the service.
 * @details
 * A `DeferredResponseFactory` functions as a factory for `DeferredResponse`s
 * and aggregator of deferred operations. It is intended to be used as a base
 * class which creates `DeferredResponse` objects and which collects intermediate
 * values used for the batch pattern. Typically a class derived from this class
 * will be the `Batch` aggregator reflected in the other batched API implementations.
 * The `DeferredResponseFactory` collects the parameters for each service method and 
 * returns a `DeferredResponse` object which corresponds to those aggregated service
 */
class DeferredResponseFactory {
public:
    /***
     * @brief Creates a deferred operation from the supplied HTTP request object.
     *
     * The DeferredResponseProcessor is responsible for converting the requestToDefer
     * parameter to a Response<T>.
     */
    template<typename T>
    DeferredResponse<T> CreateDeferredResponse(Azure::Core::Http::Request requestToDefer);

    /***
     * @brief Creates a deferred operation from the supplied HTTP request object, specifying
     * a post-processing function.
     *
     * Create a deferred response for the specified request, and execute the processRawResponse
     * function to convert the response from the service to a Response object.
     * NOTE: If the processRawResponse parameter is implemented as a lambda, the lambda
     * MUST NOT capture any local variables by reference. Local variables will have
     * moved out of scope by the time the processRawResponse lambda is executed.
     */
    template<typename T>
    DeferredResponse<T> CreateDeferredResponse(Azure::Core::Http::Request requestToDefer,
        std::function<Response<T>(std::shared_ptr<RawResponse> rawResponse)> const& processRawResponse);

    /***
     * \brief Creates a deferred operation from the supplied HTTP request object, specifying
     * a post-processing function and specifying specific credentials to be used in the
     * operation.
     *
     * Create a deferred response for the specified request, and execute the processRawResponse
     * function to convert the response from the service to a Response object.
     * NOTE: If the processRawResponse parameter is implemented as a lambda, the lambda
     * MUST NOT capture any local variables by reference. Local variables will have
     * moved out of scope by the time the processRawResponse lambda is executed.
     */
    template<typename T>
    DeferredResponse<T> CreateDeferredResponse(Azure::Core::Http::Request requestToDefer,
        std::shared_ptr<TokenCredential const> clientCredentials,
        std::function<Response<T>(std::shared_ptr<RawResponse> rawResponse)> const& processRawResponse);
};

class DeferredResponseProcessor {
public:
    DeferredResponseFactory& CreateDeferredResponseFactory() = 0;
    Response SubmitBatch(DeferredResponseFactory const& factory, Azure::Context context = Azure::Context{});
};
}}}
```

### Proposed Pattern details

This proposal is functionally identical to the original .Net pattern and is largely consistent with the
"Batch" pattern implemented in other services.

The pattern described above was intended as an abstract representation of the pattern - the actual apis implemented
by storage are likely to have a similar shape, but likely different names for the `_internal` types.



> [!NOTE:]
> A batched operation should NOT take a `Context` object - a batched operation is not an actual network operation.

This API surface is likely to be somewhat confusing to C++ developers because C++ classes don't normally function as both aggregations AND
factories. For C++ developers, it would be better if the factory was an actual factory which allows the caller to aggregate
operations separately, however that solution cannot meet the requirements for the pattern because you cannot aggregate different
types in a C++ collection using C++14 semantics (it would be possible with std::variant or using RTTI, but unfortunately those options are not
available).

#### Internal: `DeferredResponseFactory`

The `DeferredResponseFactory` class as described above is *not* a strict requirement
of the pattern.

One possible implementation of this pattern derives the `Batch` object from a `DeferredResponseFactory`:

```c++
  class StorageBatch : Azure::Core::_internal::DeferredResponseFactory {


  private:
    StorageBatch() : Azure::Core::_internal::DeferredResponseFactory()
    {
    }

```

Another possible implementation has the `DeferredResponseFactory` as a member of the class:

```c++
  class StorageBatch {


  private:
    Azure::Core::_internal::DeferredResponseFactory deferredFactory;


    StorageBatch() : Azure::Core::_internal::DeferredResponseFactory()
    {
    }

```

The `Batch` objects service methods will return the value of the `DeferredResponseFactory`'s `CreateDeferredResponse` method:

```c++
Azure::Core::DeferredResponse<Models::ModelTemplate<Models::ModelType>>
StorageBatch::PerformServiceMethod(
    std::string const& serviceParameter,
    SetPolicyOptions const& options)
{
  Azure::Core::Http::Request request = CreateServiceMethodRequest(serviceParameter, options);

  // Create a deferred operation executing the specified lambda when the request completes.
  // Note that the lambda capture needs to be careful to ensure that the lifetime of all the captured
  // fields is equal to or greater than the lifetime of the `Batch` object.
  return deferredFactory.CreateDeferredResponse<Models::ModelTemplate<Models::ModelType>>(
      request, [](std::unique_ptr<Azure::Core::Http::RawResponse>& rawResponse) {
        auto result = ProcessSetPolicyResponse(rawResponse);
        return result;
      });
}
```


#### Internal: `DeferredResponseProcessor`

The `DeferredResponseProcessor` is a role in the pattern, not typically a concrete class. It exists to complete the pattern. The `DeferredResponseProcessor`
implements the `SubmitBatch` API which formats a request to the service aggregating the service requests in the provided `DeferredResponseFactory` object. It
is responsible for ensuring that when a client calls `GetResponse<T>` on the `DeferredResponse` object that the response will function correctly.

>>>>>>> Stashed changes
## Discussion

Azure Blob Storage implements a concept called "batched operations" - the
service client collects a sequence of operations and then applies those separate operations
in a single REST API call. Currently those operations are limited to "Delete" and
"Set Blob Access Tier". Interestingly, these operations do NOT have a return value,
which significantly simplifies the API surface.

Based on investigation of the existing SDKs, the Storage batch operation appears to be unique.
While there are other Azure services which implement batching functionality, they do not appear
to use the same mechanisms that the Storage API uses.

The core requirements for the Batch pattern are:

- The batch pattern should enable aggregating multiple operations into a single REST API call
  - The batch client will operate on the aggregated operations in a client specific fashion.
- The batch pattern must support operations which are authored from different azure identities
  - This requirement comes from the storage team.
- The batch pattern needs to be able to return objects which cannot be copied (objects
  with a deleted copy constructor and assignment operator).
- The batch pattern should feel like a native C++ pattern.

## Existing batch API patterns

=======
- The batch pattern should enable aggregating multiple operations into a single REST API call
  - The batch client will operate on the aggregated operations in a client specific fashion.
- The batch pattern must support operations which are authored from different azure identities
  - This requirement comes from the storage team.
- The batch pattern needs to be able to return objects which cannot be copied (objects
  with a deleted copy constructor and assignment operator).
- The batch pattern should feel like a native C++ pattern.

## Existing batch API patterns

The .Net client for several Azure APIs which support batch operations were surveyed to determine existing patterns
for batch processing. The surveyed clients were:

- Azure Monitor
- Azure EventHub
- Azure Search Documents
- Azure Blob Storage Batch
- Microsoft CognitiveServices Vision CustomVision Training
- Microsoft CognitiveServices Language LUIS Authoring
- Azure CommunicationThread (and ACS in general - there are a few ACS APIs that do batch)

Most of the batch APIs surveyed follow a common pattern of "client creates an aggregator object and passes the aggregator object as an input to the batch APIs".

Based on the survey results, there are three patterns implemented.

1. "atomic" results batched operations. Used by Azure Monitor, EventHub and others. In this pattern, the
   aggregator accepts a series of objects and when the aggregator object is submitted (via a `SubmitBatch` method),
   the API returns a single success or failure.
1. "partial" results batched operations. There are two variants of the "partial" results operation:
   1. The developer provides an ID for each batched entity and when the service responds, it includes
      the batched ID in the results which allows the customer to determine which operation failed. This pattern is used
      by Search Documents, Cognitive Services, ACS and other clients.
   1. The batched operation simulates a non-batched operation. The batch aggregator returns a `Response<T>` and when the client
      attempts to retrieve the `Value` of the response, the API processes the result of the operation and behaves as if it were an
      un-batched API. This pattern is only used by the Blob Storage Batch API.

### Storage Blob Batch Client implementation (cross language)

>>>>>>> Stashed changes
Each languages implementation of the Blob Storage batch functionality is slightly
different, based on the requirements of each language, but most of the languages
share a common development paradigm.

In this pattern, the `StorageBatchClient` class exposes a `CreateBatch` method which returns
a `Batch` object. The `Batch` object exposes a set of methods analogous to the
service methods and returns a `Result` or `Result<void>` object (depending on the language).

== Logically the return value of the `Batch` object methods is the return value of the service method.==

The `StorageBatchClient` then exposes a `SubmitBatch` method which takes the `Batch`
object as an input and submits each of the requests associated with the batch operation.

After that API has completed, the developer can then inspect the `Result` objects returned
from the `Batch` object to determine the ultimate return from the API.

Note that the pattern described above does _not_ support the requirement that operations be authored
from different azure identities (because the Batch object returned by the StorageBatchClient has no
mechanism to accept requests from different Azure identities).

## Proposed Azure Core API for supporting Batch operations

### Original Proposal

The original proposal for Batch was modeled after the C# and Java solutions for batch processing.

The client creates a `Batch` object, calls into "operation" methods to create batched operations,
collecting results for those operations. The "operation" methods return a `Later<T>` where `T` is the
nominal return value from the operation.

The client then passes the `Batch` object to a `SubmitBatch` method on the `BatchClient` object which
executes the batched operations remotely.

The client can call the `get` method on the `Later<T>` to retrieve the `T` object
corresponding to the returned call.

It is an error to call the `get` method before calling the `SubmitBatch` method.

Here is an example (from the original `Later<T>` spec) of how a developer might
use the original Batch API proposal:

```c++
int main() {
  Batch batch;
  auto laterResult = batch.SomeOperation();
  auto laterResult2 = batch.AnotherOperation();

  BatchClient().SubmitBatch(batch);

  try {
    auto result = laterResult.get();
    std::cout << result << std::endl;
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  auto result2 = laterResult2.get();
  std::cout << result2.m_a << std::endl;
}
```

The Storage team provided the following as a concrete example of how the pattern would be used in C++:

```c++
auto batch = blobBatchClient.CreateBatch();

auto laterResponse1 = batch.DeleteBlob("sample1");
auto laterResponse2 = batch.DeleteBlob("sample2");

blobBatchClient.SubmitBatch(batch);

auto deleteResponse1 = laterResponse1.Get();

try {
    auto deleteResponse2 = laterResponse2.Get();
    // successful
} except (Azure::Storage::Exception& e) {
    // error
}
```

#### Commentary on the original `Later<T>` Design

##### Advantages

This design is almost identical to the original .Net and Java design. That means that it is extremely
familiar to developers who are used to using the .Net and Java pattern.

##### Disadvantages

There are a couple of challenges with this design - the first is that the original pattern for
the batch object is independently creatable - that runs counter to many of the azure design
guidelines, but is simple to fix - add a `CreateBatch` method to the `BatchClient` (that is
what .Net and Java do as well).

The second issue is that the `T` object in the original proposal needs to be a `Response<T>` object
which requires double nesting of objects, which is unpleasent - for Java and .Net, the `Batch` object
returned a `Response<T>` object directly.

And finally, the `Later<T>` proposal

