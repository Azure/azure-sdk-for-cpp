# Release History

## 12.1.0-beta.1 (Unreleased)

### Bugs Fixed

- Fixed a memory leak issue while parsing XML.

## 12.0.0 (2021-06-08)

### Other Changes

- Fixed a filename encoding issue.

## 12.0.0-beta.11 (2021-05-19)

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Removed `Azure::PagedResponse<T>`.

### Bugs Fixed

- Fixed a stream leak issue in `ReliableStream`.

## 12.0.0-beta.10 (2021-04-16)

### Features Added

- Added server timeout support.
- Added `Azure::PagedResponse<T>` for returning paginated collections.

### Breaking Changes

- Removed `Azure::Storage::Common::PackageVersion`.
- Moved `ReliableStream` to internal namespace.
- Removed `HttpGetterInfo` and `HTTPGetter` from the `Azure::Storage` namespace.

## 12.0.0-beta.9 (2021-03-23)

### Features Added

- Added `Azure::Storage::Common::PackageVersion`.

## 12.0.0-beta.8 (2021-02-12)

### Breaking Changes

- Removed the `Azure::Storage::Md5` class from `crypt.hpp`. Use the type from `Azure::Core::Cryptography` namespace instead, from `azure/core/cryptography/hash.hpp`.
- Renamed `Crc64` to `Crc64Hash` and change it to derive from the `Azure::Core::Cryptography::Hash` class.

## 12.0.0-beta.7 (2021-02-03)

### Features Added

- Added additional information in `StorageException`.

### Breaking Changes

- `AccountSasResource::BlobContainer` was renamed to `AccountSasResource::Container`.

### Bugs Fixed

- Fixed `ClientRequestId` wasn't filled in `StorageException`.

## 12.0.0-beta.6 (2021-01-14)

### Features Added

- Added new type `ContentHash`.
- Added definition of `Metadata`.
- Support setting account SAS permission with a raw string.

### Breaking Changes

- Renamed `SharedKeyCredential` to `StorageSharedKeyCredential`.
- Renamed `StorageSharedKeyCredential::UpdateAccountKey` to `Update`.
- Made `StorageRetryPolicy`, `StoragePerRetryPolicy` and `SharedKeyPolicy` private by moving them to the `Details` namespace.
- Removed `StorageRetryOptions`, use `Azure::Core::Http::RetryOptions` instead.
- Moved Account SAS into `Azure::Storage::Sas` namespace.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Made version strings private by moving them into the `Details` namespace.
- Moved `Base64Encode` and `Base64Decode` from the `Azure::Storage` namespace to `Azure::Core` and removed the string accepting overload of `Base64Encode`.
- Renamed public constants so they no longer start with the prefix `c_`. For example, `c_InfiniteLeaseDuration` became `InfiniteLeaseDuration`.

### Bugs Fixed

- Fixed default EndpointSuffix when parsing a connection string. (A community contribution, courtesy of _[lordgamez](https://github.com/lordgamez)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- Gabor Gyimesi _([GitHub](https://github.com/lordgamez))_

## 12.0.0-beta.5 (2020-11-13)

### Breaking Changes

- Rename `LastModifiedTimeAccessConditions` to `ModifiedTimeConditions`.
- Rename `StorageError` to `StorageException`.
- Rename header file `storage_error.hpp` to `storage_exception.hpp`.
- Rename `SharedKeyCredential::SetAccountKey` to `SharedKeyCredential::UpdateAccountKey`.
- Rename `AccountSasBuilder::ToSasQueryParameters` to `AccountSasBuilder::GenerateSasToken`.
- Remove `storage_version.hpp` and add `version.hpp`.
- Make `SharedKeyCredential` a class.

### Other Changes

- Remove support for specifying SAS version.

## 1.0.0-beta.3 (2020-10-13)

### Features Added

- Support for customizable retry policy.

## 1.0.0-beta.2 (2020-09-09)

### Features Added

- Release based on azure-core_1.0.0-beta.1.

## 1.0.0-beta.1 (2020-08-28)

### Features Added

- Support for Account SAS.
- Support for Base64 Encoding/Decoding.
- Support for MD5, CRC64.
- Support for Shared Key Credential.
