# Release History

## 1.0.0-beta.7 (Unreleased)

### New Features

- Added `HttpPolicyOrder` for adding custom Http policies to sdk clients.

### Breaking Changes

- Removed `Azure::Core::Http::HttpPipeline` by making it internal, used only within the SDK.
- Split `Azure::Core::RequestConditions` into `Azure::Core::MatchConditions` and `Azure::Core::ModifiedConditions`.
- Removed `TransportKind` enum from `Azure::Core::Http`.
- Renamed `NoRevoke` to `EnableCertificateRevocationListCheck` for `Azure::Core::Http::CurlTransportSSLOptions`.
- Renamed `GetString()` to `ToString()` in `Azure::Core::DateTime`.
- Renamed `GetUuidString()` to `ToString()` in `Azure::Core::Uuid`.
- Moved `BodyStream` and its derived types from `Azure::Core::Http` namespace to `Azure::IO`, and moved the `body_stream.hpp` header from `azure/core/http` to `azure/core/io`.
- Moved `NullBodyStream` to internal usage only. It is not meant for public use.
- Removed `LimitBodyStream`.
- Renamed `AddHeader()` from `Request` and `RawResponse` to `SetHeader()`.
- Introduced `Azure::Core::CaseInsensitiveMap` which is now used to store headers in `Azure::Core::Http::Request` and `Azure::Core::Http::RawResponse`.
- Renamed `TransportPolicyOptions` to `TransportOptions`.
- Renamed `TelemetryPolicyOptions` to `TelemetryOptions`.
- Renamed `ValuePolicyOptions` to `ValueOptions`.
- Removed `StartTry()` from `Azure::Core::Http::Request`.
- Move `Azure::Core::Context` to be the last parameter for consistency, instead of first in various azure-core types. For example:
  - `BodyStream::Read(uint8_t* buffer, int64_t count, Azure::Core::Context const& context)`
  - `BodyStream::ReadToEnd(BodyStream& body, Azure::Core::Context const& context)`
  - `HttpPolicy::Send(Request& request, NextHttpPolicy policy, Azure::Core::Context const& context)`
  - `Operation<T>::PollUntilDone(std::chrono::milliseconds period, Azure::Core::Context& context)`
  - `TokenCredential::GetToken(Http::TokenRequestOptions const& tokenRequestOptions, Azure::Core::Context const& context)`
- Changed type of `Azure::Core::Http::RetryOptions::StatusCodes` from `std::vector` to `std::set`.
- Renamed `Azure::Core::Http::LoggingPolicy` to `LogPolicy`.
- Introduced `Azure::Core::Http::LogOptions`, a mandatory parameter for `LogPolicy` construction. Entities that are not specified in the allow lists are hidden in the log.
- Moved `Azure::Core::Logging` namespace entities to `Azure::Core::Logger` class.
- Removed `Azure::Core::DateTime::GetRfc3339String()`: `Azure::Core::DateTime::ToString()` was extended to provide the same functionality.

### Bug Fixes

- Make sure to rewind the body stream at the start of each request retry attempt, including the first.

## 1.0.0-beta.6 (2021-02-09)

### New Features

- Added support for HTTP conditional requests `MatchConditions` and `RequestConditions`.
- Added the `Hash` base class and MD5 hashing APIs to the `Azure::Core::Cryptography` namespace available from `azure/core/cryptography/hash.hpp`.

### Breaking Changes

- Removed `Context::CancelWhen()`.
- Removed `LogClassification` and related functionality, added `LogLevel` instead.

### Bug Fixes

- Fixed computation of the token expiration time in `BearerTokenAuthenticationPolicy`.
- Fixed compilation dependency issue for MacOS when consuming the SDK from VcPkg.
- Fixed support for sending requests to endpoints with a custom port within the url on Windows when using the WinHttp transport.

## 1.0.0-beta.5 (2021-02-02)

### New Features

- Added support for HTTP validators `ETag`.

### Breaking Changes

- Make `ToLower()` and `LocaleInvariantCaseInsensitiveEqual()` internal by moving them from `Azure::Core::Strings` to `Azure::Core::Internal::Strings`.
- `BearerTokenAuthenticationPolicy` constructor takes `TokenRequestOptions` struct instead of scopes vector. `TokenRequestOptions` struct has scopes vector as data member.
- `TokenCredential::GetToken()` takes `TokenRequestOptions` instead of scopes vector.

### Bug Fixes

- Fixed the parsing of the last chunk of a chunked response when using the curl transport adapter.
- Fixed reading the value from `retry-after` header in `RetryPolicy`.
- Fix link errors when producing a DLL and add UWP compilation support.
- Do not pass a placeholder user-agent string as a fallback when using WinHttp.
- Initialize local variables in implementation to fix warning within release builds on Linux.

## 1.0.0-beta.4 (2021-01-13)

### New Features

- Added a WinHTTP-based `HttpTransport` called `WinHttpTransport` and use that as the default `TransportPolicyOptions.Transport` on Windows when sending and receiving requests and responses over the wire.
- Added `Range` type to `Azure::Core::Http` namespace.
- Added support for long-running operations with `Operation<T>`.
- Added support for setting a custom transport adapter by implementing the method `std::shared_ptr<HttpTransport> ::AzureSdkGetCustomHttpTransport()`.
- Added interoperability between `std::chrono::system_clock` types and `DateTime`.
- Added default constructor to `DateTime` and support for dates since 0001.
- Added Base64 encoding and decoding utility APIs to the `Azure::Core` namespace available from `azure/core/base64.hpp`.
- Added `Http::Response<void>` template specialization.
- Added `GetHeadersAsString()` on the `Azure::Core::Http::Request` class.
- Added a `platform.hpp` header file for defining commonly used OS-specific `#define` constants.
- Added `IsCancelled()` on the `Azure::Core::Context` class.

### Breaking Changes

- Removed `DateTime::operator Duration()`, `DateTime::Duration` typedef and `DateTime::Now()`.
- Moved `Azure::Core::BearerTokenAuthenticationPolicy`, defined in `azure/core/credentials.hpp` to `Azure::Core::Http` namespace in `azure/core/http/policy.hpp` header.
- Changed type of `Token::ExpiresOn` to `DateTime`.
- Renamed exception `OperationCanceledException` to `OperationCancelledException` and `ThrowIfCanceled()` to `ThrowIfCancelled()` on the `Context` class.
- Moved `Azure::Core::Version`, defined in `azure/core/version.hpp` to the `Azure::Core::Details` namespace.
- Changed `Azure::Core::AuthenticationException` to derive from `std::exception` instead of `std::runtime_error`.
- Changed the `BodyStream::Read` API from being a pure virtual function to non-virtual.
- Removed `CurlConnection`, `CurlConnectionPool`, `CurlSession`, and `CurlNetworkConnection` by making headers meant as implementation, private.
- Removed option `AllowBeast` from `CurlTransportSSLOptions` in `CurlTransportOptions`.
- Changed default option `NoRevoke` from `CurlTransportSSLOptions` for the `CurlTransportOptions` to `true`. This disables the revocation list checking by default.

### Bug Fixes

- Fixed the Curl transport adapter connection pooling when setting options.
- Fixed setting up the default transport adapter.
- Fixed linker error of missing pthread on Linux.
- Initialize class data members to avoid MSVC warning.
- Throw `Azure::Core::Http::TransportException` if `curl_easy_init()` returns a null handle.

### Other changes and Improvements

- Added support for distributing the C++ SDK as a source package via vcpkg.

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
