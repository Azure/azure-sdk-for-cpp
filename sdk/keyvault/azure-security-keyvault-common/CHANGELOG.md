# Release History

## 4.0.0-beta.4 (Unreleased)

### Breaking Changes

- Removed `SHA256`, `SHA384`, and `SHA512` hashing classes by making them internal since the end user doesn't need them.
- Removed header `single_page.hpp`.

## 4.0.0-beta.3 (2021-06-08)

No breaking changes or new features added. Includes only implementation enhancements.

## 4.0.0-beta.2 (2021-05-18)

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Removed `KeyVaultException`.
- Removed `ClientOptions`.

## 4.0.0-beta.1 (2021-04-07)

### New Features

- KeyVaultException.
