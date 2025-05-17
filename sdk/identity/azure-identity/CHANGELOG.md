# Release History

## 1.12.0-beta.1 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

- [[#4952]](https://github.com/Azure/azure-sdk-for-cpp/issues/4952) Fixed `ManagedIdentityCredential` to fail fast if IMDS authentication is not available.
- [[#4669]](https://github.com/Azure/azure-sdk-for-cpp/issues/4669) Fixed the order of credentials in `DefaultAzureCredential`: `ManagedIdentityCredential` before `AzureCliCredential`.

### Other Changes

## 1.11.0 (2025-04-08)

### Features Added

- Added `Subscription` to `AzureCliCredentialOptions` which allows the caller to specify an Azure subscription that does not match the current Azure CLI subscription.
- [[#6321]](https://github.com/Azure/azure-sdk-for-cpp/issues/6321) Log Client ID used in `ManagedIdentityCredential`.

### Bugs Fixed

- [[#5235]](https://github.com/Azure/azure-sdk-for-cpp/issues/5235) Warnings in `azure/identity.hpp` cause strict builds to fail.

## 1.11.0-beta.1 (2025-03-11)

### Features Added

- Added `Subscription` to `AzureCliCredentialOptions` which allows the caller to specify an Azure subscription that does not match the current Azure CLI subscription.
- [[#6321]](https://github.com/Azure/azure-sdk-for-cpp/issues/6321) Log Client ID used in `ManagedIdentityCredential`.

### Bugs Fixed

- [[#5235]](https://github.com/Azure/azure-sdk-for-cpp/issues/5235) Warnings in `azure/identity.hpp` cause strict builds to fail.

## 1.10.1 (2024-11-08)

### Bugs Fixed

- Fix overflow issue in token cache.

### Other Changes

- [[#6086]](https://github.com/Azure/azure-sdk-for-cpp/pull/6086) Correct minimum version specification for the Azure Core dependency. (A community contribution, courtesy of _[jdblischak](https://github.com/jdblischak)_)

Thank you to our developer community members who helped to make Azure Identity better with their contributions to this release:

- John Blischak _([GitHub](https://github.com/jdblischak))_

## 1.10.0 (2024-10-08)

### Features Added

- Added support for providing an object ID or a resource ID to `ManagedIdentityCredential`.
- Added support for passing in the x509 certificate and its corresponding private key directly to `ClientCertificateCredential`, rather than reading from a pem file.
- Added support for sending an x5c parameter in `ClientCertificateCredential`.

### Breaking Changes

- Previously, if a clientId was specified for Cloud Shell managed identity, which is not supported, the clientId was passed into the request body. Now, an exception will be thrown if a clientId is specified for Cloud Shell managed identity.

### Bugs Fixed

- Fixed the request sent in `AzurePipelinesCredential` so it doesn't result in a redirect response when an invalid system access token is provided.

### Other Changes

- Allow certain response headers to be logged in `AzurePipelinesCredential` for diagnostics and include them in the exception message.
- In `ClientCertificateCredential`, add the x5c parameter of the JWT token as a JSON array rather than a JSON string.

## 1.10.0-beta.1 (2024-09-17)

### Features Added

- Added support for providing an object ID to `ManagedIdentityCredential`.
- Added support for passing in the x509 certificate and its corresponding private key directly to `ClientCertificateCredential`, rather than reading from a pem file.
- Added support for sending an x5c parameter in `ClientCertificateCredential`.

### Breaking Changes

- Previously, if a clientId was specified for Cloud Shell managed identity, which is not supported, the clientId was passed into the request body. Now, an exception will be thrown if a clientId is specified for Cloud Shell managed identity.

## 1.9.0 (2024-08-06)

### Features Added

- Added `AzurePipelinesCredential` for authenticating an Azure Pipelines service connection with workload identity federation.
- Added `ClientAssertionCredential` to enable applications to authenticate with custom client assertions.

## 1.9.0-beta.2 (2024-07-22)

### Features Added

- Added `ClientAssertionCredential` to enable applications to authenticate with custom client assertions.
- Added support for providing a Resource ID to `ManagedIdentityCredential`.
- Added support for customizing the IMDS endpoint within `ManagedIdentityCredential`.

## 1.9.0-beta.1 (2024-06-21)

### Features Added

- Added `AzurePipelinesCredential` for authenticating an Azure Pipelines service connection with workload identity federation.

## 1.8.0 (2024-06-11)

### Features Added

- [[#4474]](https://github.com/Azure/azure-sdk-for-cpp/issues/4474) Enable proactive renewal of Managed Identity tokens.
- [[#5116]](https://github.com/Azure/azure-sdk-for-cpp/issues/5116) `AzureCliCredential`: Added support for the new response field which represents token expiration timestamp as time zone agnostic value.

### Bugs Fixed

- Managed identity bug fixes.

## 1.7.0-beta.2 (2024-02-09)

### Features Added

- [[#4474]](https://github.com/Azure/azure-sdk-for-cpp/issues/4474) Enable proactive renewal of Managed Identity tokens.

## 1.7.0-beta.1 (2024-01-11)

### Features Added

- [[#5116]](https://github.com/Azure/azure-sdk-for-cpp/issues/5116) `AzureCliCredential`: Added support for the new response field which represents token expiration timestamp as time zone agnostic value.

### Bugs Fixed

- [[#5075]](https://github.com/Azure/azure-sdk-for-cpp/issues/5075) `AzureCliCredential` assumes token expiration time without local time zone adjustment.

### Other Changes

- [[#5141]](https://github.com/Azure/azure-sdk-for-cpp/issues/5141) Added error response details to the `AuthenticationException` thrown when the authority host returns error response.

## 1.6.0 (2023-11-10)

### Features Added

- Added `WorkloadIdentityCredential`.
- When one of the credentials within `DefaultAzureCredential` is successful, it gets re-used during all subsequent attempts to get the token.
- Updated `ClientSecretCredentialOptions` and `ClientCertificateCredentialOptions` to read the default value for the authority host option from the environment variable first.

### Breaking Changes

- Add `WorkloadIdentityCredential` to the `DefaultAzureCredential`.

### Bugs Fixed

- Do not throw an exception during `AzureCliCredential` construction, but rather delay it to the `GetToken()` call.
- Harden checks for the tenant ID.
- Disallow space character when validating tenant id and scopes as input for `AzureCliCredential`.
- Add authority host url validation to reject non-HTTPS schemes.
- [[#4084]](https://github.com/Azure/azure-sdk-for-cpp/issues/4084) Remove OpenSSL dependency on Windows. (A community contribution, courtesy of _[teo-tsirpanis](https://github.com/teo-tsirpanis)_)

### Other Changes

- Add default values to some `WorkloadIdentityCredentialOptions` fields such as authority host by reading them from the environment.
- Add logging to `WorkloadIdentityCredential` to help with debugging.
- Create separate lists of characters that are allowed within tenant ids and scopes in `AzureCliCredential`.

### Acknowledgments

Thank you to our developer community members who helped to make Azure Identity better with their contributions to this release:

- Theodore Tsirpanis _([GitHub](https://github.com/teo-tsirpanis))_

## 1.6.0-beta.3 (2023-10-12)

### Bugs Fixed

- Change the default value for the authority host option to be read from the environment variable first.
- Do not throw an exception during `AzureCliCredential` construction, but rather delay it to the `GetToken()` call.

## 1.6.0-beta.2 (2023-09-13)

### Features Added

- Add support for reading the tenant id, client id, and the token file path for `WorkloadIdentityCredential` from the environment variables.

### Breaking Changes

- Modify the order of the credentials used within the `DefaultAzureCredential` to be consistent with other languages.
- Add `WorkloadIdentityCredential` to the `DefaultAzureCredential`.

### Bugs Fixed

- [[#4084]](https://github.com/Azure/azure-sdk-for-cpp/issues/4084) Remove OpenSSL dependency on Windows. (A community contribution, courtesy of _[teo-tsirpanis](https://github.com/teo-tsirpanis)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Identity better with their contributions to this release:

- Theodore Tsirpanis _([GitHub](https://github.com/teo-tsirpanis))_

## 1.6.0-beta.1 (2023-08-11)

### Features Added

- Added support for `WorkloadIdentityCredential`.

## 1.5.1 (2023-07-06)

### Bugs Fixed

- [[#4723]](https://github.com/Azure/azure-sdk-for-cpp/issues/4723) Accept a wider variety of token responses.

## 1.5.0 (2023-05-04)

### Features Added

- Added support for challenge-based and multi-tenant authentication.
- Added `DefaultAzureCredential`.

### Bugs Fixed

- [[#4443]](https://github.com/Azure/azure-sdk-for-cpp/issues/4443) Fixed potentially high CPU usage on Windows.

### Other Changes

- Improved diagnostics to utilize `Azure::Core::Credentials::TokenCredential::GetCredentialName()`.
- Improved log messages.

## 1.5.0-beta.2 (2023-04-06)

### Features Added

- Added support for challenge-based and multi-tenant authentication.

### Bugs Fixed

- [[#4443]](https://github.com/Azure/azure-sdk-for-cpp/issues/4443) Fixed potentially high CPU usage on Windows.

### Other Changes

- Improved diagnostics to utilize `Azure::Core::Credentials::TokenCredential::GetCredentialName()`.

## 1.5.0-beta.1 (2023-03-07)

### Features Added

- Added `DefaultAzureCredential`.

### Other Changes

- Improved log messages.

## 1.4.0 (2023-02-07)

### Features Added

- Added token caching. To benefit from it, share the `shared_ptr` to the same credential instance between multiple client instances.
- Added Azure CLI Credential.
- Added authority host overriding support for `ClientCertificateCredential`.
- Added Azure Stack support for `ClientCertificateCredential`.
- Added Azure App Service API version `2019-08-01` support for `ManagedIdentityCredential`.

## 1.4.0-beta.3 (2023-01-10)

### Features Added

- Added Azure CLI Credential.
- Added authority host overriding support for `ClientCertificateCredential`.
- Added Azure Stack support for `ClientCertificateCredential`.

### Bugs Fixed

- Changed token cache mode to per-credential-instance. In order to get benefits from token caching, share the same credential between multiple client instances.

### Other Changes

- Added token cache support to all credentials.

## 1.4.0-beta.2 (2022-11-08)

### Features Added

- Added token caching.

## 1.4.0-beta.1 (2022-06-30)

### Features Added

- Added Azure App Service API version `2019-08-01` support for `ManagedIdentityCredential`.

## 1.3.0 (2022-06-07)

### Features Added

- Added `ClientCertificateCredential`, and updated `EnvironmentCredential` to support client certificate authentication.
- Added `ChainedTokenCredential`.

## 1.3.0-beta.2 (2022-05-10)

### Features Added

- Added `ClientCertificateCredential`, and updated `EnvironmentCredential` to support client certificate authentication.

## 1.3.0-beta.1 (2022-04-05)

### Features Added

- Added `ChainedTokenCredential`.

## 1.2.0 (2022-03-08)

### Features Added

No changes since `1.2.0-beta.1`.

## 1.2.0-beta.1 (2022-02-08)

### Features Added

- Enabled `EnvironmentCredential` and `ManagedIdentityCredential` to work on UWP.

## 1.1.1 (2022-01-11)

### Bugs Fixed

- [[#2741]](https://github.com/Azure/azure-sdk-for-cpp/issues/2741) Fixed linking problem when Azure SDK is built as DLL.

## 1.1.0 (2021-08-10)

### Features Added

- Added `ManagedIdentityCredential`.

### Bugs Fixed

- Fixed minor memory leak when obtaining a token.

## 1.1.0-beta.1 (2021-07-02)

### Features Added

- Added `ManagedIdentityCredential`.

### Bugs Fixed

- Fixed minor memory leak when obtaining a token.

## 1.0.0 (2021-06-04)

No API changes since `1.0.0-beta.6`.

## 1.0.0-beta.6 (2021-05-18)

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.

## 1.0.0-beta.5 (2021-04-07)

### New Features

- Add Active Directory Federation Service (ADFS) support to `ClientSecretCredential`.

### Breaking Changes

- Removed `Azure::Identity::PackageVersion`.

## 1.0.0-beta.4 (2021-03-11)

### New Features

- Added `Azure::Identity::PackageVersion`.

### Breaking Changes

- Removed `TransportPolicyOptions` from `ClientSecretCredentialOptions`. Updated the options to derive from `ClientOptions`.

## 1.0.0-beta.3 (2021-02-02)

### Breaking Changes

- `ClientSecretCredential` constructor takes `ClientSecretCredentialOptions` struct instead of authority host string. `TokenCredentialOptions` struct has authority host string as data member.

## 1.0.0-beta.2 (2021-01-13)

### Breaking Changes

- Moved `Azure::Identity::Version`, defined in `azure/identity/version.hpp` to the `Azure::Identity::Details` namespace.

### Other Changes and Improvements

- Add high-level and simplified identity.hpp file for simpler include experience for customers.

## 1.0.0-beta.1 (2020-11-11)

### New Features

- Support for Client Secret Credential.
- Support for Environment Credential.
