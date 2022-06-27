# Batched Operations in Azure Core

TL;DR: The batch pattern for storage is highly specific to storage's scenarios and
does not match the patterns used for other Azure APIs. So batch operations should NOT
be a part of the Azure Core API surface.

The rest of this document discusses the pattern that the storage team should use to implement
batched operations. It is roughly based on the existing batch pattern proposed by the storage team.

## Proposed batched API design pattern for Azure Storage C++ Client

The following is the proposed solution to the storage batch client.

> ***Note:*** The `_internal` `DeferredResponseFactory` and
 `DeferredResponseProcessor` classes described below are not strictly
required for the design, but are examples of how the "batch" pattern operates.

The only customer facing construct is the `DeferredResponse<T>` type.

Note that a working example of this pattern can be found [on GitHub](https://github.com/LarryOsterman/azure-sdk-for-cpp/tree/larryo/batchprototype).

> NOTE:
> The following are the required elements of this pattern:
>
> 1. The `DeferredResponse<T>` type with a public GetResponse() method;
> 1. A `CreateBatchFactory()` method on the client class and
> 1. A `SubmitBatch()` method on the client class which takes a BatchFactory object as a parameter.
>     1. The SubmitBatch() throws all network layer exceptions,
>     1. The GetResponse() method throws any exceptions returned by the service.

Everything else below describes a possible implementation of the pattern. It is
understood that adjustments to the implementation may have to be made to meet the
implementation details of service client implementations.

There are three major components of this design:

1. `DeferredResponse<T>` - A `DeferredResponse` represents an `Azure::Response`
which has not yet been executed.
1. `DeferredResponseFactory` - A `DeferredResponseFactory` (or "Batch Factory")
is a factory for creating `DeferredResponse` objects - it exposes service methods
which are deferrable.
1. `DeferredResponseProcessor` - The `DeferredResponseProcessor` has two methods,
one of which returns a "Batch Factory" and the other which takes a
"Batch Factory" and processes all the deferred operations associated with the factory.
Note that `DeferredResponseProcessor` is a role in the pattern, it is not expected
to be a concrete implementation.

The following is a concrete representation of the pattern:

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
 * Note that the template argument for `DeferredResponse` does NOT include `Response`.
 * That is because a DeferredResponse IS a response, so including `Response` in
 * the template argument is redundant.
 *
 * > Note that calls to `GetResponse` are *not* idempotent. This is because a `Response<T>`
 * contains a unique pointer to the `RawResponse` object. Implementations need to guard
 * against multiple calls to `GetResponse`.
 */
template<typename T>
class DeferredResponse {
public:
    /**
     * @brief Retrieve the Response corresponding to this deferred operation.
     *
     * Note that this MAY throw an exception if the corresponding service method fails.
     *
     * @returns The Response corresponding to this deferred operation.
     *
     * @throws std::exception if the corresponding service method fails.
     *
     */
    virtual Response<T> GetResponse() = 0;
};

namespace _internal {
/**
 * @brief A DeferredResponseFactory creates DeferredResponse objects
 * which are created from an HTTP request to be sent to the service.
 *
 * @details
 * A `DeferredResponseFactory` functions as a factory for `DeferredResponse`s
 * and aggregator of deferred operations. It is intended to be used as a base
 * class which creates `DeferredResponse` objects and which collects intermediate
 * values used for the batch pattern. Typically a class derived from this class
 * will be the `Batch` aggregator reflected in the other batched API implementations.
 * The `DeferredResponseFactory` collects the parameters for each service method and 
 * returns a `DeferredResponse` object which corresponds to those aggregated service.
 */
class DeferredResponseFactory {
public:
    /***
     * @brief Creates a deferred operation with explicit shared state.
     */
      template <typename T>
      DeferredResponse<T> CreateDeferredResponse(
          std::shared_ptr<DeferredResponseSharedBase> deferredOperationShared);
    /***
     * @brief Creates a deferred operation from the supplied HTTP request object, specifying
     * a post-processing function.
     *
     * Create a deferred response for the specified request, and execute the processRawResponse
     * function to convert the response from the service to a Response object.
     * NOTE: If the processRawResponse parameter is implemented as a lambda, the lambda
     * MUST NOT capture any local variables by reference. Local variables will have
     * moved out of scope by the time the processRawResponse lambda is executed.
     *
     * @param requestToDefer 
     * @details Note that it is the callers responsibility to ensure that any state
     * captured in requestToDefer remains valid throughtout the lifetime of the
     * DeferredResponse<T>.
     */
    template<typename T>
    DeferredResponse<T> CreateDeferredResponse(Azure::Core::Http::Request requestToDefer,
        std::function<Response<T>(std::shared_ptr<RawResponse> rawResponse)> const& processRawResponse);
};

/**
 * @brief In the preferred implementation of this pattern, a DeferredResponseProcessor is 
 * normally not a discrete class, but instead is simply two methods on a service
 * client.
 */
class DeferredResponseProcessor {
public:
    /**
     * Returns a Batch object which will create deferred responses for service methods.
     */
    DeferredResponseFactory& CreateDeferredResponseFactory() = 0;

    /**
     * Submit a set of batched operations to the service
     */
    Response SubmitBatch(DeferredResponseFactory const& factory, Azure::Context context = Azure::Context{});
};
}}}
```

## Using the Batch pattern

The following is an example of a client using the Batch pattern.

In this example, the client creates a service client, then creates a Batch factory
from the client.

It then creates two `DeferredResponse` objects, one for a call to `SetAttestationPolicy`,
the next for a call to `ResetAttestPolicy`.

It then calls the `SubmitBatch` method on the service client which is responsible
for serializing the batch operation to the service and processing the response
from the service.

For each `DeferredResponse` object, the client may call the `GetResponse()` method
which will process the response from the service and return the response as if
the operation had been executed.

> [!NOTE] The `GetResponse()` method may (will) throw an exception if the service
failed the operation. In general, the `SubmitBatch` call will only fail if there
is a transport error communicating with the service.

```cpp
auto client = CreateClient();

