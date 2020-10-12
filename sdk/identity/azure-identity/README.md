# Azure SDK Identity Library for C++

Azure::Identity (`azure-identity`) provides shared primitives, abstractions, and helpers for modern Azure SDK client libraries written in the C++. These libraries follow the [Azure SDK Design Guidelines for C++][azure_sdk_cpp_development_guidelines].

The library contains commonly (but not universally) used credential types.

## Getting started

Typically, you will not need to download `azure-identity`; it will be downloaded for you as a dependency of the client libraries.  In case you want to download it explicitly (to implement your own client library, for example), you can find the source
in here.

## Key concepts

Azure::Identity credentials:
- Client Secret Credential (`ClientSecretCredential`)
- Environment Credential (`EnvironmentCredential`)

## Troubleshooting

Three main ways of troubleshooting failures are:
- Inspecting exceptions
- Enabling logging (`Available in future release`)
- Distributed tracing (`Available in future release`)

## Next steps

Explore and install available Azure SDK libraries.

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

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/core/azure-core/LICENSE) license.

<!-- LINKS -->
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pull-requests
[azure_sdk_cpp_development_guidelines]: https://azure.github.io/azure-sdk/cpp_introduction.html
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash
