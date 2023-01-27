# _Release History

## _1.0.0-beta.4 (Unreleased)

### _Features Added

### _Breaking Changes

### _Bugs Fixed

### _Other Changes

## _1.0.0-beta.3 (2022-08-04)

### _Other Changes

- Removed hard dependency on `opentelemetry-cpp` package version.

## _1.0.0-beta.2 (2022-06-30)

### _Breaking Changes

- The `Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider` type can only be instantiated via a factory method: `OpenTelemetryProvider::Create()`.

### _Other Changes

- Removed `_internal` APIs from the public API surface. Also removed most of the `_internal` APIs from the public `opentelemetry.hpp` headers.

## _1.0.0-beta.1 (2022-06-07)

### _Features Added

- Initial release
