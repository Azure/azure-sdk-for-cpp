# Release History

## 1.0.0-beta.9 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 1.0.0-beta.8 (2024-04-09)

### Breaking Changes

- Claims Based Security authentication now longer throws a `std::runtime_error`, and instead follows the pattern of the rest of the AMQP library and returns an error.
- Authentication now throws `Azure::Core::Credentials::AuthenticationException` instead of `std::runtime_error`.
- Added `Cancelled` status to `CbsOperationResult` and `ManagementOperationStatus`.

### Bugs Fixed

- [[#5284]](https://github.com/Azure/azure-sdk-for-cpp/issues/5284) [azure-identity][azure-messaging-eventhubs] Impossible to catch exception resulting in SIGABRT signal.
- [[#5297]](https://github.com/Azure/azure-sdk-for-cpp/issues/5297): Enabled multiple simultaneous `ExecuteOperation` calls.
- Fixed crash when Link Detach message is received while link is being destroyed.

### Other Changes

- `std::ostream` inserter for message body no longer prints the body of the message.
- Tidied up the output of the `AmqpMessage` `std::ostream` inserter.
- Added several `std::ostream` inserters.
- Pass numeric values to `std::ostream` inserters by value not by reference.

## 1.0.0-beta.7 (2024-02-02)

### Features Added

- The `Close` method on AMQP Message Sender and Message Receiver now blocks until the client receives a `DETACH` response from the remote node.

### Breaking Changes

- The `Close` method on AMQP Message Sender and Message Receiver now accepts an `Azure::Core::Context` parameter. This parameter is defaulted so this change should not affect existing code.

### Bugs Fixed

- Fixed uAMQP connection channel so that a channel is released when an END performative is received from the remote node instead of when the END performative is sent to the remote node.
- Enabled more than one uAMQP session to be created on a single connection.

## 1.0.0-beta.6 (2024-01-11)

### Features Added

- AMQP Value reference counts are now atomic, this fixes several AMQP related crashes.

### Breaking Changes

- `MessageReceiver` returns a pointer to the received message instead of a copy.

### Bugs Fixed

- Fixed several memory leaks.
- AMQP Link Credits now work as expected.
- Integrated the fix for NVD - CVE-2024-21646.

## 1.0.0-beta.5 (2023-11-07)

### Breaking Changes

- Refactored AMQP headers to isolate internal-only types to "internal" subdirectory in headers.

### Other Changes

- Removed public dependency on azure-uamqp-c to enable local bug fixes.

## 1.0.0-beta.4 (2023-10-05)

### Features Added

- AMQP moved from a polling model to an asynchronous model.
- Added a new `MessageReceiver::TryWaitForIncomingMessage` API which allows callers to "peek" at the contents of 
the incoming message queue, returning an already received message or an empty message if none is available.

### Breaking Changes

- Removed the `QueueSend` API from `MessageSender` because it was not compatible with the new asynchronous model.
- The new asynchronous model requires the user to call `Close()` on the `MessageSender` and `MessageReceiver` 
to ensure operations have stabilized before destroying the object.
- For connection listeners (primarily test scenarios), if you call `Open()` or `Listen()` on a connection, you MUST call `Close()` 
before the connection is destroyed.
- The `Connection::Close()` method no longer requires that the caller provide connection disconnect information.
- The `Session::End()` method no longer requires that the caller provide session disconnect information.
- Several asserts have been added which will force termination of the running application if invariants have not been met.

### Bugs Fixed

- Several fixes related to the new asynchronous model. Ensures that message senders and receivers are always closed, 
and that resources are released.

## 1.0.0-beta.3 (2023-09-07)

### Bugs Fixed

- When a message sender is destroyed, close the underlying AMQP link if it hasn't been closed already.

## 1.0.0-beta.2 (2023-08-04)

### Features Added

- Added `Azure::Core::Amqp::Models::AmqpBinaryData::operator=(std::vector<std::uint8_t> const&)`.
- Added `Azure::Core::Amqp::Models::AmqpMessage::MessageFormat`.
- Collection types (`AmqpArray`, `AmqpMap`, `AmqpList`, `AmqpBinaryData`, `AmqpSymbol` and `AmqpComposite`):
  - Added explicit cast operator to underlying collection type.
  - Added `find()`.
- Rationalized the return code for AMQP MessageSender and MessageReceiver and Management APIs to use AmqpError for error codes.
- Added additional AMQP Error values.

### Breaking Changes

- Renamed `Azure::Core::Amqp::Models::AmqpMessageFormatValue` to `AmqpDefaultMessageFormatValue`.
- Changed the return values for the MessageSender, MessageReceiver and Management APIs.

## 1.0.0-beta.1 (2023-07-06)

### Features Added

- Initial release
