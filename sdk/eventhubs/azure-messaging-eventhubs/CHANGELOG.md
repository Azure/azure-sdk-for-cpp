# Release History

## 1.0.0-beta.15 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 1.0.0-beta.14 (2026-08-18)

### Features Added

- [[#7250]](https://github.com/Azure/azure-sdk-for-cpp/issues/7250) Restored connection-string authentication for `ProducerClient` and `ConsumerClient`, including support for the Event Hubs emulator.
- [[#7295]](https://github.com/Azure/azure-sdk-for-cpp/issues/7295) Connection-string authentication now works on the Rust AMQP backend. `ProducerClient` and `ConsumerClient` no longer throw when the caller passes a connection string.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) `ProducerClient::Send` now builds a new sender on each retry attempt. A failed attempt discards the sender, the session, and the connection for that partition, so the next attempt builds all three again and authenticates with a current token. A send that a link detach ended previously failed for the life of the client.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) `PartitionClient::ReceiveEvents` now attaches a new receiver after a link fault, and it starts after the last event that it gave the caller. So the caller sees no duplicate event and no lost event. A permanent condition, for example `amqp:link:stolen`, still reaches the caller at once. A call that already holds events gives them back and recovers on the next call.

### Bugs Fixed

- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) A teardown of the cached sender no longer runs while another thread sends on that sender. `ProducerClient::Send` gives each attempt a copy of the sender, and a failed attempt on one thread closed the object that a second thread was using. On the Rust AMQP backend that close frees the sender, so the race was a use after free. Each partition now has a guard that lets sends run at the same time and makes a teardown wait for the sends in flight. `ProducerClient::Close` uses the same guard, and it now logs a failed close and continues instead of leaving the other objects open.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) `ProducerClient::CreateBatch` now builds a new sender when it cannot read the maximum message size. The client caches a sender for each partition, and a cached sender holds a link that the service detaches after 30 idle minutes. The size of a batch comes from the attached link, so this call was the first one to touch the dead link, and it threw. The `Send(EventData)` overloads go through this call, so the whole producer failed after an idle period even though `Send` builds a new sender on each attempt.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) Updated producer retries to honor `EventHubsException::IsTransient`, treat empty AMQP error conditions as transient, stop immediately for unknown and known non-transient failures, preserve bounded retries for AMQP runtime failures, and make backoff cancellable through `Azure::Core::Context`. Retry accounting now always performs the initial attempt and treats `MaxRetries` as additional retry attempts.
- [[#7335]](https://github.com/Azure/azure-sdk-for-cpp/issues/7335) The internal properties client can no longer end the process through its destructor. `EventHubsPropertiesClient::Close` marked the client closed only after `ManagementClient::Close` returned, so a close that threw left the client marked open, and it also skipped the session end. The destructor then closed the same client a second time with no try block, and a destructor is `noexcept`, so that second throw would call `std::terminate`. `Close` now marks the client closed before the call, gives each step its own try block, and ends the session on the failing path too. The destructor catches and logs. This is a guard against a throwing close, and not a fix for an observed crash: a live test that left a management link idle for 35 minutes did not make the uAMQP close throw. Release 1.0.0-beta.11 records the same rule in [[#6957]](https://github.com/Azure/azure-sdk-for-cpp/issues/6957).
- [[#7257]](https://github.com/Azure/azure-sdk-for-cpp/issues/7257) Fixed the partition key on a batch envelope. `EventDataBatch::ToAmqpMessage` wrote the `x-opt-partition-key` value to the AMQP delivery-annotations section. The Event Hubs service ignores that section. The batch also built the envelope from the message before the code applied the partition key annotation. A batch with a partition key thus spread across all partitions. The partition key now goes in the message-annotations section, on the batch envelope and on each message in the batch. A batch that sets `EventDataBatchOptions::PartitionKey` now lands on one partition.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) `ProducerClient::Send` no longer waits forever when the connection goes away. A caller that gives no deadline now gets a bound of 60 seconds on the send, and the client treats that timeout as transient, so the next attempt builds a new sender. A close of the sender or of the receiver also comes back when the connection is gone, instead of waiting for a detach that never arrives, and the attach that a rebuild runs next carries the same bound. `PartitionClient::ReceiveEvents` keeps the deadline of the caller, because a receive is a long poll that the caller controls.
- [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254) `Processor::Close` and `ConsumerClient::Close` now finish closing every partition after one AMQP teardown reports a lost connection. A receiver detach can report `IllegalSessionState` after the service resets its connection. That error stopped the cleanup loops, left other connections open, and could end the process when those open connections were destroyed.
- [[#7261]](https://github.com/Azure/azure-sdk-for-cpp/issues/7261) `ProducerClient` and `ConsumerClient` now accept an `eventHub` argument that differs from the connection string `EntityPath` only by ASCII letter case. `GetEventHubName()` returns the connection string spelling. An argument that differs by more than case still throws `std::invalid_argument`.
- The retry log lines now name the attempt. Every attempt wrote the same warning, so a reader could not tell the first attempt from the last one. The line that reports the exhausted retries also names the attempt that the operation reached and the value of `MaxRetries`. The attempt numbers are 1-based.
- `ProducerClient::CreateBatch` now makes one further attempt when it cannot establish the sender and the claims based security open reported `CbsOpenResult::Error`. `Send` runs under a retry policy, but `CreateBatch` did not, so the step that resolves the host, opens the socket, negotiates TLS and runs the claims based security handshake had no second attempt, and a burst after an idle period lost every event whose sender failed to build. Both establishment sites take this path: the first attempt, and the rebuild that follows a failed maximum message size read on a cached link. The bound is one attempt because uAMQP writes the transport reason to its own log and returns no value that carries it. Other results reach the caller at once. The retry builds a new connection, because a failed open can leave the socket in a state that is not closed.

### Other Changes

- Documented that `EventDataBatch::TryAdd` generates a message ID for a copy of the event when the caller did not set one. The caller's own `EventData` object does not change.
- The partition key of the batch now replaces a `x-opt-partition-key` annotation that the caller set on a raw AMQP message. The partition key of the batch is the routing key, so the two values must agree.
- The batch envelope now carries the message ID of the first message in the batch. This includes a message ID that `TryAdd` generated.
- `ReceivedEventData` now logs a warning when the `x-opt-offset` annotation is not a string. The service always sends a string, so a different type is a service contract break. The client discarded that value with no record before.

## 1.0.0-beta.13 (2026-06-17)

### Bugs Fixed

- [[#7130]](https://github.com/Azure/azure-sdk-for-cpp/issues/7130) Fixed `RetryOperation::Execute` silently swallowing the final exception when every retry attempt threw, which caused `ProducerClient::Send` to return without delivering the batch and without surfacing the underlying failure. The last exception is now rethrown when retries are exhausted, and `ProducerClient::Send` throws if `Execute` ever reports failure as a defense in depth.

## 1.0.0-beta.11 (2026-05-14)

### Breaking Changes

- Changed the `EventData::CorrelationId` and `EventData::MessageId` fields from `Azure::Nullable<AmqpValue>` to `AmqpValue` since `AmqpValue` embeds the concept of nullability already.
- Removed the connection string authentication mechanism from the EventHubs clients.

### Bugs Fixed

- [[#6957]](https://github.com/Azure/azure-sdk-for-cpp/issues/6957) Do not throw exceptions past destructors.

## 1.0.0-beta.10 (2024-11-01)

### Bugs Fixed

- [[#6064]](https://github.com/Azure/azure-sdk-for-cpp/issues/6064) Utilize new telemetry features from Azure Core.

## 1.0.0-beta.9 (2024-06-11)

### Bugs Fixed

- Fixed eventhub connection properties to better align with the names used by other Azure SDKs.

## 1.0.0-beta.8 (2024-05-07)

### Features Added

- Added support for the EventHubs emulator.

## 1.0.0-beta.7 (2024-04-09)

### Breaking Changes

- Renamed `EventDataBatch::TryAddMessage` to `EventDataBatch::TryAdd` to better reflect the method's use.

### Bugs Fixed

- Fixed [[#5297]](https://github.com/Azure/azure-sdk-for-cpp/issues/5297). The actual fix for this was to use a single management client per connection.

## 1.0.0-beta.6 (2024-02-06)

### Breaking Changes

- `PartitionClient::Close` now takes an optional `Azure::Core::Context` parameter to reflect that it now waits until the `Close` verb has fully completed. This should not affect existing clients.
- `ProcessorPartitionClient::Close` now takes an optional `Azure::Core::Context` parameter to reflect that it now waits until the `Close` verb has fully completed. This should not affect existing clients.

## 1.0.0-beta.5 (2024-01-11)

### Breaking Changes

- EventHub `ConsumerClient` and `ProcessorClient` objects now return pointers to `EventData` objects instead of `EventData` objects by value.

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
