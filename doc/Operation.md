# Operation\<T>

The Azure SDK for C++ defines an `Operation<T>` templated abstract class to support a long running operation ([LRO](http://restalk-patterns.org/long-running-operation-polling.html)).

The SDK clients would define a concrete class from the `Operation<T>` which an end users can consume for getting the status of the running operation.

## Operation Status

The status of the operation defines the stages which the operation can be in from its creation to completion. The following table shows the supported status of an `Operation<T>`.

| Status     | Description                                                                    |
| ---------- | ------------------------------------------------------------------------------ |
| NotStarted | The operation is created but the server has not yet started it.                |
| Running    | The operation is running on the server side and has not yet been completed.      |
| Succeeded  | The server has successfully finished running the LRO. This is a `final stage`. |
| Cancelled  | The operation was interrupted before completion. This is a `final stage`.      |
| Failed     | The server failed to run the LRO. This is a `final stage`.                     |

> Note: A final stage means that the status won't change anymore.

## Consuming an Operation\<T>

Any concrete class of `Operation<T>` will expose a `Poll()` and `PollUntilDone()` APIs. These methods will update the `operation status`. The status of an operation won't be automatically updated without calling `Poll()`.

### Poll

The `Poll()` method will send a request to the Server to get the current status of the `LRO`. The Server will then return an Http raw response with information about the completion of the LRO. The Operation parses the response and updates its status. Finally, a read only reference to the http raw response (the one used to update the status) is returned by `Poll()`. We can use this raw response to parse specific information contained in the response such a progress, time, percentage or other data. The operation will only check for changes in the `operation status`, but the operation-consumer will know better about all the data coming back from the server in the response.

The next code demonstrates the use of `Poll()`.

```cpp
/**
 * Consuming Operation<T> interface to Poll for LRO updates.
 *
 */
auto operationResult = client.ApiReturnOperation(...);

while (true)
{
    // Get a read-only ref to the Http raw response after polling. Poll will update the status.
    auto& response = operationResult.Poll();

    if(operationResult.IsDone()) {
        // Operation completed.
        break;
    }

    // Read the http headers in the response
    for (auto& header : response.GetHeaders()) {
        // ... consumer-code
    }
}

// Check is the Operation has created the value `T`
if (!operationResult.HasValue())
{
    // The Operation completed but it couldn't build the value `T`. Most likely the operation stage ended with error or it was cancelled.
    throw std::runtime_error("Operation finished with status:" + operationResult.Status());
}

// At this point, the Operation is success and we can get its value.
auto valueT = operationResult.Value();
```

### PollUntilDone

The `PollUntilDone()` is a convenience method for repeatedly call `Poll()` until the status is updated to a _final stage_. A `Response<T>` will be returned wrapping the model `T` and the Http raw response from which it was created. This method is useful when we are not interested about the Http raw response for each polling request.

The `PollUntilDone()` method requires an `std::chrono::milliseconds` input parameter which controls how much time to wait between calling `Poll`.

In the code below, instead of manually polling for updates, the `PollUntilDone` method is called.

```cpp
/**
 * Consuming Operation<T> interface. Calling PollUntilDone.
 *
 */
auto operationResult = client.ApiReturnOperation(...);

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

## Operation\<T> lifetime

The `Operation<T>` holds an Http raw response. Each Operation\<T> derived class defines the moment when the Http raw response is moved to the Operation. This means that an Operation can be created without initially holding an Http raw response. In this case, calling `GetRawResponse` method from the Operation will `throw` as no http raw response has been set for it yet.

The concrete Operation class defines since when the `GetRawResponse` can be called, we need to review each derived class to know at what time it is valid or not to call `GetRawResponse`.

The code below show two examples of Operation derive classes. The first one requires an http raw response to be created, ensuring that `GetRawResponse` can be called at any moment after the Operation is created. The second class does not set an http raw response when it is init, which means that `GetRawResponse` can only be called after calling `Poll`, otherwise it will throw.

```cpp
/**
 * Example of operations construction and `GetRawResponse` valid calls.
 *
 */

// The operation can only be constructed based on an http raw response.
class OperationA : public Azure::Core::Operation<Model> {
public:
    OperationA(Azure::Core::Http::RawResponse rawResponse);
};

// This operation can be created with empty http raw response.
class OperationB : public Azure::Core::Operation<Model> {
public:
    OperationB();
};

void main() {
    // Getting the Http raw response from OperationA is safe all the time.
    OperationA operationA(Azure::Core::Http::RawResponse(...));
    auto& httpRawResponse = operationA.GetRawResponse();

    // But operationB will throw
    OperationB operationB();
    // will throw, http raw response not yet set.
    auto&  httpRawResponse = operationB.GetRawResponse();
}
```

The reason why the Operation\<T> is not asking all derived classes to be init based on an Http raw response is by design to allow each concrete operation to define its behavior up to the first `Poll()` call.

The Operation\<T> will maintain the ownership of the Http raw response and the value (once created). This means that `Extract` is not supported by the operation and calling `GetRawResponse()` and `Value()` will return read-only reference to its internal members.

## Creating an Operation\<T>

As any concrete class, the Operation\<T> defines the next contract to be implemented:

| Method | Description | Highlights |
| --- | --- | --- |
| PollInternal | Implement how to get an update for the operation. Although the update can come from anywhere, it needs to be returned as a unique_ptr<Azure::Core::Http::RawResponse> which the Operation will own. | - This method **must** update the operation status.<br>- This is the only expected method where the operation status should be updated.<br>- If the operation status is updated to `completed` and is *successful*, this method **must** init the value\<T> model before returning (if the implementation of `T Value()` will just return the internal value field.).<br>- This method is not expected to be exposed as public, but implemented as private. |
| PollUntilDoneInternal | Implement an strategy for keep polling for updates and return the `value\<T>` once the operation has successfully finished. | - This method **must** set the member field `value\<T>` before returning it if the implementation of `T Value()` will just return the internal value field.<br>- This method is not expected to be exposed as public, but implemented as private. |
| GetRawResponseInternal | Usually this implementation would just return the reference to the http raw response member, but it can be further used to make changes to the http raw response if required. | - This method *does not* need to check if the http raw response member is not set before returning it, the Operation\<T> would add that validation for all derived classes.<br>- This method is not expected to be exposed as public, but implemented as private. |
| Value | Implement the way the model `T` is returned. | - If the `Poll` implementation is creating the model `T` when the operation is completed and updates the value member from the operation to hold the value `T`, then this method should only return the member value.<br>- If `Poll` is not creating the model `T`, this method **must** init the value `T` and return it. |
| GetResumeToken | For operations that support resuming, this method defines how to generate or provide the resume token. | - If the operation does not support `resume`, this method **must** call `std::abort`. |

The next code show how to create an Operation\<T> concrete class that uses `Poll` to update the `value` member field.

```cpp
/**
 * Example for a concrete Operation<T> using Poll to create model T.
 *
 */

// Implementing Operation<T> contract
class fooOperation : public Azure::Core::Operation<Model> {

  private:
    // Define a private member for the value T = Model
    Model m_valueT;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override {
        
        // `GetFooStatus` return a Response<fooOperationProgressResult> model which is used to set the Operation status. 
        auto response = GetFooStatus();
        if (!response->Status.HasValue())
        {
            m_status = Azure::Core::OperationStatus::Failed;
        }
        else if (response->Status.GetValue() == Model::FooStatus::InProgress)
        {
            m_status = Azure::Core::OperationStatus::Running;
        }
        else if (response->Status.GetValue() == Model::FooStatus::Success)
        {
            m_status = Azure::Core::OperationStatus::Succeeded;
            // Create the model T and make Operation to own it.
            m_valueT = InitModel(response);
        }
        else
        {
            m_status = Azure::Core::OperationStatus::Failed;
        }

        return response.ExtractRawResponse();
    }

    Azure::Response<Model> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override {
      
        while (true)
        {
            // Poll will update the operation status
            auto rawResponse = Poll(context);

            if (m_status == Azure::Core::OperationStatus::Succeeded)
            {
                // Response<T> can be created now. Note how we need to make a copy of the http raw response.
                // This is because Operation must never give up the ownership.
                return Azure::Response<Model>(
                    m_valueT, std::make_unique<Azure::Core::Http::RawResponse>(rawResponse));
            }
            else if (m_status == Azure::Core::OperationStatus::Failed)
            {
                throw Azure::Core::RequestFailedException("Operation failed");
            }
            else if (m_status == Azure::Core::OperationStatus::Cancelled)
            {
                throw Azure::Core::RequestFailedException("Operation was cancelled");
            }

            std::this_thread::sleep_for(period);
        };
    }

    // Operation<T> base class takes care of validating `m_rawResponse`.
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::string GetResumeToken() const override
    {
      // Not supported
      std::abort();
    }

  public:
   // Since Poll is updating the value T, we can return it here. 
    Model Value() const override { return m_valueT; }

  };
```

Now in the next code sample, the operation is not using `Poll` to init the Model T, and instead, it waits until `Value()` is called to generate the model. By doing this, the Operation no longer needs to keep a model T member, since it is always build on demand from the http raw response.

```cpp
/**
 * Example for a concrete Operation<T> with no value member. Creates the value from the http raw response from `Poll`.
 *
 */

// Implementing Operation<T> contract
class fooOperation : public Azure::Core::Operation<Model> {

  private:
    // Note: No private member for the value T required.

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override {
        
        // `GetFooStatus` return a Response<fooOperationProgressResult> model which is used to set the Operation status. 
        auto response = GetFooStatus();
        if (!response->Status.HasValue())
        {
            m_status = Azure::Core::OperationStatus::Failed;
        }
        else if (response->Status.GetValue() == Model::FooStatus::InProgress)
        {
            m_status = Azure::Core::OperationStatus::Running;
        }
        else if (response->Status.GetValue() == Model::FooStatus::Success)
        {
            m_status = Azure::Core::OperationStatus::Succeeded;
            // No need to create the model here, we will wait until someone ask for it to init it.
        }
        else
        {
            m_status = Azure::Core::OperationStatus::Failed;
        }

        return response.ExtractRawResponse();
    }

    Azure::Response<Model> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override {
      
        while (true)
        {
            // Poll will update the operation status
            auto rawResponse = Poll(context);

            if (m_status == Azure::Core::OperationStatus::Succeeded)
            {
                // Response<T> can be created now. Note how we need to make a copy of the http raw response.
                // This is because Operation must never give up the ownership.
                // Note how the T is created from the rawResponse.
                return Azure::Response<Model>(
                    Model(rawResponse), std::make_unique<Azure::Core::Http::RawResponse>(rawResponse));
            }
            else if (m_status == Azure::Core::OperationStatus::Failed)
            {
                throw Azure::Core::RequestFailedException("Operation failed");
            }
            else if (m_status == Azure::Core::OperationStatus::Cancelled)
            {
                throw Azure::Core::RequestFailedException("Operation was cancelled");
            }

            std::this_thread::sleep_for(period);
        };
    }

    // Operation<T> base class takes care of validating `m_rawResponse`.
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::string GetResumeToken() const override
    {
      // Not supported
      std::abort();
    }

  public:
    // Since Poll is not creating the Model, it is created at this point instead based on the http raw response.
    Model Value() const override { return Model(*m_rawResponse); }

  };
  ```
  
