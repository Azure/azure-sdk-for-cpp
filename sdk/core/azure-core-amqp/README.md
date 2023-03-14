# Azure SDK AMQP Library for C++

Azure::Core::_internal::AMQP (`azure-core-amqp`) provides an implementation
to enable developers to create Azure SDKs which consume the AMQP protocol.

## Getting started

### Include the package

The easiest way to acquire the OpenTelemetry library is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Core OpenTelemetry package via vcpkg:

```cmd
> vcpkg install azure-core-amqp-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-core-amqp-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-core-amqp)
```

## Key concepts



## Examples


## Next steps

You can build and run the tests locally by executing `azure-core-uamqp-test`. Explore the `test` folder to see advanced usage and behavior of the public classes.

## Troubleshooting

If you run into issues while using this library, please feel free to [file an issue](https://github.com/Azure/azure-sdk-for-cpp/issues/new).

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