# Release History

## 4.0.0-beta.2 (Unreleased)

### New Features

- Added support for importing and deserializing EC and OCT keys.

### Breaking Changes

- Removed `Azure::Security::KeyVault::Keys::JsonWebKey::to_json`.
- Replaced static functions from `KeyOperation` and `KeyCurveName` for static const members.
- Replaced the enum `JsonWebKeyType` for a class with static const members as ext enum.
- Renamed `MaxResults` to `MaxPageResults` for `GetSinglePageOptions`.

### Bug Fixes

- Fix getting a resume token from delete and recover key operations.

## 4.0.0-beta.1 (2021-04-07)

### New Features

- Added `Azure::Security::KeyVault::Keys::KeyClient` for get, create, list, delete, backup, restore, and import key operations.
- Added high-level and simplified `key_vault.hpp` file for simpler include experience for customers.
- Added model types which are returned from the `KeyClient` operations, such as `Azure::Security::KeyVault::Keys::KeyVaultKey`.
