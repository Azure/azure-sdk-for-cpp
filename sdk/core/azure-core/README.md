# Azure SDK Core Library for C++

Azure::Core (`azure-core`) provides shared primitives, abstractions, and helpers for modern Azure SDK client libraries written in the C++. These libraries follow the [Azure SDK Design Guidelines for C++][azure_sdk_cpp_development_guidelines].

The library allows client libraries to expose common functionality in a consistent fashion. Once you learn how to use these APIs in one client library, you will know how to use them in other client libraries.

## Getting started

Typically, you will not need to download `azure-core`; it will be downloaded for you as a dependency of the client libraries. In case you want to download it explicitly (to implement your own client library, for example), you can find the source in here, or use vcpkg to install the package `azure-core-cpp`.

### Include the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Core package via vcpkg:

```cmd
> vcpkg install azure-core-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-core-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-core)
```

## Key concepts

The main shared concepts of `Azure::Core` include:

- Handling streaming data and input/output (I/O) via `BodyStream` along with its derived types.
- Accessing HTTP response details for the returned model of any SDK client operation, via `Response<T>`.
- Polling long-running operations (LROs), via `Operation<T>`.
- Exceptions for reporting errors from service requests in a consistent fashion via the base exception type `RequestFailedException`.
- Abstractions for Azure SDK credentials (`TokenCredential`).
- Replaceable HTTP transport layer to send requests and receive responses over the network.
- HTTP pipeline and HTTP policies such as retry and logging, which are configurable via service client specific options.

### Long Running Operations

Some operations take a long time to complete and require polling for their status. Methods starting long-running operations return `Operation<T>` types.

You can intermittently poll whether the operation has finished by using the `Poll()` method on the returned `Operation<T>` and track progress of the operation using `Value()`. Alternatively, if you just want to wait until the operation completes, you can use `PollUntilDone()`.

```{.cpp}
SomeServiceClient client;

auto operation = *client.StartSomeLongRunningOperation();

while (!operation.IsDone())
{
  std::unique_ptr<Http::RawResponse> response = operation.Poll();

  auto partialResult = operation.Value();

  // Your per-polling custom logic goes here, such as logging progress.

  // You can also try to abort the operation if it doesn't complete in time.

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

auto finalResult = operation.Value();

```

### HTTP Transport adapter

Out of the box, the Azure SDK for C++ supports the libcurl and WinHTTP libraries as HTTP stacks for communicating with Azure services over the network. The SDK also provides a mechanism for `customer-implemented` _HTTP transport adapter_. [You can learn more about the transport adapter in this doc](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/HttpTransportAdapter.md#http-transport-adapter).

## Troubleshooting

Three main ways of troubleshooting failures are:

- Inspecting exceptions
- Enabling logging (see [SDK Log Messages](#sdk-log-messages))

### SDK Log Messages

The simplest way to enable logs is to set `AZURE_LOG_LEVEL` environment variable to one of the values:
|`AZURE_LOG_LEVEL`|`Azure::Core::Diagnostics::Logger::Level`|Log message level
|-|-|-
|`4`, or `error`, or `err`|`Error`|Logging level for failures that the application is unlikely to recover from.
|`3`, or `warning`, or `warn`|`Warning`|Logging level when a function fails to perform its intended task.
|`2`, or `informational`, or `information`, or `info`|`Informational`|Logging level when a function operates normally.
|`1`, or `verbose`, or `debug`|`Verbose`|Logging level for detailed troubleshooting scenarios.

Then, log messages will be printed to console (`stderr`).
Note that `stderr` messages can be redirected into a log file like this:

On Windows:
```cmd
myprogram.exe 2> log.txt
```

On Linux or macOS:
```sh
./myprogram 2> log.txt
```

In addition, log messages can be programmatically processed by providing a callback function, which can save them to a file, or display them in a desired custom way.
```cpp
#include <azure/core/diagnostics/logger.hpp>

int main()
{
    using namespace Azure::Core::Diagnostics;

    // See above for the level descriptions.
    Logger::SetLevel(Logger::Level::Verbose);

    // SetListener accepts std::function<>, which can be either lambda or a function pointer.
    Logger::SetListener([&](auto lvl, auto msg){ /* handle Logger::Level lvl and std::string msg */ });
}
```

Note, the listener callback is executed on the same thread as the operation that triggered the log message. 
 It is recommended implementation due the minimal amount of log message processing on the callback thread.
Where message processing is required, consider implementing in a way that the callback pushes the message string into a thread-safe queue, so that another thread would pick the messages from that queue and handle them.

## Next steps

Explore and install available Azure SDK libraries.

## Contributing

For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions. Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors

Many people all over the world have helped make this project better. You'll want to check out:

- [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
- [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
- [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
- Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->

### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sdk_cpp_development_guidelines]: https://azure.github.io/azure-sdk/cpp_introduction.html
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash