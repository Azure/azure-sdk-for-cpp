# Release History

## 12.0.0-beta.6 (Unreleased)

### New Features

- Add new type `ContentHash`.

### Breaking Changes

- Rename `SharedKeyCredential` to `StorageSharedKeyCredential`.
- Rename `StorageSharedKeyCredential::UpdateAccountKey` to `StorageSharedKeyCredential::Update`.
- Move `StorageRetryPolicy`, `StoragePerRetryPolicy` and `SharedKeyPolicy` to `Details` namespace.
- Remove `StorageRetryOptions`, use `Azure::Core::Http::RetryOptions` instead.
- Move Account SAS into `Azure::Storage::Sas` namespace.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Move version strings into `Details` namespace.
- Move `Base64Encode` and `Base64Decode` from the `Azure::Storage` namespace to `Azure::Core`.
- Rename the string accepting overload of `Base64Encode` to `Base64EncodeText` and make it internal by moving it to the `Azure::Storage::Internal` namespace.

## 12.0.0-beta.5 (2020-11-13)

### Breaking Changes

- Rename `LastModifiedTimeAccessConditions` to `ModifiedTimeConditions`.
- Rename `StorageError` to `StorageException`.
- Rename header file `storage_error.hpp` to `storage_exception.hpp`.
- Rename `SharedKeyCredential::SetAccountKey` to `SharedKeyCredential::UpdateAccountKey`.
- Rename `AccountSasBuilder::ToSasQueryParameters` to `AccountSasBuilder::GenerateSasToken`.
- Remove `storage_version.hpp` and add `version.hpp`.
- Make `SharedKeyCredential` a class.

### Other Changes and Improvements

- Remove support for specifying SAS version.

## 1.0.0-beta.3 (2020-10-13)

### New Features

- Support for customizable retry policy.

## 1.0.0-beta.2 (2020-09-09)

### New Features

- Release based on azure-core_1.0.0-beta.1.

## 1.0.0-beta.1

### New Features

- Support for Account SAS.
- Support for Base64 Encoding/Decoding.
- Support for MD5, CRC64.
- Support for Shared Key Credential.
