# Libcurl Transport Adapter

The Azure SDK for C++ provides an HTTP transport adapter implementation using the well known libcurl library. There are some general guidelines that you should know if you decide to use this implementation.

## Global init and clean up

Libcurl provides the functions [curl_global_init](https://curl.se/libcurl/c/curl_global_init.html) and [curl_global_cleanup](https://curl.se/libcurl/c/curl_global_cleanup.html) which are expected to be called at the start and before exiting the application. However, for a modular part of the application, like a library, it is expected that each library using libcurl will call the `curl_global_init` and `curl_global_cleanup`. From [libcurl global constants documentation](https://curl.se/libcurl/c/libcurl.html):

> Note that if multiple modules in the program use libcurl, they all will separately call the libcurl functions, and that's OK because only the first curl_global_init and the last curl_global_cleanup in a program change anything. (libcurl uses a reference count in static memory).

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

  // Use the Azure SDK clients.

  curl_global_cleanup();
  return 0;
}

```

If the application is not using libcurl at all, only the Azure SDK library will call `curl_global_init` and `curl_global_cleanup`. If the application is calling the global functions and using libcurl, then the calls to these functions from the Azure SDK library won't change anything. So, as an application or library owner, you only need to worry about calling the global libcurl functions if you will be using libcurl directly.
