# Release History

## 1.0.0-beta.3 (Unreleased)

### Breaking Changes

- `Azure::Core::Http::Url::AppendPath` now does not encode the input by default.

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
