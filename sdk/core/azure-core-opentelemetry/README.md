# Azure SDK Core Tracing Library for C++

Azure::Core::OpenTelemetry (`azure-core-opentelemetry`) provides an implementation
to enable customers to implement tracing in the Azure SDK for C++ libraries.

## Getting started

### Include the package

The easiest way to acquire the OpenTelemetry library is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Core OpenTelemetry package via vcpkg:

```cmd
> vcpkg install azure-core-opentelemetry-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-core-opentelemetry-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-core-opentelemetry)
```

## Key concepts

The `azure-core-opentelemetry` package supports enabling tracing for Azure SDK packages, using an [OpenTelemetry](https://opentelemetry.io/) `Tracer`.

By default, all libraries log with a `NoOpTracer` that takes no action. To enable tracing, you will need to set a global tracer provider following the instructions in the [OpenTelemetry getting started guide](https://opentelemetry-cpp.readthedocs.io/en/latest/api/GettingStarted.html) or the [Enabling Tracing using OpenTelemetry example](#enabling-tracing-using-opentelemetry) below.

### Span Propagation

Core Tracing supports both automatic and manual span propagation. Automatic propagation is handled using OpenTelemetry's API and will work well in most scenarios when run in `Node.js`.

For customers who require manual propagation, or to provide context propagation in the browser, all client library operations accept an optional options collection where a tracingContext can be passed in and used as the currently active context. Please see the [Manual Span Propagation example](#manual-span-propagation-using-opentelemetry) below for more details.

### OpenTelemetry Compatibility

Most Azure SDKs use [OpenTelemetry](https://opentelemetry.io/) to support tracing. Specifically, we depend on the [azure-core-opentelemetry](https://github.com/open-telemetry/opentelemetry-cpp/blob/main/docs/building-with-vcpkg.md) VCPKG package.


## Examples

### Enabling tracing using OpenTelemetry

```cpp
<TBD>
```

### Manual Span Propagation using OpenTelemetry

```cpp
<TBD>
```

## Next steps

You can build and run the tests locally by executing `azure-core-opentelemetry-test`. Explore the `test` folder to see advanced usage and behavior of the public classes.

## Troubleshooting

If you run into issues while using this library, please feel free to [file an issue](https://github.com/Azure/azure-sdk-for-cpp/issues/new).

### OpenTelemetry Compatibility Errors


> Ideally you'd want to use OpenTelemetry 1.3.0 or higher.

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

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2Fsdk%2Fcore%2Fcore-opentelemetry%2FREADME.png)