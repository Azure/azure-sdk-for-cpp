# _Azure Identity client library for C++
The Azure Identity library provides Azure Active Directory token authentication support across the Azure SDK. It provides a set of `TokenCredential` implementations which can be used to construct Azure SDK clients which support AAD token authentication.
This library follows the [Azure SDK Design Guidelines for C++][azure_sdk_cpp_development_guidelines].

  [Source code][source] | [API reference documentation][doxygen] | [Azure Active Directory documentation][aad_doc]

## _Getting started
### _Include the package

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

## _Key concepts
### _Credentials

A credential is a class which contains or can obtain the data needed for a service client to authenticate requests. Service clients across Azure SDK accept credentials when they are constructed, and service clients use those credentials to authenticate requests to the service.

The Azure Identity library focuses on OAuth authentication with Azure Active directory, and it offers a variety of credential classes capable of acquiring an AAD token to authenticate service requests. All of the credential classes in this library are implementations of the `TokenCredential` abstract class in [azure-core][azure_core_library], and any of them can be used by to construct service clients capable of authenticating with a `TokenCredential`.

### _Authenticating Service Principals

<table border="1" width="100%">
  <thead>
    <tr>
      <th>credential class</th>
      <th>usage</th>
      <th>configuration</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>ClientSecretCredential</code></td>
      <td>authenticates a service principal using a secret</td>
      <td><a href="https://docs.microsoft.com/azure/active-directory/develop/app-objects-and-service-principals">Service principal authentication</a></td>
    </tr>
    <tr>
      <td><code>ClientCertificateCredential</code></td>
      <td>authenticates a service principal using a certificate</td>
      <td><a href="https://docs.microsoft.com/azure/active-directory/develop/app-objects-and-service-principals">Service principal authentication</a></td>
    </tr>
  </tbody>
</table>

## _Environment Variables
`EnvironmentCredential` can be configured with environment variables. Each type of authentication requires values for specific variables:

#### _Service principal with secret
<table border="1" width="100%">
  <thead>
    <tr>
      <th>variable name</th>
      <th>value</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>AZURE_CLIENT_ID</code></td>
      <td>id of an Azure Active Directory application</td>
    </tr>
    <tr>
      <td><code>AZURE_TENANT_ID</code></td>
      <td>id of the application's Azure Active Directory tenant</td>
    </tr>
    <tr>
      <td><code>AZURE_CLIENT_SECRET</code></td>
      <td>one of the application's client secrets</td>
    </tr>
  </tbody>
</table>

#### _Service principal with certificate
<table border="1" width="100%">
  <thead>
    <tr>
      <th>variable name</th>
      <th>value</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>AZURE_CLIENT_ID</code></td>
      <td>id of an Azure Active Directory application</td>
    </tr>
    <tr>
      <td><code>AZURE_TENANT_ID</code></td>
      <td>id of the application's Azure Active Directory tenant</td>
    </tr>
    <tr>
      <td><code>AZURE_CLIENT_CERTIFICATE_PATH</code></td>
      <td>path to a PEM-encoded certificate file including private key (without password protection)</td>
    </tr>
  </tbody>
</table>

Configuration is attempted in the above order. For example, if values for a client secret and certificate are both present, the client secret will be used.

## _Managed Identity Support
The [Managed identity authentication](https://docs.microsoft.com/azure/active-directory/managed-identities-azure-resources/overview) is supported via the `ManagedIdentityCredential` for the following Azure Services:
* [Azure Virtual Machines](https://docs.microsoft.com/azure/active-directory/managed-identities-azure-resources/how-to-use-vm-token)
* [Azure Cloud Shell](https://docs.microsoft.com/azure/cloud-shell/msi-authorization)
* [Azure Arc](https://docs.microsoft.com/azure/azure-arc/servers/managed-identity-authentication)

## _Chained Token Credential
`ChainedTokenCredential` allows users to customize the credentials considered when authenticating.

An example below demonstrates using `ChainedTokenCredential` which will attempt to authenticate using `EnvironmentCredential`, and fall back to authenticate using `ManagedIdentityCredential`.
```cpp
// Authenticate using environment credential if it is available; otherwise use the managed identity credential to authenticate.
auto chainedTokenCredential = std::make_shared<Azure::Identity::ChainedTokenCredential>(
    Azure::Identity::ChainedTokenCredential::Sources{
        std::make_shared<Azure::Identity::EnvironmentCredential>(),
        std::make_shared<Azure::Identity::ManagedIdentityCredential>()});

Azure::Service::Client azureServiceClient("serviceUrl", chainedTokenCredential);
```

## _Troubleshooting
Credentials raise exceptions either when they fail to authenticate or cannot execute authentication.
When credentials fail to authenticate, the `AuthenticationException` is thrown and it has the `what()` functions returning the description why authentication failed.

## _Contributing
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

### _Additional Helpful Links for Contributors
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### _Community-->
### _Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### _License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sdk_cpp_development_guidelines]: https://azure.github.io/azure-sdk/cpp_introduction.html
[source]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity
[aad_doc]: https://docs.microsoft.com/azure/active-directory/
[azure_core_library]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core
[doxygen]: https://azure.github.io/azure-sdk-for-cpp/
