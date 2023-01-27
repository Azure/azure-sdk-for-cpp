# _Release History

## _4.2.0-beta.1 (Unreleased)

### _Features Added

### _Breaking Changes

### _Bugs Fixed

### _Other Changes

## _4.1.0 (2022-10-11)

### _Features Added

- Keyvault 7.3 support added for Certificates.

## _4.1.0-beta.1 (2022-07-07)

### _Features Added

- Keyvault 7.3 support added for Certificates.

### _Breaking Changes

- Removed ServiceVersion type, replaced with ApiVersion field in the CertificateClientOptions type.

## _4.0.0 (2022-06-07)

### _Breaking Changes

- Renamed `keyvault_certificates.hpp` to `certificates.hpp`.

## _4.0.0-beta.2 (2022-03-08)

### _Breaking Changes
- Updated `CreateCertificateOperation.PollUntilDone()` (returned from `StartCreateCertificate()`)  to return the operation status instead of the newly created certificate.

## _4.0.0-beta.1 (2021-11-09)

### _New Features

- Initial beta release of Azure Security Key Vault Certificates API for CPP.
  - Added `Azure::Security::KeyVault::Certificates::CertificateClient` for get, create, list, delete, backup, restore, and import certificate operations.
  - Added high-level and simplified `keyvault_certificates.hpp` file for simpler include experience for customers.
  - Added model types which are returned from the `CertificateClient` operations, such as `Azure::Security::KeyVault::Certificates::KeyVaultCertificate`.
