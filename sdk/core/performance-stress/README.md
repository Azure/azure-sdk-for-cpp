# Azure SDK performance stress C++

Azure performance stress for C++ (`azure-performance-stress`) provides shared primitives, abstractions, and helpers for running performance tests for an Azure SDK clients for C++. It represent the C++ version of the [.NET original version](https://github.com/Azure/azure-sdk-for-net/tree/master/common/Perf).

## Getting started

See the [pre-requisites](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pre-requisites) for building before going to the next step.

### Build

Use the build option `BUILD_PERFORMANCE_TESTS` to build the test framework and all the performance tests. See the next example.

```bash
#
# Building the Performance framework and tests.
#
git clone https://github.com/Azure/azure-sdk-for-cpp.git
cd azure-sdk-for-cpp
mkdir build
cd build
cmake -DBUILD_TESTING=ON -DBUILD_PERFORMANCE_TESTS=ON ..
cmake --build .
```

### Run

Once building is completed, a performance test application will be created inside the the test folder from each service SDK. For instance, the Azure core performance test application would be inside: `build/sdk/core/azure-core/test/performance`. See next example for running the performance framework 


## Key concepts

TODO

## Examples

TODO

## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors  
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/core/performance-stress/LICENSE) license.

<!-- LINKS -->
[azure_sdk_for_c_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md
[azure_sdk_for_c_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#developer-guide
[azure_sdk_for_c_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pull-requests
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash
