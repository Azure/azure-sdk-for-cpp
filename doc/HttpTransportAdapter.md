# HTTP Transport Adapter
The Azure SDK for C++ creates and sends HTTP request using an HTTP Transport Adapter. This adapter provides a general abstraction for an HTTP stack (like libcurl, WinHTTP, etc). It defines the interface that the Azure SDK for C++ will call to put the HTTP request into the wire.

## Default HTTP Transport Adapter
The Azure SDK for C++ provides two HTTP transport adapters as part of the azure-core package:
- The libcurl transport adapter
- WinHttp transport adapter

Azure SDK for C++ users can create and use their own transport adapter as well [see below](#Building-a-custom-HTTP-Transport-Adapter).

For the simplest case, when there was not any specific configuration set for the Azure SDK for C++, the default transport adapter will be selected based on the OS. The WinHTTP transport adapter will be used for Windows-based systems (making libcurl an optional dependency). The libcurl transport adapter will be selected for any non-windows-based system.

## Building HTTP transport adapter
The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required for each HTTP transport adapter.

Multiple HTTP transport adapters can be built as part of the project. For example, if you want to send HTTP request with the libcurl transport adapter for some cases and the WinHTTP transport adapter for another cases.
Note that combining WinHTTP transport adapter with another transport adapter is only possible when using Windows.

Another example is if you want to create your own HTTP transport adapter for some other C++ HTTP stack. In this case, you can build your own HTTP transport adapter and also the libcurl HTTP transport adapter and run tests to compare the correctness of both. 

## Using the HTTP transport adapter
The HTTP transport adapter is set up when a service SDK client is init. For example, when creating an Azure Storage SDK client. The HTTP transport adapter is part of the client options argument that can be used to init the client. See the next example.
```cpp
  /*
  * Using the HTTP transport adapter.
  */

  // When no options are provided, the HTTP transport adapter is set to the default one.
  auto defaultStorageClient = BlobServiceClient(url, credential);

  // Use client options to set an HTTP transport adapter.
  BlobClientOptions options;
  options.TransportPolicyOptions.Transport = std::make_shared<Azure::Core::Http::CurlTransport>();
  auto storageClient = BlobServiceClient(url, credential, options);
```

### Re-use the HTTP transport adapter 
Note that the HTTP transport adapter is a `shared_ptr`. This is because you can re-use the same HTTP transport adapter for multiple clients. There are two ways of doing this. The first one is by using the same HTTP transport adapter to init a second client. The second one is when the client can create a new client directly witch inherits the same configuration. See the next example:

```cpp
  /* 
  *  Option 1. Init a new client with the same HTTP transport adapter.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions optionsA;
  optionsA.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClientA = BlobServiceClient(url, credential, options);
  // The second client.
  BlobClientOptions optionsB;
  optionsB.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClientB = BlobServiceClient(url, credential, options);

  /* 
  *  Option 2. Create new client from a parent client.
  */
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
  BlobClientOptions optionsA;
  optionsA.TransportPolicyOptions.Transport = curlTransportAdapter;
  auto storageClientA = BlobServiceClient(url, credential, options);
  // Create clientB from clientA.
  auto clientB = storageClientA.GetBlobContainerClient(containerName)
```

Note that the second option could be not supported for some Azure SDK clients if it doesn't make sense for the service behavior (like Key Vault client).

The Azure SDK for C++ client `will hold an internal copy of the options` passed in the arguments. This means that the options object used to init the client can be disposed. However, since the HTTP transport adapter is a shared ptr, it won't be disposed until any client using it is disposed first.

## HTTP Transport Adapter Options
The libcurl and WinHTTP transport adapters can also be init with specif options that control features from the HTTP stack underneath it.
For example, you can set a PROXY or override the default CA path for libcurl. See next example.
```cpp
  /*
  * Customize libcurl http transport adapter.
  */

  // Create HTTP transport adapter options and set options.
  Azure::Core::Http::CurlTransportOptions curlTransportOption;
  curlTransportOption.Proxy = "https://my.proxy.com";
  // Create HTTP transport adapter with options.
  auto curlTransportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlTransportOption);

  BlobClientOptions options;
  options.TransportPolicyOptions.Transport = curlTransportAdapter; 
  auto storageClient = BlobServiceClient(url, credential, options);
```

The HTTP transport adapter are specific for each adapter and are optional. This means that if you are creating your own HTTP transport adapter, you don't need to create options for it if you don't want to.


## Building a custom HTTP Transport Adapter
The Azure SDK for C++ uses CMake options to define what HTTP transport adapters to build.

You can see [CMake options](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#cmake-build-options) to learn about all the supported options and what is the specific option required to be set when building your own HTTP transport adapter.

Follow next steps to implement your own HTTP Transport Adapter.

1. Implement interface. The HTTP transport adapter interface is define in `Azure::Core::HTTP::HttpTransport` from `inc/azure/core/http/transport.hpp`.

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

The implementation needs to take care of using the data from the argument `request` containing all the HTTP request information to write the HTTP request using some C++ HTTP stack. Then it needs to create the raw response from the response and return it. The `context` argument can be optionally used to check if the request is cancelled before performing I/O operations (like writing to the network). 
