# Release History

## 12.4.0 (2024-09-17)

- New features in `12.4.0-beta.1` are now generally available.

## 12.4.0-beta.1 (2024-08-07)

### Features Added

- Added ability to retrieve SAS string to sign for debugging purposes.

## 12.3.0 (2024-07-16)

### Features Added

- New features in `12.3.0-beta.1` are now generally available.

## 12.3.0-beta.1 (2024-06-11)

### Features Added

- Bumped up API version to `2024-08-04`.
- Added more detailed messaging for authorization failure cases.

## 12.2.0 (2023-11-07)

### Features Added

- New features in `12.2.0-beta.1` are now generally available.

## 12.2.0-beta.1 (2023-10-17)

### Features Added

- Added new extendable enum `QueueAudience`
- Added new field `Audience` in `QueueClientOptions`

## 12.1.0 (2023-09-12)

### Features Added

- New features in `12.1.0-beta.1` are now generally available.

## 12.1.0-beta.1 (2023-08-12)

### Features Added
- Bumped up API version to `2019-12-12`.
- Bumped up SAS token version to `2019-12-12`.
- TenantId can now be discovered through the service challenge response, when using a TokenCredential for authorization.
    - A new property is now available on `QueueClientOptions` called `EnableTenantDiscovery`. If set to `true`, the client will attempt an initial unauthorized request to the service to prompt a challenge containing the tenantId hint.

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
