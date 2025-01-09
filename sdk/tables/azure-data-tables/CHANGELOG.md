# Release History

## 1.0.0-beta.6 (2025-01-14)

### Breaking Changes

- Removed constructor for SAS token authentication in `TableServiceClient` and `TableClient`.
- Simplified APIs by removing redundant structures.

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