AttestationBatchFactory batch = client->GetBatchFactory();
auto setOp = batch.SetAttestationPolicy(GetParam().TeeType,
    AttestationCollateral::GetMinimalPolicy());
auto resetOp = batch.ResetAttestationPolicy(GetParam().TeeType);

client->SubmitBatch(batch);

ValidateSetPolicyResponse(client, setOp.GetResponse(), AttestationCollateral::GetMinimalPolicy());

auto response = resetOp.GetResponse();
ValidateSetPolicyResponse(client, response, Azure::Nullable<std::string>());
```

### Proposed Pattern details

This proposal is functionally identical to the original .Net pattern and is largely consistent with the
"Batch" pattern implemented for Storage in other languages.

The pattern described above was intended as an abstract representation of the pattern - the actual apis implemented
by storage are likely to have a similar shape, but likely different names for the `_internal` types.

> [!NOTE]
> A batched operation should NOT take a `Context` object - a batched operation is not an actual network operation.

#### Shared State

Note that in this model, the `DeferredResponseFactory` MUST aggregate each of the
deferred responses returned by the various "Batch Factory" methods (because the
lifetime of the Batch Factory is independent from the lifetime of the returned
`DeferredResponse` objects). To implement this, the pattern implementation should
maintain state which is shared between the `DeferredResponse` objects and the
`DeferredResponseFactory`. The shared state should contain all that is needed to
send a request to the server and process the response received from the server.

#### `BatchFactory` Class

A concrete example of a "Batch" factory is:

```c++

class AttestationBatchFactory : public Azure::Core::_internal::DeferredResponseFactory {
public:
  Azure::Core::DeferredResponse<Models::AttestationToken<Models::PolicyResult>>
  SetAttestationPolicy(
      Models::AttestationType const& attestationType,
      std::string const& policyToSet,
      SetPolicyOptions const& options = SetPolicyOptions());

  Azure::Core::DeferredResponse<Models::AttestationToken<Models::PolicyResult>>
  ResetAttestationPolicy(
      Models::AttestationType const& attestationType,
      SetPolicyOptions const& options = SetPolicyOptions());
      :
      :
}
```

One possible implementation of the `SetAttestationPolicy` API above could be:

```cpp

Azure::Core::DeferredResponse<Models::AttestationToken<Models::PolicyResult>>
AttestationBatchFactory::SetAttestationPolicy(
    Models::AttestationType const& attestationType,
    std::string const& policyToSet,
    SetPolicyOptions const& options)
{
  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  Models::AttestationToken<void> const tokenToSend(
      m_parentClient->CreateAttestationPolicyToken(policyToSet, options.SigningKey));
  auto sharedContext = std::make_shared<
      AttestationBatchShared<Models::AttestationToken<Models::PolicyResult>>::SharedContext>(
      tokenToSend.RawToken);

  Azure::Core::Http::Request request
      = m_parentClient->CreateSetPolicyRequest(attestationType, sharedContext->BodyStream);

  AttestationTokenValidationOptions tokenOptions = options.TokenValidationOptionsOverride
      ? *options.TokenValidationOptionsOverride
      : m_parentClient->m_tokenValidationOptions;

  auto deferredShared(
      std::make_shared<AttestationBatchShared<Models::AttestationToken<Models::PolicyResult>>>(
          request,
          [this, tokenOptions](std::unique_ptr<Azure::Core::Http::RawResponse>& rawResponse) {
            switch (rawResponse->GetStatusCode())
            {
              // 200, 201, 202, 204 are accepted responses
              case Azure::Core::Http::HttpStatusCode::Ok:
              case Azure::Core::Http::HttpStatusCode::Created:
              case Azure::Core::Http::HttpStatusCode::Accepted:
              case Azure::Core::Http::HttpStatusCode::NoContent:
                break;
              default:
                throw Azure::Core::RequestFailedException(rawResponse);
            }
            return m_parentClient->ProcessSetPolicyResponse(tokenOptions, rawResponse);
          },
          sharedContext));

  std::shared_ptr<DeferredResponseSharedBase> sharedBase(deferredShared);
  return CreateDeferredResponse<Models::AttestationToken<Models::PolicyResult>>(sharedBase);
}

```

In this example, the `request` object depends on additional state (a `std::string` and
`BodyStream` object). To handle this case, the code creates a `AttestationBatchShared` object which
is shared between the `DeferredResponse<T>` object and the `DeferredResponseFactory`. This
object ensures that the data needed for the `request` is not freed until after the
batched request is processed.

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
