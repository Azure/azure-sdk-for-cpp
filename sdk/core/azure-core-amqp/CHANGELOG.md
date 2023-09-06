# Release History

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
