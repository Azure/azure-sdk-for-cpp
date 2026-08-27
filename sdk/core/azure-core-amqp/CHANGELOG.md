# Release History

## 1.0.0-beta.13 (Unreleased)

### Features Added

- The Rust AMQP backend now generates SAS tokens from a shared access key. It sends CBS put-token requests with the `servicebus.windows.net:sastoken` token type.
- Added support for the AMQP Decimal types (AmqpDecimal128, AmqpDecimal64, and AmqpDecimal32).

### Breaking Changes

### Bugs Fixed

- The uAMQP message sender now encodes delivery annotations, message annotations, and the footer as described sections, so a uAMQP receiver can decode a message that carries them. Before, the sender wrote the bare maps, and the receiving link failed with "Error decoding message" and went to the error state. [[#7376]](https://github.com/Azure/azure-sdk-for-cpp/issues/7376)
- On the uAMQP transport, `MessageSender::Open` and `MessageReceiver::Open` now throw `_detail::CbsPutTokenFailedException` when the service rejects the CBS put-token. The type derives from `std::runtime_error` and carries the original `AuthenticationException`, which `RethrowOriginal()` throws again. The Event Hubs clients use the type to tell a rejected put-token from a credential failure. `ManagementClient::Open` and `ManagementClient::ExecuteOperation` still throw `AuthenticationException`. [[#7376]](https://github.com/Azure/azure-sdk-for-cpp/issues/7376)
- uAMQP pollable registration and removal no longer block each other while a poll is in flight. The
  polling registry now waits on completion notifications, and sender, receiver, and link setup and
  teardown do not hold connection locks across registry operations. The polling thread now sleeps
  when no pollable is registered. It spun on one core after a process closed its last connection.
  [[#7370]](https://github.com/Azure/azure-sdk-for-cpp/issues/7370)
- uAMQP now tears down unsettled sends without leaving late dispositions with freed callback state. Sender Open cleanup no longer deadlocks with link polling. Sender Open, sender Close, and receiver Close report caller cancellation separately from synthetic timeout. [[#7350]](https://github.com/Azure/azure-sdk-for-cpp/issues/7350)
- A close that fails now leaves the object closed. `ManagementClient`, `MessageSender`, and `MessageReceiver` kept the open flag when the close threw, and the destructor then stopped the process. [[#7323]](https://github.com/Azure/azure-sdk-for-cpp/issues/7323)
- The connection no longer returns a cached CBS token that is at or near its expiry. It authenticates the audience again instead. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The connection now replaces each cached CBS token before that token expires, so a client that runs for longer than one token lifetime keeps working. This refresh applies to the uAMQP transport. Without this refresh, a send that gets the `amqp:unauthorized-access` condition stops at the first attempt, because the Event Hubs producer treats that condition as not transient. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The token refresh thread now wakes when a caller authenticates a new audience. Before, its wait tested the stop flag alone, so it slept on a deadline that it computed before that audience existed, and it left the new token unexamined for up to one minute. A token with a lifetime shorter than that delay could expire before its first refresh. This change applies to the uAMQP transport. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The management client now gets the current token for each operation and puts it in the `security_token` application property. Before, it sent the token that it got when it opened. A management client lives as long as the client that owns it, so that token expired while the client continued to send it. The service reads that property: an operation that carries a token that is not valid gets the `amqp:not-allowed` condition with status code 500, and Event Hubs does not count that condition as transient. This change applies to the uAMQP transport. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- An AMQP operation that the caller did not bound now gets a default deadline of 60 seconds, and a connection that goes to the error or the end state now wakes every operation that waits on it. A send on a connection that the service dropped waited forever on both transports: uAMQP stops the poll of the connection in those two states, and the Rust runtime blocks the calling thread with no bound. A receive keeps the deadline of the caller, because a receive is a long poll that the caller controls. The link attach and the link detach carry the same bound on both backends, because a rebuild runs both of them against the connection that died. The uAMQP sender attach also registers for the connection wake, so a connection that dies during a rebuild attach ends that wait. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The Rust connection now marks itself closed before it calls the native close. A close that reached its new bound returned an error and threw with the open flag still set. The Event Hubs recover path catches that exception and then destroys the connection, and the destructor stopped the process because the flag stayed set. `ManagementClient`, `MessageSender`, and `MessageReceiver` already follow this rule. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- Each uAMQP send now owns its completion state. A send that reached its deadline left the uAMQP operation in flight, because a cancel can make uAMQP call the completion handler two times. That operation later pushed its result into the one queue that the sender shared between every send, so the next send took the stale result as its own. A retry could thus report the outcome of the attempt before it. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The uAMQP receiver now registers its connection waiter under the connection lock. That waiter reads the saved error of the link on the calling thread, while the polling thread writes the same field, so the read was a data race. The sender already used this lock. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- A cancelled caller no longer stops a teardown on the Rust transport. A close took the time that remained for the caller as its bound. A cancelled context leaves no time, so the bound was zero and every close failed at once with an `Elapsed` error. The caller then stopped before it closed the connection, and the destructor of that connection stopped the process. A teardown now takes the default bound when the deadline of the caller already passed. It keeps a caller deadline that has time left. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- The uAMQP sender now holds every send that waits, not one send at a time. `ProducerClient::Send` lets sends to one partition run together, so a second send replaced the first in the old single slot. uAMQP drains each in-flight send before it reports the error state, so no send was stranded in practice. This change keeps the error wake correct if that order changes. [[#7254]](https://github.com/Azure/azure-sdk-for-cpp/issues/7254)
- `ServiceBusSasConnectionStringCredential` no longer throws when the connection string `EntityPath` and the entity path argument differ only by ASCII letter case. The comparison folds ASCII A-Z only. It is not the culture-aware `InvariantCultureIgnoreCase` comparison that .NET uses. `GetEntityPath()` returns the connection string spelling. .NET returns the explicit argument instead, so this behavior deviates from .NET. [[#7261]](https://github.com/Azure/azure-sdk-for-cpp/issues/7261)
- A claims based security open that fails now names the result, the audience, and the function that asked for the token. The exception said "Could not open Claims Based Security object." and nothing more, so a reader could not tell which audience failed or whether the failure came from the first authentication or from the token refresh. The connection also writes a warning that adds the token type and the token expiry. Neither the exception nor the warning holds the token.
- The claims based security warning also names the connection instance, the host, the connection state, and the time that the open took. A reader can now separate a client fault from a service fault: an open that fails in about no time, on a connection that is already in the `End` state, never reached the service, and a repeated instance number shows that the client did not build a new connection. The connection state is now atomic, because the uAMQP polling thread writes it while another thread reads it for this line.
- The uAMQP connection now names the reason that the service gives when it closes the connection. uAMQP reads that reason from the `close` performative and offers it through one subscription, and the connection did not take that subscription, so the reason reached no log. A client that the service closed after an idle period saw only the failures that followed. A transport I/O error now writes a separate warning with the connection instance, host, and state. The uAMQP callback provides no error payload, so the warning says that no details are available.
- The uAMQP message sender and message receiver now write the detach condition and the description each time the service detaches a link, and the line no longer needs the AMQP trace option. Each one also writes the line for a detach that arrives during the open. Before, that detach was silent, and the open reported a generic error that did not name the condition. The `$cbs` links take that path when the service rejects the attach.
- A claims based security open that fails now throws `CbsOpenFailedException`, which carries the `CbsOpenResult`. The three failures need different handling: `Error` reached the transport and may be retried, while `Cancelled` is the caller's own cancellation or deadline and `Invalid` is a state error. The result was previously readable only by matching the message text, so a reword would have changed caller behavior with no compiler error. The type derives from `std::runtime_error` and carries the same message, so existing handlers keep working. The Rust backend reports every open failure by throwing rather than by returning a result, so those throws are classified at the shared call site and carry the same type.
- The uAMQP management client now closes the message sender when the message receiver fails to open. Two handlers returned a status without that close, and a message sender that stays open stops the process in its own destructor.
- The uAMQP management client now names the management node and the open status in the lines that it writes when an open fails, and it keeps the text of the exception that ended the open. The message sender open failure moved from the Error level to the Warning level, because that call reports the failure to its caller.

### Other Changes

## 1.0.0-beta.12 (2026-05-14)

### Features Added

Rust based AMQP library is now available for use in the Azure SDK for C++. This replaces the uAMQP library with a library based on the azure_core_amqp Rust crate.

### Breaking Changes

Updated `MessageProperties` to remove `Azure::Nullable` from the types which are an `AmqpValue` because the `AmqpValue` already embeds the concept of nullability.

## 1.0.0-beta.11 (2024-09-12)

### Bugs Fixed

- Updated vendored copy of uAMQP to reflect upstream changes.

## 1.0.0-beta.10 (2024-06-06)

### Bugs Fixed

- [[#5536]](https://github.com/Azure/azure-sdk-for-cpp/issues/5536) Fixed use-after free in MessageSender and MessageReceiver when opening a connection.

## 1.0.0-beta.9 (2024-05-06)

### Bugs Fixed

- Fixed a potential deadlock where a message receiver Open call could block indefinitely when adding the Link to the Pollables.

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
