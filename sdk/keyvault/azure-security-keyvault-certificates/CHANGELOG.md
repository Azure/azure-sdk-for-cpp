# Release History

## 4.3.0-beta.1 (2024-04-09)

### Features Added

- Updated to API version 7.5.

## 4.2.1 (2024-01-16)

### Bugs Fixed

- [[#4754]](https://github.com/Azure/azure-sdk-for-cpp/issues/4754) Thread safety for authentication policy.

## 4.2.0 (2023-05-09)

### Features Added

- Added support for challenge-based and multi-tenant authentication.

## 4.2.0-beta.1 (2023-04-11)

### Features Added

- Added support for challenge-based and multi-tenant authentication.

## 4.1.0 (2022-10-11)

### Features Added

- Keyvault 7.3 support added for Certificates.

## 4.1.0-beta.1 (2022-07-07)

### Features Added

- Keyvault 7.3 support added for Certificates.

### Breaking Changes

- Removed ServiceVersion type, replaced with ApiVersion field in the CertificateClientOptions type.

## 4.0.0 (2022-06-07)

### Breaking Changes

- Renamed `keyvault_certificates.hpp` to `certificates.hpp`.

## 4.0.0-beta.2 (2022-03-08)

### Breaking Changes
- Updated `CreateCertificateOperation.PollUntilDone()` (returned from `StartCreateCertificate()`)  to return the operation status instead of the newly created certificate.

## 4.0.0-beta.1 (2021-11-09)

### New Features

- Initial beta release of Azure Security Key Vault Certificates API for CPP.
  - Added `Azure::Security::KeyVault::Certificates::CertificateClient` for get, create, list, delete, backup, restore, and import certificate operations.
  - Added high-level and simplified `keyvault_certificates.hpp` file for simpler include experience for customers.
  - Added model types which are returned from the `CertificateClient` operations, such as `Azure::Security::KeyVault::Certificates::KeyVaultCertificate`.
