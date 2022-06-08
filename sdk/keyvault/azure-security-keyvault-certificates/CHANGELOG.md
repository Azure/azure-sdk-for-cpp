# Release History

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
