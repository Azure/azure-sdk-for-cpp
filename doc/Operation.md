# Operation\<T>

The Azure SDK for C++ defines an Operation as an abstract class to support a long running operation ([LRO](https://github.com/microsoft/api-guidelines/blob/vNext/Guidelines.md#13-long-running-operations)).

The SDK clients would define a concrete class from the `Operation<T>` which an end users can consume for getting the status of the running operation.

## Operation Status

The status of the operation defines the stages which the operation can be in from its creation to completion. The following table shows the supported status of an `Operation<T>`.

| Status     | Description                                                                    |
| ---------- | ------------------------------------------------------------------------------ |
| NotStarted | The operation is created but the server has not yet started it.                |
| Running    | The operation is running on the server side and has not yet been completed.      |
| Succeeded  | The server has successfully finished running the LRO. This is a `terminal stage`. |
| Cancelled  | The operation was interrupted before completion. This is a `terminal stage`.      |
| Failed     | The server failed to run the LRO. This is a `terminal stage`.                     |

> Note: A terminal stage means that the status won't change anymore.

## Consuming an Operation\<T>

Concrete classes derived from `Operation<T>` expose both `Poll()` and `PollUntilDone()` methods. `PollUntilDone()` returns only after operation has run to completion (success, failure or cancelled). Each call to `Poll()` sends a request to the service once time to check the status of the operation; each call to `Poll()` updates the `operation status`.

### PollUntilDone

`PollUntilDone()` repeatedly calls `Poll()` until the operation reaches a terminal stage ans either returns the final result (if the operation succeeds) or throws an exception (if the operation fails).

The code below demonstrates how to use `PollUntilDone()`.

```cpp
/**
 * Consuming Operation<T> interface. Calling PollUntilDone.
 *
 */
auto operationResult = client.StartLongRunningOperation(...);

try
{
    // Update each second
    auto response = operationResult.PollUntilDone(std::chrono::milliseconds(1000));

    // No need to check for `HasValue` here. The Operation will throw if the value can't be created after completing.
    auto valueT = response.ExtractValue();
    auto rawResponse = response.ExtractRawResponse();

}
catch (Azure::Core::RequestFailedException const& e)
{
    // Analyze the exception or re-throw.
}
```

### Poll

The `Poll()` method sends a request to the service to get the current status of the long-running operation (LRO).

The code below demonstrates how to use `Poll()`.

```cpp
/**
 * Consuming Operation<T> interface to Poll for LRO updates.
 *
 */
auto operationResult = client.StartLongRunningOperation(...);

while (true)
{
    // Get a read-only ref to the HTTP raw response after polling. Poll will update the status.
    auto& response = operationResult.Poll();

    if(operationResult.IsDone()) {
        // Operation completed.
        break;
    }

    // Read the HTTP headers in the response
    for (auto& header : response.GetHeaders()) {
        // ... consumer-code
    }
}

// Check is the Operation has created the value `T`
if (!operationResult.HasValue())
{
    // The Operation failed/cancelled.
    throw std::runtime_error("Operation finished with status:" + operationResult.Status());
}

// The operation completed successfully, use the response.
auto valueT = operationResult.Value();
```

## Operation\<T> lifetime

The `Operation<T>` holds an HTTP raw response. Each Operation\<T> derived class defines the moment when the HTTP raw response is moved to the Operation. This means that an Operation can be created without initially holding an HTTP raw response. In this case, calling `GetRawResponse` method from the Operation will `throw` as no HTTP raw response has been set for it yet.

The concrete Operation class defines since when the `GetRawResponse` can be called, we need to review each derived class to know at what time it is valid or not to call `GetRawResponse`.

The code below show two examples of Operation derive classes. The first one requires an HTTP raw response to be created, ensuring that `GetRawResponse` can be called at any moment after the Operation is created. The second class does not set an HTTP raw response when it is init, which means that `GetRawResponse` can only be called after calling `Poll`, otherwise it will throw.

```cpp
/**
 * Example of operations construction and `GetRawResponse` valid calls.
 *
 */

// The operation can only be constructed based on an HTTP raw response.
class OperationA : public Azure::Core::Operation<Model> {
public:
    OperationA(Azure::Core::Http::RawResponse rawResponse);
};

// This operation can be created with empty HTTP raw response.
class OperationB : public Azure::Core::Operation<Model> {
public:
    OperationB();
};

void main() {
    // Getting the HTTP raw response from OperationA is safe all the time.
    OperationA operationA(Azure::Core::Http::RawResponse(...));
    auto& httpRawResponse = operationA.GetRawResponse();

    // But operationB will throw
    OperationB operationB();
    // will throw, HTTP raw response not yet set.
    auto&  httpRawResponse = operationB.GetRawResponse();
}
```

The reason why the Operation\<T> is not asking all derived classes to be init based on an HTTP raw response is by design to allow each concrete operation to define its behavior up to the first `Poll()` call.

The Operation\<T> will maintain the ownership of the HTTP raw response and the value (once created). This means that `Extract` is not supported by the operation and calling `GetRawResponse()` and `Value()` will return read-only reference to its internal members.