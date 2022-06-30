# Release History

## 4.3.0-beta.1 (Unreleased)

### Features Added

- Keyvault 7.3 support added for Keys. 

## 4.2.0 (2021-10-05)

### Features Added

- [2833](https://github.com/Azure/azure-sdk-for-cpp/issues/2833) Added `GetCryptographyClient()` to `KeyClient` to return a `CryptographyClient` that uses the same options, policies, and pipeline as the `KeyClient` that created it.

## 4.1.0 (2021-09-08)

### Features Added

- Added `GetUrl()` to `KeyClient`.

### Bugs Fixed

- [2750](https://github.com/Azure/azure-sdk-for-cpp/issues/2750) Support for Azure `managedhsm` cloud and any other non-public Azure cloud.

## 4.0.0 (2021-08-10)

### Other Changes

- Consolidated keyvault and cryptography client options and model files into single headers.

## 4.0.0-beta.4 (2021-07-20)

### Features Added

- Added `GetIv()` to `EncryptParameters` and `DecryptParameters`.
- Added `BackupKeyResult` for `BackupKey()` return type.

### Breaking Changes

- Removed `Azure::Security::KeyVault::Keys::ServiceVersion::V7_0` and `V7_1`.
- Removed `Azure::Security::KeyVault::Keys::Cryptography::ServiceVersion::V7_0` and `V7_1`.
- Removed `CryptographyClient::RemoteClient()` and `CryptographyClient::LocalOnly()`.
- Removed the general constructor from `EncryptParameters` and `DecryptParameters`.
- Removed access to `Iv` field member from `EncryptParameters` and `DecryptParameters`.
- Removed `Encrypt(EncryptionAlgorithm, std::vector, context)`.
- Removed `Decrypt(DecryptAlgorithm, std::vector, context)`.
- Removed `JsonWebKey::HasPrivateKey()`.
- Removed the `MaxPageResults` field from `GetPropertiesOfKeysOptions`, `GetPropertiesOfKeyVersionsOptions`, and `GetDeletedKeysOptions`.
- Renamed header `list_keys_single_page_result.hpp` to `list_keys_responses.hpp`.
- Updated `BackupKey()` API return type to `BackupKeyResult` model type.
- Renamed `KeyPropertiesPageResult` to `KeyPropertiesPagedResponse`.
- Renamed `DeletedKeyPageResult` to `DeletedKeyPagedResponse`.
- Changed the container for `KeyOperations` from `std::list` to `std::vector` within `CreateKeyOptions` and `UpdateKeyProperties()`.
- Changed the return type of `CrytographyClient` APIs like `Encrypt()` to return `Response<T>` rather than the `T` directly.
- Renamed high-level header from `key_vault_keys.hpp` to `keyvault_keys.hpp`.

## 4.0.0-beta.3 (2021-06-08)

### Breaking Changes

- Updated `MaxPageResults` type to `int32_t`, from `uint32_t`, affecting:
  - `GetDeletedKeysOptions()`.
  - `GetPropertiesOfKeysOptions()`.
  - `GetPropertiesOfKeyVersionsOptions()`.
- Updated `CreateRsaKeyOptions::KeySize` type from `uint64_t` to `int64_t`.
- Updated `CreateRsaKeyOptions::PublicExponent` type from `uint64_t` to `int64_t`.
- Updated `CreateOctKeyOptions::KeySize` type from `uint64_t` to `int64_t`.

## 4.0.0-beta.2 (2021-05-18)

### New Features

- Added support for importing and deserializing EC and OCT keys.
- Added cryptography client.
- Added `CreateFromResumeToken()` to `DeletedKeyOperation` and `RecoverKeyOperation`.

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Renamed `GetPropertiesOfKeysSinglePage()` to `GetPropertiesOfKeys()`.
- Renamed `GetPropertiesOfKeyVersionsSinglePage()` to `GetPropertiesOfKeyVersions()`.
- Renamed `GetDeletedKeysSinglePage()` to `GetDeletedKeys()`.
- Renamed `KeyPropertiesSinglePage` to `KeyPropertiesPageResult`.
- Renamed `DeletedKeySinglePage` to `DeletedKeyPageResult`.
- Renamed `GetPropertiesOfKeysSinglePageOptions` to `GetPropertiesOfKeysOptions`.
- Renamed `GetPropertiesOfKeyVersionsSinglePageOptions` to `GetPropertiesOfKeyVersionsOptions`.
- Renamed `GetDeletedKeysSinglePageOptions` to `GetDeletedKeysOptions`.
- Removed `Azure::Security::KeyVault::Keys::JsonWebKey::to_json`.
- Replaced static functions from `KeyOperation` and `KeyCurveName` with static const members.
- Replaced the enum `JsonWebKeyType` for a class with static const members as an extensible enum called `KeyVaultKeyType`.
- Renamed `MaxResults` to `MaxPageResults` for `GetSinglePageOptions`.
- Changed the returned type for list keys, key versions, and deleted keys from `Response<T>` to `PagedResponse<T>` affecting:
  - `GetPropertiesOfKeysSinglePage()` and `GetPropertiesOfKeyVersionsSinglePage()` now returns `KeyProperties`.
  - `GetDeletedKeysSinglePage()` now returns `DeletedKey`.
- Removed `ResumeDeleteKeyOperation()` and `ResumeRecoverKeyOperation()`.

### Bug Fixes

- Fix getting a resume token from delete and recover key operations.

## 4.0.0-beta.1 (2021-04-07)

### New Features

- Added `Azure::Security::KeyVault::Keys::KeyClient` for get, create, list, delete, backup, restore, and import key operations.
- Added high-level and simplified `key_vault.hpp` file for simpler include experience for customers.
- Added model types which are returned from the `KeyClient` operations, such as `Azure::Security::KeyVault::Keys::KeyVaultKey`.
