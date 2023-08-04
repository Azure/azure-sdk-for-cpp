# Release History

## 1.0.0-beta.2 (2023-08-04)

### Features Added

- Added `Azure::Core::Amqp::Models::AmqpBinaryData::operator=(std::vector<std::uint8_t> const&)`.
- Added `Azure::Core::Amqp::Models::AmqpMessage::MessageFormat`.
- Collection types (`AmqpArray`, `AmqpMap`, `AmqpList`, `AmqpBinaryData`, `AmqpSymbol` and `AmqpComposite`):
  - Added explicit cast operator to underlying collection type.
  - Added `find()`.

### Breaking Changes

- Renamed `Azure::Core::Amqp::Models::AmqpMessageFormatValue` to `AmqpDefaultMessageFormatValue`.

## 1.0.0-beta.1 (2023-07-06)

### Features Added

- Initial release
