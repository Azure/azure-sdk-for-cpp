# Azure Identity client library for C++
The Azure Identity library provides Azure Active Directory token authentication support across the Azure SDK. It provides a set of `TokenCredential` implementations which can be used to construct Azure SDK clients which support AAD token authentication.
This library follows the [Azure SDK Design Guidelines for C++][azure_sdk_cpp_development_guidelines].

  [Source code][source] | [API reference documentation][doxygen] | [Azure Active Directory documentation][aad_doc]

## Getting started
### Include the package

The easiest way to acquire the C++ SDK is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Identity package via vcpkg:

```cmd
> vcpkg install azure-identity-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-identity-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-identity)
```

### Prerequisites

* An [Azure subscription][azure_sub].
* The [Azure CLI][azure_cli] can also be useful for authenticating in a development environment, creating accounts, and managing account roles.

### Authenticate the client

When debugging and executing code locally it is typical for a developer to use their own account for authenticating calls to Azure services. There are several developer tools which can be used to perform this authentication in your development environment.

#### Authenticate via the Azure CLI

Developers can use the [Azure CLI][azure_cli] to authenticate. Applications using the `DefaultAzureCredential` or the `AzureCliCredential` can then use this account to authenticate calls in their application when running locally.

To authenticate with the [Azure CLI][azure_cli], users can run the command `az login`. For users running on a system with a default web browser, the Azure CLI will launch the browser to authenticate the user.

## Key concepts
### Credentials

A credential is a class which contains or can obtain the data needed for a service client to authenticate requests. Service clients across Azure SDK accept credentials when they are constructed, and service clients use those credentials to authenticate requests to the service.

The Azure Identity library focuses on OAuth authentication with Azure Active directory, and it offers a variety of credential classes capable of acquiring an AAD token to authenticate service requests. All of the credential classes in this library are implementations of the `TokenCredential` abstract class in [azure-core][azure_core_library], and any of them can be used by to construct service clients capable of authenticating with a `TokenCredential`.

