# Release History

## 12.1.0-beta.1 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 12.0.0 (2022-04-06)

### New Features

- This release includes all features from beta.1 to beta.4. This is the first stable release of a ground-up rewrite of our client libraries to ensure consistency, idiomatic design, productivity and an excellent developer experience. It was created following the [Azure SDK Design Guideline for C++](https://azure.github.io/azure-sdk/cpp_introduction.html).

## 12.0.0-beta.4 (2022-03-09)

### Other Changes

- Changed SAS token signed version to `2018-03-28`.

## 12.0.0-beta.3 (2022-02-14)

### Other Changes

- No public changes in this release.

## 12.0.0-beta.2 (2021-11-08)

### Breaking Changes

- Renamed `GetServicePropertiesResult` to `QueueServiceProperties`.
- Renamed `GetServiceStatisticsResult` to `ServiceStatistics`.
- Renamed `GetQueuePropertiesResult` to `QueueProperties`.
- Renamed `GetQueueAccessPolicyResult` to `QueueAccessPolicy`.
- Wrapped the first parameter of `QueueClient::SetAccessPolicy`, a vector of signed identifiers into a struct `QueueAccessPolicy`.
- Renamed `ReceiveMessagesResult` to `ReceivedMessages`.
- Renamed `PeekMessagesResult` to `PeekedMessages`.

## 12.0.0-beta.1 (2021-09-08)

### New Features

- Added support for Queue features:
  - QueueServiceClient::ListQueues
  - QueueServiceClient::SetProperties
  - QueueServiceClient::GetProperties
  - QueueServiceClient::GetStatistics
  - QueueServiceClient::CreateQueue
  - QueueServiceClient::DeleteQueue
  - QueueClient::Create
  - QueueClient::Delete
  - QueueClient::GetProperties
  - QueueClient::SetMetadata
  - QueueClient::GetAccessPolicy
  - QueueClient::SetAccessPolicy
  - QueueClient::EnqueueMessage
  - QueueClient::ReceiveMessages
  - QueueClient::PeekMessages
  - QueueClient::UpdateMessage
  - QueueClient::DeleteMessage
  - QueueClient::ClearMessages
- Added support for queue SAS.