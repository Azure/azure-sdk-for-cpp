# _Release History

## _1.4.0-beta.4 (Unreleased)

### _Features Added

### _Breaking Changes

### _Bugs Fixed

### _Other Changes

## _1.4.0-beta.3 (2023-01-10)

### _Features Added

- Added Azure CLI Credential.
- Added authority host overriding support for `ClientCertificateCredential`.
- Added Azure Stack support for `ClientCertificateCredential`.

### _Bugs Fixed

- Changed token cache mode to per-credential-instance. In order to get benefits from token caching, share the same credential between multiple client instances.

### _Other Changes

- Added token cache support to all credentials.

## _1.4.0-beta.2 (2022-11-08)

### _Features Added

- Added token caching.

## _1.4.0-beta.1 (2022-06-30)

### _Features Added

- Added Azure App Service API version `2019-08-01` support for `ManagedIdentityCredential`.

## _1.3.0 (2022-06-07)

### _Features Added

- Added `ClientCertificateCredential`, and updated `EnvironmentCredential` to support client certificate authentication.
- Added `ChainedTokenCredential`.

## _1.3.0-beta.2 (2022-05-10)

### _Features Added

- Added `ClientCertificateCredential`, and updated `EnvironmentCredential` to support client certificate authentication.

## _1.3.0-beta.1 (2022-04-05)

### _Features Added

- Added `ChainedTokenCredential`.

## _1.2.0 (2022-03-08)

### _Features Added

No changes since `1.2.0-beta.1`.

## _1.2.0-beta.1 (2022-02-08)

### _Features Added

- Enabled `EnvironmentCredential` and `ManagedIdentityCredential` to work on UWP.

## _1.1.1 (2022-01-11)

### _Bugs Fixed

- [2741](https://github.com/Azure/azure-sdk-for-cpp/issues/2741) Fixed linking problem when Azure SDK is built as DLL.

## _1.1.0 (2021-08-10)

### _Features Added

- Added `ManagedIdentityCredential`.

### _Bugs Fixed

- Fixed minor memory leak when obtaining a token.

## _1.1.0-beta.1 (2021-07-02)

### _Features Added

- Added `ManagedIdentityCredential`.

### _Bugs Fixed

- Fixed minor memory leak when obtaining a token.

## _1.0.0 (2021-06-04)

No API changes since `1.0.0-beta.6`.

## _1.0.0-beta.6 (2021-05-18)

### _Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.

## _1.0.0-beta.5 (2021-04-07)

### _New Features

- Add Active Directory Federation Service (ADFS) support to `ClientSecretCredential`.

### _Breaking Changes

- Removed `Azure::Identity::PackageVersion`.

## _1.0.0-beta.4 (2021-03-11)

### _New Features

- Added `Azure::Identity::PackageVersion`.

### _Breaking Changes

- Removed `TransportPolicyOptions` from `ClientSecretCredentialOptions`. Updated the options to derive from `ClientOptions`.

## _1.0.0-beta.3 (2021-02-02)

### _Breaking Changes

- `ClientSecretCredential` constructor takes `ClientSecretCredentialOptions` struct instead of authority host string. `TokenCredentialOptions` struct has authority host string as data member.

## _1.0.0-beta.2 (2021-01-13)

### _Breaking Changes

- Moved `Azure::Identity::Version`, defined in `azure/identity/version.hpp` to the `Azure::Identity::Details` namespace.

### _Other Changes and Improvements

- Add high-level and simplified identity.hpp file for simpler include experience for customers.

## _1.0.0-beta.1 (2020-11-11)

### _New Features

- Support for Client Secret Credential.
- Support for Environment Credential.