See [Credential Classes](#credential-classes) for a complete listing of available credential types.

### DefaultAzureCredential

`DefaultAzureCredential` combines credentials commonly used to authenticate when deployed, with credentials used to authenticate in a development environment.

> Note: `DefaultAzureCredential` is intended to simplify getting started with the SDK by handling common scenarios with reasonable default behaviors. It is not recommended to use it in production. Developers who want more control or whose scenario isn't served by the default settings should use other credential types.

The `DefaultAzureCredential` attempts to authenticate via the following mechanisms, in this order, stopping when one succeeds:

1. **Environment** - The `DefaultAzureCredential` will read account information specified via [environment variables](#environment-variables) and use it to authenticate.
1. **Azure CLI** - If the developer has authenticated an account via the Azure CLI `az login` command, the `DefaultAzureCredential` will authenticate with that account.
1. **Managed Identity** - If the application is deployed to an Azure host with Managed Identity enabled, the `DefaultAzureCredential` will authenticate with that account.

`DefaultAzureCredential` uses [`ChainedTokenCredential`](#chained-token-credential) that consists of a chain of `EnvironmentCredential`, `AzureCliCredential`, and `ManagedIdentityCredential`. Implementation, including the order in which credentials are applied is documented, but it may change from release to release.

`DefaultAzureCredential` intends to provide a credential that "just works out of the box and without requiring any information", if only the environment is set up sufficiently for the credential to work.
Therefore, it could be simple to use, but since it uses a chain of credentials, it could be a bit complicated to diagnose if the environment setup is not sufficient.
TO help with this, `DefaultAzureCredential` code paths are instrumented with [log messages](#troubleshooting).

## Examples

See the [code samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity/samples).

## Chained Token Credential

`ChainedTokenCredential` allows users to set up custom authentication flow consisting of multiple credentials.

An example below demonstrates using `ChainedTokenCredential` which will attempt to authenticate using `EnvironmentCredential`, and fall back to authenticate using `ManagedIdentityCredential`.
```cpp
// A configuration demonstrated below would authenticate using EnvironmentCredential if it is
// available, and if it is not available, would fall back to use AzureCliCredential, and then to
// ManagedIdentityCredential.
auto chainedTokenCredential = std::make_shared<Azure::Identity::ChainedTokenCredential>(
    Azure::Identity::ChainedTokenCredential::Sources{
        std::make_shared<Azure::Identity::EnvironmentCredential>(),
        std::make_shared<Azure::Identity::AzureCliCredential>(),
        std::make_shared<Azure::Identity::ManagedIdentityCredential>()});

Azure::Service::Client azureServiceClient("serviceUrl", chainedTokenCredential);
```

## Managed Identity Support

The [Managed identity authentication](https://docs.microsoft.com/azure/active-directory/managed-identities-azure-resources/overview) is supported via the `ManagedIdentityCredential` for the following Azure Services:
* [Azure App Service and Azure Functions](https://docs.microsoft.com/azure/app-service/overview-managed-identity)
* [Azure Cloud Shell](https://docs.microsoft.com/azure/cloud-shell/msi-authorization)
* [Azure Arc](https://docs.microsoft.com/azure/azure-arc/servers/managed-identity-authentication)
* [Azure Virtual Machines](https://docs.microsoft.com/azure/active-directory/managed-identities-azure-resources/how-to-use-vm-token)

## Environment Variables

`DefaultAzureCredential` and `EnvironmentCredential` can be configured with environment variables. Each type of authentication requires values for specific variables:

#### Service principal with secret
|Variable name|Value
|-|-
|`AZURE_TENANT_ID`|ID of the application's Azure AD tenant
|`AZURE_CLIENT_ID`|ID of an Azure AD application
|`AZURE_CLIENT_SECRET`|one of the application's client secrets
|`AZURE_AUTHORITY_HOST`|(optional) [authentication authority URL](https://docs.microsoft.com/azure/active-directory/develop/authentication-national-cloud)

#### Service principal with certificate
|variable name|Value
|-|-
|`AZURE_CLIENT_ID`|ID of an Azure AD application
|`AZURE_TENANT_ID`|ID of the application's Azure AD tenant
|`AZURE_CLIENT_CERTIFICATE_PATH`|path to a PFX or PEM-encoded certificate file including private key
|`AZURE_AUTHORITY_HOST`|(optional) [authentication authority URL](https://docs.microsoft.com/azure/active-directory/develop/authentication-national-cloud)

Configuration is attempted in the above order. For example, if values for a client secret and certificate are both present, the client secret will be used.

## Credential classes

### Authenticate Azure-hosted applications
|Credential | Usage
|-|-
|`DefaultAzureCredential`|Provides a simplified authentication experience to quickly start developing applications run in Azure.
|`ChainedTokenCredential`|Allows users to define custom authentication flows composing multiple credentials.
|`ManagedIdentityCredential`|Authenticates the managed identity of an Azure resource.
|`EnvironmentCredential`|Authenticates a service principal or user via credential information specified in environment variables.

### Authenticate service principals
|Credential | Usage
|-|-
|`ClientSecretCredential`|Authenticates a service principal [using a secret](https://learn.microsoft.com/azure/active-directory/develop/app-objects-and-service-principals).
|`ClientCertificateCredential`|Authenticates a service principal [using a certificate](https://learn.microsoft.com/azure/active-directory/develop/app-objects-and-service-principals).

### Authenticate via development tools
|Credential | Usage
|-|-
|`AzureCliCredential`|Authenticates in a development environment [with the Azure CLI](https://learn.microsoft.com/cli/azure/authenticate-azure-cli).

## Troubleshooting

1. Azure Identity [SDK log messages](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#sdk-log-messages) contain disgnostic information, and start with "`Identity: `".
1. Credentials raise exceptions either when they fail to authenticate or cannot execute authentication. When a credential fails to authenticate, an `AuthenticationException` is thrown. The exception has the `what()` function that provides more information about the failure.

## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit the [Contributor License Agreement](https://cla.microsoft.com).

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

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azure_cli]: https://learn.microsoft.com/cli/azure
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sdk_cpp_development_guidelines]: https://azure.github.io/azure-sdk/cpp_introduction.html
[source]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity
[aad_doc]: https://docs.microsoft.com/azure/active-directory/
[azure_core_library]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core
[doxygen]: https://azure.github.io/azure-sdk-for-cpp/
