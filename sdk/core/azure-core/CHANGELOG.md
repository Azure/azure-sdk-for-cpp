# Release History

## 1.0.0-beta.4 (Unreleased)


## 1.0.0-beta.3 (2020-11-11)

### New Features
- Added `strings.hpp` with `Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual` and `Azure::Core::Strings::ToLower`.
- Added `GetPort()` to `Url`.
- Added `TransportPolicyOptions`.
- Added `TelemetryPolicyOptions`.
- Added `RequestFailedException` deriving from `std::runtime_error`.
- Added `CurlTransportOptions` for the `CurlTransport`.
- Added `DateTime` supporting dates since 1601.
- Added `OperationCanceledException`.
- Added `Encode` and `Decode` to `Url`.

### Breaking Changes

- Removed `azure.hpp`.
- Removed macro `AZURE_UNREFERENCED_PARAMETER`.
- Bump CMake version from 3.12 to 3.13.
- Bump libcurl version from 7.4 to 7.44.
- Moved `ClientSecretCredential` and `EnvironmentCredential` to the Identity library.
- `Url` class changes:
  - `AppendPath` now does not encode the input by default.
  - Signature updated for `SetHost`, `SetPath` and `AppendPath`.
  - Removed `SetFragment`.
  - Renamed `AppendQueries` to `AppendQueryParameters`.
  - Renamed `AppendQuery` to `AppendQueryParameter`.
  - Renamed `RemoveQuery` to `RemoveQueryParameter`.
  - Renamed `GetQuery` to `GetQueryParameters`.

### Bug Fixes

- Prevent pipeline of length zero to be created.
- Avoid re-using a connection when a request to upload data fails while using the `CurlTransport`.
- Add entropy to `Uuid` generation.

### Other changes and Improvements

- Add high-level and simplified core.hpp file for simpler include experience for customers.
- Add code coverage using gcov with gcc.
- Update SDK-defined exception types to be classes instead of structs.
- Updated `TransportException` and `InvalidHeaderException` to derive from `RequestFailedException`.
- Vcpkg dependency version updated to 2020.11.
- Make libcurl network requests cancelable by Context::Cancel().
- Validate HTTP headers for invalid characters.
- Calling `Cancel()` from context now throws `OperationCanceledException`.

## 1.0.0-beta.2 (2020-10-09)

### Breaking Changes

- Throw Azure::Http::TransportException if creating new connection fails.
- Response objects store Nullable\<T\>.

### Bug Fixes

- Switched to a more stable wait on sockets to address connection timeouts.
- Replace `Nullable(const T&)` with `Nullable(T)` to avoid extra copy when initialized with an rvalue.

### Other changes and Improvements

- Improved performance on windows when using libcurl.
- Pinned the version of package dependencies.
- Added NOTICE.txt for 3rd party dependencies.

## 1.0.0-beta.1 (2020-09-09)

- Initial release
