# Libcurl Transport Adapter

The Azure SDK for C++ provides an http transport adapter implementation using the well known libcurl library. There are some general guidelines that you should know if you decide to use this implementation.

## Global init and clean up

Libcurl provides the functions `curl_global_init` and `curl_global_cleanup` which are expected to be called at the start and before exiting the application. Similarly, the Azure SDK for C++ exposes the static function `CleanUp` in the `Azure::Core::Http::CurlTransport`. This function is expected to be called before calling `curl_global_cleanup`.

Consider the next example:

```cpp
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file Demonstrate how to use libcurl and the Azure SDK for C++. 
 *
 */

#include <curl/curl.h>
#include <azure/storage/blob/blobs.hpp>

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);

  // Use the Azure SDK clients or any other libcurl usage in your application.
  
  // Clean any active handle from the Azure SDK.
  Azure::Core::Http::CurlTransport::CleanUp();

  curl_global_cleanup();
  return 0;
}

```

>Note: Not calling `Azure::Core::Http::CurlTransport::CleanUp()` could lead to a segmentation fault on Windows, as mentioned in this [issue](https://github.com/Azure/azure-sdk-for-cpp/issues/1499).
