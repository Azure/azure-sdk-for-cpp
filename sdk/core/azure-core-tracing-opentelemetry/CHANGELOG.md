# Release History

## 1.0.0-beta.4 (2023-02-07)

### Features Added

- Aligned OpenTelemetry tracing infrastructure with OpenTelemetry 1.17.0 conventions for use with Azure Monitor.

### Other Changes
- Updated test collateral to reflect Azure Core tracing changes.
- Suppress several warnings from opentelemetry-cpp package.

## 1.0.0-beta.3 (2022-08-04)

### Other Changes

- Removed hard dependency on `opentelemetry-cpp` package version.

## 1.0.0-beta.2 (2022-06-30)

### Breaking Changes

- The `Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider` type can only be instantiated via a factory method: `OpenTelemetryProvider::Create()`.

### Other Changes

- Removed `_internal` APIs from the public API surface. Also removed most of the `_internal` APIs from the public `opentelemetry.hpp` headers.

## 1.0.0-beta.1 (2022-06-07)

### Features Added

- Initial release
