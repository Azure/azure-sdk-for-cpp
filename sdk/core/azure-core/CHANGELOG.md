# Release History

## 1.0.0-beta.3 (Unreleased)

### Breaking Changes

- `Azure::Core::Http::Url::AppendPath` now does not encode the input by default.
- Removed `azure.hpp`.

### New Features
- Added `strings.hpp` with `Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual` and `Azure::Core::Strings::ToLower`.
- Added `OperationCanceledException`.

### Other changes and Improvements

- Add high-level and simplified core.hpp file for simpler include experience for customers.
- Add code coverage using gcov with gcc.
- Update SDK-defined exception types to be classes instead of structs.

### Bug Fixes

- Prevent pipeline of length zero to be created.
- Avoid re-using a connection when an uploading request fail when using CurlTransport.

### New Features

- Add `RequestFailException` deriving from `std::runtime_error`.

### Other changes and Improvements

- Updated `TransportException` and `InvalidHeaderException` to derive from `RequestFailedException`.

## 1.0.0-beta.2 (2020-10-09)

### Breaking Changes

- Throw Azure::Http::TransportException if creating new connection fails.
- Response objects store Nullable\<T\>.
- Calling `Cancel()` from context now throws `OperationCanceledException`.

### Bug Fixes

- Switched to a more stable wait on sockets to address connection timeouts.
- Replace `Nullable(const T&)` with `Nullable(T)` to avoid extra copy when initialized with an rvalue.

### Other changes and Improvements

- Improved performance on windows when using libcurl.
- Pinned the version of package dependencies.
- Added NOTICE.txt for 3rd party dependencies.

## 1.0.0-beta.1 (2020-09-09)

- Initial release
