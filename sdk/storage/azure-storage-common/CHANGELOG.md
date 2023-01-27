# _Release History

## _12.4.0-beta.1 (Unreleased)

### _Features Added

### _Breaking Changes

### _Bugs Fixed

### _Other Changes

## _12.3.0 (2022-09-06)

### _Features Added

- Features in `12.3.0-beta.1` are now generally available.

## _12.3.0-beta.1 (2022-08-09)

### _Features Added

- Added support for encryption scope SAS (`ses` query parameter in SAS token).
- Added support for permanent delete permission in SAS.

## _12.2.4 (2022-06-07)

### _Bugs Fixed

- Fixed a bug where text of XML element cannot be empty.

## _12.2.3 (2022-04-06)

### _Bugs Fixed

- Fixed a bug where we got error when XML request body is too big.

## _12.2.2 (2022-03-09)

### _Features Added

- Added `SetImmutabilityPolicy` permission for account SAS.
- Bumped up SAS token service version to `2020-08-04`.

## _12.2.1 (2022-02-14)

### _Other Changes

- No public changes in this release.

## _12.2.0 (2021-09-08)

### _Features Added

- Used new xml library on Windows, dropped dependency for libxml2.

### _Bugs Fixed

- Fixed a bug that may cause crash when parsing XML.

## _12.1.0 (2021-08-10)

### _Bugs Fixed

- Avoid time domain casting exception during request cancellation. (A community contribution, courtesy of _[johnwheffner](https://github.com/johnwheffner)_)

### _Acknowledgments

Thank you to our developer community members who helped to make Azure Storage better with their contributions to this release:

- John Heffner _([GitHub](https://github.com/johnwheffner))_

## _12.0.1 (2021-07-07)

### _Bug Fixes

- Fixed a memory leak issue while parsing XML.

## _12.0.0 (2021-06-08)

### _Other Changes and Improvements

- Fixed a filename encoding issue.

## _12.0.0-beta.11 (2021-05-19)

### _Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Removed `Azure::PagedResponse<T>`.

### _Bug Fixes

- Fixed a stream leak issue in `ReliableStream`.

## _12.0.0-beta.10 (2021-04-16)

### _New Features

- Added server timeout support.
- Added `Azure::PagedResponse<T>` for returning paginated collections.

### _Breaking Changes

- Removed `Azure::Storage::Common::PackageVersion`.
- Moved `ReliableStream` to internal namespace.
- Removed `HttpGetterInfo` and `HTTPGetter` from the `Azure::Storage` namespace.

## _12.0.0-beta.9 (2021-03-23)

### _New Features

- Added `Azure::Storage::Common::PackageVersion`.

## _12.0.0-beta.8 (2021-02-12)

### _Breaking Changes

- Removed the `Azure::Storage::Md5` class from `crypt.hpp`. Use the type from `Azure::Core::Cryptography` namespace instead, from `azure/core/cryptography/hash.hpp`.
- Renamed `Crc64` to `Crc64Hash` and change it to derive from the `Azure::Core::Cryptography::Hash` class.

## _12.0.0-beta.7 (2021-02-03)

### _New Features

- Added additional information in `StorageException`.

### _Breaking Changes

- `AccountSasResource::BlobContainer` was renamed to `AccountSasResource::Container`.

### _Bug Fixes

- Fixed `ClientRequestId` wasn't filled in `StorageException`.

## _12.0.0-beta.6 (2021-01-14)

### _New Features

- Added new type `ContentHash`.
- Added definition of `Metadata`.
- Support setting account SAS permission with a raw string.

### _Breaking Changes

- Renamed `SharedKeyCredential` to `StorageSharedKeyCredential`.
- Renamed `StorageSharedKeyCredential::UpdateAccountKey` to `Update`.
- Made `StorageRetryPolicy`, `StoragePerRetryPolicy` and `SharedKeyPolicy` private by moving them to the `Details` namespace.
- Removed `StorageRetryOptions`, use `Azure::Core::Http::RetryOptions` instead.
- Moved Account SAS into `Azure::Storage::Sas` namespace.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Made version strings private by moving them into the `Details` namespace.
- Moved `Base64Encode` and `Base64Decode` from the `Azure::Storage` namespace to `Azure::Core` and removed the string accepting overload of `Base64Encode`.
- Renamed public constants so they no longer start with the prefix `c_`. For example, `c_InfiniteLeaseDuration` became `InfiniteLeaseDuration`.

### _Bug Fixes

- Fixed default EndpointSuffix when parsing a connection string. (A community contribution, courtesy of _[lordgamez](https://github.com/lordgamez)_)

### _Acknowledgments

Thank you to our developer community members who helped to make Azure Storage better with their contributions to this release:

- Gabor Gyimesi _([GitHub](https://github.com/lordgamez))_

## _12.0.0-beta.5 (2020-11-13)

### _Breaking Changes

- Rename `LastModifiedTimeAccessConditions` to `ModifiedTimeConditions`.
- Rename `StorageError` to `StorageException`.
- Rename header file `storage_error.hpp` to `storage_exception.hpp`.
- Rename `SharedKeyCredential::SetAccountKey` to `SharedKeyCredential::UpdateAccountKey`.
- Rename `AccountSasBuilder::ToSasQueryParameters` to `AccountSasBuilder::GenerateSasToken`.
- Remove `storage_version.hpp` and add `version.hpp`.
- Make `SharedKeyCredential` a class.

### _Other Changes and Improvements

- Remove support for specifying SAS version.

## _1.0.0-beta.3 (2020-10-13)

### _New Features

- Support for customizable retry policy.

## _1.0.0-beta.2 (2020-09-09)

### _New Features

- Release based on azure-core_1.0.0-beta.1.

## _1.0.0-beta.1 (2020-08-28)

### _New Features

- Support for Account SAS.
- Support for Base64 Encoding/Decoding.
- Support for MD5, CRC64.
- Support for Shared Key Credential.
