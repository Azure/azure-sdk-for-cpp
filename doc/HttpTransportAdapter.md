# HTTP Transport Adapter

The Azure SDK for C++ creates and sends HTTP requests using an HTTP Transport Adapter. This adapter provides a general abstraction for an HTTP stack (like libcurl, WinHTTP, etc) by defining the interface it needs to call that anyone can implement to customize which HTTP stack gets used.

## Default HTTP Transport Adapter
The Azure SDK for C++ provides two HTTP transport adapters as part of the azure-core package:
- The libcurl transport adapter
- WinHttp transport adapter

Azure SDK for C++ users can create and use their own transport adapter as well [by following the guidance below](#Building-a-custom-HTTP-Transport-Adapter).

For the simplest case, when no specific configuration is set, the default transport adapter will be selected based on the OS. The WinHTTP transport adapter will be used for Windows-based systems (making libcurl an optional dependency). The libcurl transport adapter will be selected for any non-windows-based system.

## Building HTTP transport adapter

The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required for each HTTP transport adapter.

Multiple HTTP transport adapters can be built as part of the project. This is to support scenarios where you want to send HTTP request with the libcurl transport adapter for some cases and the WinHTTP transport adapter for others.
Note WinHTTP transport adapter is only supported on Windows.

Another example is if you want to create your own HTTP transport adapter for some other C++ HTTP stack. In this case, you can build your own HTTP transport adapter and test it by comparing it to the behavior of the libcurl HTTP transport adapter.

## Using the HTTP transport adapter
The HTTP transport adapter is set up during service SDK client initialization, for example, when creating an Azure Storage SDK client. The HTTP transport adapter can be specified at client initialization via the client options argument. See the following code snippet as an example:
```cpp
  /*
  * Using the HTTP transport adapter.
  */

  // When no options are provided, the HTTP transport adapter is set to the default one based on the compiler options passed in to CMake.
  auto defaultStorageClient = BlobServiceClient(url, credential);

  // Use client options to set an HTTP transport adapter, which needs to be a shared_ptr.
  BlobClientOptions options;
  options.TransportPolicyOptions.Transport = std::make_shared<Azure::Core::Http::CurlTransport>();
  auto storageClient = BlobServiceClient(url, credential, options);
```

### Re-use the HTTP transport adapter 
Note that the HTTP transport adapter is a `shared_ptr`. This is because you can re-use the same HTTP transport adapter for multiple clients. There are two ways of doing this. The first one is by using the same HTTP transport adapter when creating another client. The second one is when you get a specific child client from an already created parent client directly, in which case the child client inherits the same configuration. See the next example:

```cpp
  /* 
  *  Option 1. Init a new client with the same HTTP transport adapter.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions optionsA;
  optionsA.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClientA = BlobServiceClient(url, credential, optionsA);
  // The second client.
  BlobClientOptions optionsB;
  optionsB.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClientB = BlobServiceClient(url, credential, optionsB);

  /* 
  *  Option 2. Create new client from a parent client.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions options;
  options.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClient = BlobServiceClient(url, credential, options);
  // Create specific child client from the parent service client.
  auto blobContainerClient = storageClient.GetBlobContainerClient(containerName)
```

Note that the second option may not supported for some Azure SDK clients if it is not applicable depending on the service behavior (for example, the Key Vault client).

Each client holds an internal copy of the options passed in the arguments. This means that the options object used to create the client can be disposed. However, since the HTTP transport adapter is a shared pointer, it won't be disposed until all clients using it are disposed first.

## HTTP Transport Adapter Options
The libcurl and WinHTTP transport adapters can also be initialized with specific options that expose control of features from the HTTP stack underneath it. For example, you can set a proxy or override the default certificate authority (CA) path for libcurl. See the following example:
```cpp
  /*
  * Customize libcurl HTTP transport adapter.
  */

  // Create the HTTP transport adapter options and set them when initializing the adapter.
  Azure::Core::Http::CurlTransportOptions curlTransportOptions;
  curlTransportOptions.Proxy = "https://my.proxy.com";
  // Create HTTP transport adapter with options.
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlTransportOptions);

  BlobClientOptions options;
  options.TransportPolicyOptions.Transport = curlTransportAdapter; 
  auto storageClient = BlobServiceClient(url, credential, options);
```

The HTTP transport adapter options are specific for each adapter and are optional. This means that if you are creating your own HTTP transport adapter, you don't need to create options for it if you don't want to.


## Building a custom HTTP Transport Adapter
The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required to be set when building your own HTTP transport adapter.

Follow next steps to implement your own HTTP Transport Adapter.

1. Implement the interface. The HTTP transport adapter interface is define in `Azure::Core::HTTP::HttpTransport` from `inc/azure/core/http/transport.hpp`.

```cpp
/*
* Implementing the HTTP transport adapter.
*/

#include "azure/core/http/transport.hpp"

// Derive from HttpTransport
class CurlTransport : public Azure::Core::Http::HttpTransport {
  public:
    // Override the Send method
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) override;
};
```

The implementation needs to take care of using the members from the `Request` argument which contains all the HTTP request information to write the HTTP request using some C++ HTTP stack. Then it needs to create the `RawResponse` from the response and return it. The `context` argument can be optionally used to check if the request is cancelled before performing I/O operations (like writing to the network). 
