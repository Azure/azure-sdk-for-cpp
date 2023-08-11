# Release History

## 1.0.0-beta.2 (Unreleased)

### Features Added

### Breaking Changes

- Storage names used for checkpoint store have been normalized to match behavior of other eventhubs clients.
- `EventDataBatch` object can no longer be directly created but instead must be created via `ProducerClient::CreateEventDataBatch`.
- `EventDataBatch::AddMessage` method has been renamed to `EventDataBatch::TryAdd` and it now returns false if the message will not fit.
- `SendEventDataBatch` method has been renamed to `Send` and it now returns a void.

### Bugs Fixed

- Setting `PartitionClientOptions::StartPosition::EnqueuedTime` now works as expected.

### Other Changes

- Azure CLI examples added to README.md file.

## 1.0.0-beta.1 (2023-08-08)

### Features Added

- Initial release.
- Supported scenarios: Sending events and receiving events.
See [README.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/README.md) for more information on how to use the EventHubs client.
