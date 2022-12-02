# HTTP Transport Adapter

The Azure SDK for C++ creates and sends HTTP requests using an HTTP Transport Adapter. This adapter provides a general abstraction for an HTTP stack (like libcurl, WinHTTP, etc.) by defining the interface it needs to call that anyone can implement to customize which HTTP stack gets used.

## Default HTTP Transport Adapter

The Azure SDK for C++ provides two HTTP transport adapters as part of the azure-core package:
- The libcurl Transport Adapter

<center>

| Supported Platforms  |
| --- |
| Linux |
| Windows |
| macOS |

</center>

- The WinHTTP Transport Adapter

<center>

| Supported Platforms  |
| --- |
| Windows |

</center>

Users can create and use their own transport adapter [by following the guidance below](#building-a-custom-http-transport-adapter).

For the simplest case, when no specific configuration is set, the default transport adapter will be selected based on the OS. The WinHTTP transport adapter will be used for Windows-based systems (making libcurl an optional dependency). The libcurl transport adapter will be selected for any non-Windows-based system. Read more about using the libcurl transport adapter here.

## Building the HTTP Transport Adapter

The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required for each HTTP transport adapter.

Multiple HTTP transport adapters can be built as part of the project. This is to support scenarios where you want to send HTTP request with the libcurl transport adapter for some cases and the WinHTTP transport adapter for others.

Another example is if you want to create your own HTTP transport adapter. In this case, you can build your own HTTP transport adapter and test it by comparing it to the behavior of the libcurl HTTP transport adapter.

## TLS 1.2

The transport adapters from the Azure SDK (libcurl and winHTTP) enforces the use of Transport Layer Security version 1.2. If you need to use an older version, please do it by creating a fork to update the source code. Or consider creating a [custom transport adapter](#building-a-custom-http-transport-adapter) implementation within your application source code.

## Using the HTTP Transport Adapter

The HTTP transport adapter is set up during service client initialization, for example, when creating an Azure Storage client. The HTTP transport adapter can be specified at client initialization via the client options argument. The example below shows how to use the default HTTP transport adapter when constructing an SDK client:
```cpp
  /*
  * Using the default HTTP transport adapter.
  */

  // When no options are provided, the HTTP transport adapter is set to the default one
  // based on the compiler options passed in to CMake.
  auto defaultStorageClient = BlobServiceClient(url, credential);
```

The example below shows how to override the default HTTP transport adapter when constructing an SDK client:

```cpp
  /*
  * Override the default HTTP transport adapter.
  */

  // Use client options to set an HTTP transport adapter, which needs to be a shared_ptr.
  BlobClientOptions options;
    // Setting the libcurl transport adapter as an example.
  // Any HTTP transport adapter can be specified here.
  options.TransportOptions.Transport = std::make_shared<Azure::Core::Http::CurlTransport>();

  auto storageClient = BlobServiceClient(url, credential, options);
```

### Re-use the HTTP Transport Adapter

Note that the HTTP transport adapter is a `shared_ptr`. This is because you can re-use the same HTTP transport adapter for multiple clients. There are two ways of doing this. The first one is by using the same HTTP transport adapter when creating another client. The second one is when you get a specific child client from an already created parent client directly, in which case the child client inherits the same options. See the next example:

```cpp
  /*
  *  Option 1. Init a new client with the same HTTP transport adapter.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions optionsA;
  optionsA.TransportOptions.Transport = curlTransportAdapter;
  auto storageClientA = BlobServiceClient(url, credential, optionsA);
  // The second client.
  BlobClientOptions optionsB;
  optionsB.TransportOptions.Transport = curlTransportAdapter;
  auto storageClientB = BlobServiceClient(url, credential, optionsB);

  /*
  *  Option 2. Create new client from a parent client.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions options;
  options.TransportOptions.Transport = curlTransportAdapter;
  auto storageClient = BlobServiceClient(url, credential, options);
  // Create specific child client from the parent service client.
  auto blobContainerClient = storageClient.GetBlobContainerClient(containerName)
```

Each client holds an internal copy of the options passed in the arguments. This means that the options object used to create the client can be disposed. However, since the HTTP transport adapter is a shared pointer, it won't be disposed until all clients using it are disposed first.

## HTTP Transport Adapter Options

The libcurl and WinHTTP transport adapters can also be initialized with specific options that expose control of features from the HTTP stack underneath them. For example, you can set a proxy or override the default certificate authority (CA) path for libcurl. See the following example:
```cpp
  #include "azure/core/http/curl_transport.hpp"
  
  /*
  * Customize libcurl HTTP transport adapter.
  */

  // Create the HTTP transport adapter options and set them when initializing the adapter.
  Azure::Core::Http::CurlTransportOptions curlTransportOptions;
  curlTransportOptions.Proxy = "https://my.proxy.com";
  // Create HTTP transport adapter with options.
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlTransportOptions);

  BlobClientOptions options;
  options.Transport.Transport = curlTransportAdapter;
  auto storageClient = BlobServiceClient(url, credential, options);
```

## Building a Custom HTTP Transport Adapter

The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required to be set when building your own HTTP transport adapter.

Follow these steps to implement your own HTTP transport adapter:

1. Implement the interface.

The HTTP transport adapter interface is define in `Azure::Core::HTTP::HttpTransport` from `inc/azure/core/http/transport.hpp`.

```cpp
/*
* Implementing the HTTP transport adapter.
*/

#include "azure/core/http/transport.hpp"

// Derive from HttpTransport
class CustomTransportAdapter : public Azure::Core::Http::HttpTransport {
  public:
    // Override the Send method
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) override;
};
```

> The implementation needs to take care of using the members from the `Request` argument which contains all the HTTP request information to write the HTTP request using some C++ HTTP stack. Then it needs to create the `RawResponse` from the response and return it. The `context` argument can be optionally used to check if the request is cancelled before performing I/O operations (like writing to the network).

2. Stateless component.

Make sure that the HTTP transport adapter that is created is stateless. This is because it can be shared from multiple SDK clients as a shared ptr. Consider designing a session object, for instance, if you need to hold some state. See the next pattern as an example.

```cpp
/*
* Using a session pattern for the HTTP transport adapter.
*/

// Create a session class first.
class CustomHttpTransportAdapterSession {
  private:
    // Keep any state like network/socket handlers here.
    SomeHttpClientLibraryHandler handler;
    SomeOtherRequiredState state;

  public:
    // Expose any functionality required. For example, this could
    // be the main method to produce the HTTP raw response.
    std::unique_ptr<Azure::Core::Http::RawResponse> Perform(
      Azure::Core::Context const& context,
      Azure::Core::Http::Request& request
    );
}
```

Then, within the `Send()` method implementation:

```cpp
// Create a new Session object when calling Send().
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) {

          // This will ensure that multiple clients can
          // call Send() at the same time.
          CustomHttpTransportAdapterSession session;

          // Use any functionally from your created session.
          // For this example, we could just return like this:
          return session.Perform();
        }

```

> If instead of having a session created for every call to Send(), we would place the state directly into the HTTP transport adapter class, any new call to Send() would corrupt the current state when sharing the same HTTP transport adapter from multiple clients.

3. Use the adapter.

At this point you have all you need to use your custom HTTP transport adapter. Refer to [Using the HTTP transport adapter](#using-the-http-transport-adapter) for how to set it upon creating the SDK client.

4. Set as default (optional).

You can optionally set your custom HTTP transport adapter as the default to avoid explicitly setting the option every time you create a new SDK client.

The first step is to set the required CMake compile option that would configure the build for using a custom HTTP transport adapter. Refer to the [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#cmake-build-options) to find out the required option for this.

The second step is to implement the method shown below in the global unnamed namespace.

```cpp
/*
* Setting an HTTP transport adapter as default.
*/

// From one of the cpp files in your application:
std::shared_ptr<Azure::Core::Http::HttpTransport> ::AzureSdkGetCustomHttpTransport() {
  // Create and return a shared ptr for your custom HTTP transport adapter.
  return std::make_shared<CustomHttpTransportAdapter>();
}
```