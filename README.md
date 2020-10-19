# Azure SDK for C++

[![Build Status](https://dev.azure.com/azure-sdk/public/_apis/build/status/cpp/cpp%20-%20client%20-%20ci?branchName=master)](https://dev.azure.com/azure-sdk/public/_build/latest?definitionId=1611&branchName=master)

This repository is for active development of the Azure SDK for C++. For consumers of the SDK we recommend visiting our [public developer docs](https://azure.github.io/azure-sdk-for-cpp) or our versioned [developer docs](https://azure.github.io/azure-sdk-for-cpp).

## Getting started

To get started with a library, see the **README.md** file located in the library's project folder. You can find these library folders grouped by service in the `/sdk` directory.

For tutorials, samples, quick starts, and other documentation, go to [Azure for C++ Developers](https://azure.github.io/azure-sdk-for-cpp).

## Packages available
Each service might have a number of libraries available from each of the following categories:
* [Client - New Releases](#client-new-releases)
* [Client - Previous Versions](#client-previous-versions)

### Client: New Releases

New wave of packages that we are announcing as **GA** and several that are currently releasing in **beta**. These libraries follow the [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html) and share a number of core features such as HTTP retries, logging, transport protocols, authentication protocols, etc., so that once you learn how to use these features in one client library, you will know how to use them in other client libraries. You can learn about these shared features at [Azure::Core](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/core/azure-core/README.md).

These new client libraries can be identified by the naming used for their folder, package, and namespace. Each will start with `azure`, followed by the service category, and then the name of the service. For example `azure-storage-blobs`.

For a complete list of available packages, please see the [latest available packages](https://azure.github.io/azure-sdk/releases/latest/#c) page.

> NOTE: If you need to ensure your code is ready for production we strongly recommend using one of the stable, non-beta libraries.

### Client: Previous Versions

Last stable versions of packages that are production-ready. These libraries provide similar functionalities to the beta packages, as they allow you to use and consume existing resources and interact with them, for example: upload a storage blob. They might not implement the [guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html) or have the same feature set. They do however offer wider coverage of services.

## Need help

- For reference documentation visit the [Azure SDK for C++ documentation](https://azure.github.io/azure-sdk-for-cpp).
- For tutorials, samples, quick starts and other documentation, visit [Azure for C++ Developers](https://docs.microsoft.com/azure/).
- File an issue via [Github Issues](https://github.com/Azure/azure-sdk-for-cpp/issues/new/choose).

## Navigating the repository

### Master branch

The master branch has the most recent code with new features and bug fixes. It does **not** represent latest released **beta** or **GA** SDK.

### Release branches (Release tagging)

For each package we release there will be a unique git tag created that contains the name and the version of the package to mark the commit of the code that produced the package. This tag will be used for servicing via hotfix branches as well as debugging the code for a particular beta or stable release version.
Format of the release tags are `<package-name>_<package-version>`. For more information please see [our branching strategy](https://github.com/Azure/azure-sdk/blob/master/docs/policies/repobranching.md#release-tagging).

## Contributing

For details on contributing to this repository, see the [contributing guide](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md).

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, view [Microsoft's CLA](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors

Many people all over the world have helped make this project better.  You'll want to check out:

- [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
- [How to build and test your change](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#developer-guide)
- [How you can make a change happen!](https://github.com/Azure/azure-sdk-for-cpp/blob/master/CONTRIBUTING.md#pull-requests)
- Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/master/LICENSE) license.

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2FREADME.png)
