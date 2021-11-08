# Release History

## 4.0.0-beta.1 

### New Features

Initial preview of Azure Security Key Vault Secrets API for CPP.

- Added `Azure::Security::KeyVault::Certificates::CertificateClient` for get, create, list, delete, backup, restore, and import certificate operations.
- Added high-level and simplified `keyvault_certificates.hpp` file for simpler include experience for customers.
- Added model types which are returned from the `CertificateClient` operations, such as `Azure::Security::KeyVault::Certificates::KeyVaultCertificate`.
