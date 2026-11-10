# Release History

## 1.0.0-beta.9 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 1.0.0-beta.8 (2026-11-09)

### Breaking Changes

- `UpdateEntity` and `MergeEntity` now send `If-Match: *` when the entity's ETag is empty. A missing entity returns 404 instead of being inserted. Use `UpdateOrInsertEntity` or `MergeOrInsertEntity` to retain the previous upsert behavior.
- Serializing an invalid Boolean, Int32, or Double property value now throws `std::invalid_argument` instead of sending the invalid value to the service.

### Bugs Fixed

- Fixed multipart transaction framing, the `InsertReplace` request method and body, and `UpdateMerge` ETag conditions.
- Preserved native JSON Boolean, Int32, and Double property types when responses omit OData type annotations.
- Serialized Boolean, Int32, and finite Double properties as native JSON values, preserving their types during read-modify-write operations. Int64 and non-finite Double values retain their string representation and type annotations.
- Preserved query options and key selectors across pages and supported partition-only continuation tokens.
- Escaped single quotes in OData entity keys before URL encoding for point, query, and transaction requests.

## 1.0.0-beta.7 (2026-11-04)

### Features Added

- [[#6522]](https://github.com/Azure/azure-sdk-for-cpp/pull/6522) Added `TableClientOptions::Audience` to configure the audience used to derive the authentication token scope.

### Other Changes

- Added support for ICU 75.1 or later. (A community contribution, courtesy of _[kou](https://github.com/kou)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Data Tables better with their contributions to this release:

- Sutou Kouhei _([GitHub](https://github.com/kou))_

## 1.0.0-beta.6 (2025-01-22)

### Breaking Changes

- Removed constructor for SAS token authentication in `TableServiceClient` and `TableClient`.
- Simplified APIs by removing redundant structures.
- Changes to the Update/Upsert APIs.

## 1.0.0-beta.5 (2024-11-22)


### Breaking Changes

- Renamed `tables_clients.hpp` to `table_client.hpp` and split `TableServiceClient` into its own file, `table_service_client.hpp`. 
- Removed the `TablesAudience` field from `TableClientOptions` since it is not required.
- Removed ServiceVersion type and changed the ApiVersion field within `TableClientOptions` to be std::string.
- Removed the `TableServiceClient` constructor that only accepts one defaulted options parameter.

### Bugs Fixed

- Use the package version for telemetry, rather than API version.

### Other Changes

- Updated samples to reflect the changes in the client.

## 1.0.0-beta.4 (2024-08-06)

### Bugs Fixed

- [[#5781]](https://github.com/Azure/azure-sdk-for-cpp/pull/5781) Fixed exception when deserializing numeric values from JSON. (A community contribution, courtesy of _[0xar1](https://github.com/0xar1)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Data Tables better with their contributions to this release:

- arwell _([GitHub](https://github.com/0xar1))_

## 1.0.0-beta.3 (2024-06-11)

### Bugs Fixed

- Fixed an issue where the `TableServiceClient` was not correctly handling the `nextPartitionKey` and `nextRowKey` continuation tokens when iterating over tables.
- Fixed an issue around InsertReplace transactions.

## 1.0.0-beta.2 (2024-04-09)

### Features Added

- Updates to models, transactions and other features.

## 1.0.0-beta.1 (2024-01-16)

### Features Added

- Initial release.
