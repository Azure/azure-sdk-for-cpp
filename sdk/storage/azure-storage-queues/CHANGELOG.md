# Release History

## 12.0.0-beta.1 (Unreleased)

### New Features

- Added support for Queue features:
  - QueueServiceClient::ListQueues
  - QueueServiceClient::SetProperties
  - QueueServiceClient::GetProperties
  - QueueServiceClient::GetStatistics
  - QueueServiceClient::CreateQueue
  - QueueServiceClient::DeleteQueue
  - QueueClient::Create
  - QueueClient::CreateIfNotExists
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
