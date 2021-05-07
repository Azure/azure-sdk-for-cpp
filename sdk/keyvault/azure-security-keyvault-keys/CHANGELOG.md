# Release History

## 4.0.0-beta.2 (Unreleased)

### New Features

- Added support for importing and deserializing EC and OCT keys.
- Added cryptography client.
- Added `CreateFromResumeToken()` to `DeletedKeyOperation` and `RecoverKeyOperation`.

### Breaking Changes

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
