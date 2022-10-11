# Release History

## 1.1.0-beta.1 (Unreleased)

### Features Added

### Breaking Changes

- Changed `AttestationClient::AttestTpm` to match `AttestOpenEnclave` and `AttestSgxEnclave`.
- Added `std::vector<uint8_t>` dataToAttest parameter to the AttestTpm() client method.
- Removed `Payload` in `TpmAttestationOptions`.
- Changed `TpmResult` in `TpmAttestationResult` to type `std::vector<uint8_t>`.

### Bugs Fixed

### Other Changes

## 1.0.0 (2022-07-07)

### Breaking Changes

- Renamed `Version` field to `ApiVersion` and removed the `ServiceVersion` enumeration.

## 1.0.0-beta.3 (2022-06-07)

### Breaking Changes

- `ValueToSend` field in `TpmAttestationOptions` becomes `Payload`.
- `AddIsolatedModeCertificatesOptions` becomes `AddIsolatedModeCertificateOptions`
- `RemoveIsolatedModeCertificatesOptions` becomes `RemoveIsolatedModeCertificateOptions`
- Renamed `AttestEnclaveOptions` to `AttestSgxEnclaveOptions` and `AttestOpenEnclaveOptions`.
- `AttestationClient` and `AttestationAdministrationClient` creation is now done using the factory method `AttestationClient::Create()` and `AttestationAdministrationClient::Create()`. 

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
