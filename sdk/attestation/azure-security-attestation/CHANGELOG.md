# Release History

## 1.0.0-beta.3 (Unreleased)

### Features Added

### Breaking Changes
- `ValueToSend` field in `TpmAttestationOptions` becomes `Payload`.
- `AddIsolatedModeCertificatesOptions` becomes `AddIsolatedModeCertificateOptions`
- `RemoveIsolatedModeCertificatesOptions` becomes `RemoveIsolatedModeCertificateOptions`
- Renamed `AttestEnclaveOptions` to `AttestSgxEnclaveOptions` and `AttestOpenEnclaveOptions`.
- Split out `AttestationClient::Create` into its own factory class `AttestationClientFactory`.
    - Note that the `AttestationClientFactory::Create` method returns a `std::unique_ptr` to the client object.
- Split out `AttestationAdministrationClient::Create` into its own factory class `AttestationAdministrationClientFactory`.
    - Note that the `AttestationAdministrationClientFactory::Create` method returns a `std::unique_ptr` to the client object.

### Bugs Fixed

### Other Changes

## 1.0.0-beta.2 (2022-05-10)

### Breaking Changes

- Renamed `AttestationOpenIdMetadata` type to `OpenIdMetadata`.
- Renamed `AttestationSigningCertificateResult` type to `TokenValidationCertificateResult` to more accurately reflect the
  purpose of the type.
- Removed the `AttestationTokenBase` class and moved its contents to the `AttestationToken` class.
- Empty `AttestationToken` types are now represented with `AttestationToken<void>` rather than `AttestationToken<>` to more idiomatically express the idea of a nullable attestation token.
- Renamed `RuntimeClaims` field to `RunTimeClaims` to align with `InitTimeClaims` type name; standardized spelling of
  `InitTimeClaims`.
- Changed input parameter to `AttestTpm` to be `AttestTpmOptions` instead of `std::string`.
- Changed output parameter of `AttestTpm` to be `TpmAttestationResult` instead of `std::string`.
- Renamed `AttestationTokenValidationOptions::ValidationTimeSlack` to `AttestationTokenValidationOptions::TimeValidationSlack`
  to improve consistency with other attestation SDKs.
- Removed the unused `AttestationValidationCollateral` API.
- Renamed `AttestOptions` to `AttestEnclaveOptions`
- Renamed `TokenValidationOptions` field in various API Options structures to be `TokenValidationOptionsOverride` to better
  reflect the semantics of the field.
- Renamed `PolicyCertificate` types to `IsolatedMode`.
  - `PolicyCertificateModificationResult` becomes `IsolatedModeCertificateModificationResult`
  - `PolicyCertificateListResult` becomes `IsolatedModeCertificateListResult`
  - `GetPolicyManagementCertificateOptions` becomes `GetIsolatedModeCertificatesOptions`
  - `AddPolicyManagementCertificatesOptions` becomes `AddIsolatedModeCertificatesOptions`
  - `RemovePolicyManagementCertificatesOptions` becomes `RemoveIsolatedModeCertificatesOptions`
  - `AttestationAdministrationClient::GetPolicyManagementCertificates` becomes `AttestationAdministrationClient::GetIsolatedModeCertificates`.
  - `AttestationAdministrationClient::AddPolicyManagementCertificate` becomes `AttestationAdministrationClient::AddIsolatedModeCertificate`.
  - `AttestationAdministrationClient::RemovePolicyManagementCertificate` becomes `AttestationAdministrationClient::RemoveIsolatedModeCertificate`.
- Removed `ClientVersion` API from `AttestationClient` and `AttestationAdministrationClient`

### Other Changes

- Added `Endpoint` property to `AttestationClient` and `AttestationAdministrationClient`

## 1.0.0-beta.1 (2022-04-05)

### Features Added

- Attestation Package creation
