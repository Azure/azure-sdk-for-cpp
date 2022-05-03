# Release History

## 1.0.0-beta.2 (2022-05-10)

### Features Added

No new features, addressed API Review Feedback.

### Breaking Changes

Breaking Changes from API Review

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
