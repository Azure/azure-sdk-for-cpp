# Release History

## 1.0.0-beta.5 (Unreleased)

### Features Added

### Breaking Changes

- EventHub `ConsumerClient` and `ProcessorClient` objects now return pointers to `EventData` objects instead of `EventData` objects by value.

### Bugs Fixed

### Other Changes

## 1.0.0-beta.4 (2023-11-07)

### Features Added

- Fully functional eventhubs Processor.
- Allow `ProducerClient` and `ConsumerClient` to be created with a connection string without an EntityPath element.

### Breaking Changes

- Removed the `LoadBalancer` type from the public API surface.
- `ConsumerClient` and `ProducerClient` objects can no longer be moved or copied.
- If the connection string provided to `ConsumerClient` or `ProducerClient` contains an EntityPath, then the `EntityPath` 
parameter to the constructor must match the value provided in the connection string.

### Other Changes

- Several `ostream` insertion operators were added for eventhubs types.

## 1.0.0-beta.3 (2023-10-10)

### Breaking Changes

- Removed all direct dependencies on Azure Blob Storage and moved those dependencies into a helper package (`azure-messaging-eventhubs-checkpointstore-blob-cpp`).

## 1.0.0-beta.2 (2023-09-12)

### Features Added

- `ProducerClient` now has convenience methods for sending events without batching.
- Added `std::ostream` insertion operators for model types to simplify debugging.

### Breaking Changes

- Storage names used for checkpoint store have been normalized to match behavior of other Azure SDK eventhubs packages.
- `EventDataBatch` object can no longer be directly created but instead must be created via `ProducerClient::CreateEventDataBatch`.
- `EventDataBatch::AddMessage` method has been renamed to `EventDataBatch::TryAddMessage` and it now returns false if the message will not fit.
- `SendEventDataBatch` method has been renamed to `Send` and it now returns a void (throwing an exception of the send fails).

### Bugs Fixed

- Setting `PartitionClientOptions::StartPosition::EnqueuedTime` now works as expected.
- Internally restructured how AMQP senders and receivers are configured to simplify code and significantly improve reliability.

### Other Changes

- Azure CLI examples added to README.md file.

## 1.0.0-beta.1 (2023-08-08)

### Features Added

- Initial release.
- Supported scenarios: Sending events and receiving events.
See [README.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/README.md) for more information on how to use the EventHubs client.
