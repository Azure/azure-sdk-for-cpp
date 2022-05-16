# Batched Operations in Azure Core

TL;DR: The batch pattern for storage is highly specific to storage's scenarios and
does not match the patterns used for other Azure APIs. So batch operations should NOT
be a part of the Azure Core API surface.

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

* The batch pattern should enable aggregating multiple operations into a single REST API call
  * The batch client will operate on the aggregated operations in a client specific fashion.
* The batch pattern must support operations which are authored from different azure identities
  * This requirement comes from the storage team.
* The batch pattern needs to be able to return objects which cannot be copied (objects
 with a deleted copy constructor and assignment operator).
* The batch pattern should feel like a native C++ pattern.

## Existing batch API patterns

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

Note that the pattern described above does *not* support the requirement that operations be authored
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

### Proposed batched API design pattern

As an alternate to the original `Later<T>` design, Larry Osterman proposes the following
pattern for Batch operations.

```cpp
namespace Azure { namespace Core {
/**
 * \brief A DeferredOperation represents an operation which will be deferred
 * until after a call to the `SubmitBatch` API.
 */
template<typename T>
class DeferredOperation{
public:
    /**
     * \brief Retrieve the Response corresponding to this deferred operation.
     */
    virtual Response<T> GetResponse() = 0;
};

/**
 * \brief A BatchFactor creates DeferredOperation objects which are created from
 * an HTTP request to be sent to the service.
 */
class DeferredOperationFactory {
public:
    /***
     * \brief Creates a deferred operation from the supplied HTTP request object.
     *
     * The DeferredOperationProcesor is responsible for converting the requestToDefer
     * parameter to a Response<T>.
     */
    template<typename T>
    DeferredOperation<T> CreateDeferredOperation(Azure::Core::Http::Request requestToDefer);

    /***
     * \brief Creates a deferred operation from the supplied HTTP request object, specifying
     * a post-processing function.
     *
     * Create a deferred response for the specified request, and execute the processRawResponse
     * function to convert the response from the service to a Response object.
     * NOTE: If the processRawResponse parameter is implemented as a lambda, the lambda
     * MUST NOT capture any local variables by reference. Local variables will have
     * moved out of scope by the time the processRawResponse lambda is executed.
     */
    template<typename T>
    DeferredOperation<T> CreateDeferredOperation(Azure::Core::Http::Request requestToDefer,
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
    DeferredOperation<T> CreateDeferredOperation(Azure::Core::Http::Request requestToDefer,
        std::shared_ptr<TokenCredential const> clientCredentials,
        std::function<Response<T>(std::shared_ptr<RawResponse> rawResponse)> const& processRawResponse);
};

class DeferredOperationProcessor {
public:
    DeferredOperationFactory& CreateDeferredOperationFactory() = 0;
    Response<void> SubmitBatch(DeferredOperationFactory const& factory, Azure::Context context = Azure::Context{});
};
}}
```

Here is how this pattern would be used for Storage (note that for maintain consistency
with Java and .Net, the name of the objects changed from DeferredOperation to Batch):

```cpp
// "batch" derives from Azure::Core::BatchFactory.
// Note that GetBatchFactory returns a reference to a `batch` object. This is because
// while the batch object is logically independent of the batch client object,
// in practice their lifetime is closely tied.
//
// Note: This restriction comes because the BatchFactory needs to be able to
Azure::Storage::Blobs::BlobBatchFactory& batchFactory= blobBatchClient.GetBatchFactory();
DeferredOperation<void> delete1 = batch.DeleteBlob("sample1");
DeferredOperation<void> delete2 = batch.DeleteBlob("sample2");

Azure::Response<void> blobBatchClient.SubmitBatch(batchFactory, context);

// GetResponse may throw an exception if processing the response.
Azure::Response<void> delete1.GetResponse();

// If the client wants to handle and process the storage result.
try {
    auto deleteResponse2 = delete2.GetResponse();
    // successful
} except (Azure::Storage::Exception& e) {
    // error
}
```

This proposal is functionally identical to the original .Net pattern. This pattern
is somewhat confusing because the BatchFactory is also an aggregation of operations.
It would be better if the factory was an actual factory which allows the caller to aggregate
operations separately, however that cannot work because you cannot aggregate different
types in a C++ collection.
