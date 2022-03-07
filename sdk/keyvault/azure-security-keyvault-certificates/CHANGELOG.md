# Release History

## 4.0.0-beta.2 (Unreleased)

### Features Added

### Breaking Changes
- Updated CreateCertificateOperation.PollUntilDone method (returned from StartCreateCertificate call)  to return the operation status instead of the newly created certificate.

### Bugs Fixed

### Other Changes

## 4.0.0-beta.1 (2021-11-09)

### New Features

- Initial beta release of Azure Security Key Vault Certificates API for CPP.
  - Added `Azure::Security::KeyVault::Certificates::CertificateClient` for get, create, list, delete, backup, restore, and import certificate operations.
  - Added high-level and simplified `keyvault_certificates.hpp` file for simpler include experience for customers.
  - Added model types which are returned from the `CertificateClient` operations, such as `Azure::Security::KeyVault::Certificates::KeyVaultCertificate`.
