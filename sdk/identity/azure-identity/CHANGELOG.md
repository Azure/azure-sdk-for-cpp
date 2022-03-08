# Release History

## 1.2.0 (Unreleased)

No changes since `1.2.0-beta.1`.

## 1.2.0-beta.1 (2022-02-08)

### Features Added

- Enabled `EnvironmentCredential` and `ManagedIdentityCredential` to work on UWP.

## 1.1.1 (2022-01-11)

### Bugs Fixed

- [2741](https://github.com/Azure/azure-sdk-for-cpp/issues/2741) Fixed linking problem when Azure SDK is built as DLL.

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
