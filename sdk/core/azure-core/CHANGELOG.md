# Release History

## 1.4.0-beta.1 (Unreleased)

### Features Added

### Breaking Changes

- `azure/core/azure_assert.hpp` header is moved to internal. `AzureNoReturnPath()` function is removed from global namespace. Associated macros, such as `AZURE_ASSERT` are renamed to indicate that they are internal. If your code was using the `AZURE_ASSERT` macro, consider using the standard library's `assert` as an alternative.

### Bugs Fixed

### Other Changes

## 1.3.1 (2021-11-05)

### Bugs Fixed

- Fixed linking error when Azure SDK is being built as DLL.

## 1.3.0 (2021-11-04)

### Features Added

- Add `NoSignal` option to the `CurlTransportAdapter`.
- Add `ConnectionTimeout` option to the `CurlTransportAdapter`.

### Bugs Fixed

[2848](https://github.com/Azure/azure-sdk-for-cpp/issues/2848) Update the libcurl transport adapter to work with HTTP/1.1 only.

### Other Changes

- Updated `base64` implementation to remove external dependency.
- Updated `Uuid` implementation for Linux to remove external dependency.

## 1.2.1 (2021-09-02)

### Bugs Fixed

- [2785](https://github.com/Azure/azure-sdk-for-cpp/issues/2785) Fix to build on g++ 5.5.

### Other Changes

- Fixed compilation error on POSIX platforms where OpenSSL was not available.
- Support CMake version 3.12

## 1.2.0 (2021-08-05)

### Features Added

- Added `Azure::Core::IO::ProgressBodyStream` type that wraps an existing BodyStream based type stream and reports progress via callback when the stream position is updated.

### Bugs Fixed

- [2647](https://github.com/Azure/azure-sdk-for-cpp/issues/2647) Make the curl transport adapter to check the connection close header.
- [2474](https://github.com/Azure/azure-sdk-for-cpp/issues/2474) Fix compiling with MSVC and `/analyze`.
- Make WinHTTP transport adapter to NOT use SSL/TLS for unsecured HTTP connections.

### Other Changes

- Updated source code to build with Clang 11. (A community contribution, courtesy of _[davidchisnall](https://github.com/davidchisnall)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- David Chisnall _([GitHub](https://github.com/davidchisnall))_

## 1.1.0 (2021-07-02)

### Bugs Fixed

- Fixed a memory leak issue in `Base64Encode()`. (A community contribution, courtesy of _[jorgen](https://github.com/jorgen)_)

### Other Changes

- Made internal-only changes to support the Azure Key Vault client library.

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- Jorgen Lind _([GitHub](https://github.com/jorgen))_

## 1.0.0 (2021-06-04)

### Bug Fixes

- Make `RequestFailedException` copiable so it can be propagated across thread.
- By default, add `x-ms-request-id` header to the allow list of headers to log.

## 1.0.0-beta.9 (2021-05-18)

### New Features

- Added `Azure::PagedResponse<T>`.

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Removed `Context::GetApplicationContext()` in favor of a new static data member `Context::ApplicationContext`.
- Renamed `Request::IsDownloadViaStream()` to `ShouldBufferResponse()`.
- Removed the `Azure::Core::Http::Request` ctor overload that takes both a `bodyStream` and a `bufferedDownload` boolean since it is not useful.
- Changed integer size parameters for buffers from `int64_t` to `size_t` in various places such as `Azure::Core::IO::BodyStream::Read()` APIs.
- Removed the `Azure::Core::Diagnostics::Logger::Listener` typedef.

### Bug Fixes

- Do not re-use a libcurl connection to same host but different port.
- Fixed curl transport issue to avoid crash at exit when curl connection pool cleanup thread is running.
- Ensure uniqueness of `Azure::Core::Uuid` on POSIX platforms.

### Other Changes and Improvements

- Modified precondition validation of function arguments to now result in assert failures rather than throwing an exception.
- Remove exposing windows.h header from our public headers.
- Improved performance of the WinHTTP transport layer on Windows for uploading large payloads.

## 1.0.0-beta.8 (2021-04-07)

### New Features

- Added `Azure::Core::Url::GetScheme()`.
- Added `Azure::Core::Context::TryGetValue()`.
- Added `Azure::Core::Context::GetDeadline()`.
- Added `Azure::Core::Credentials::TokenCredentialOptions`.
- Added useful fields to the `Azure::Core::RequestFailedException` class such as `StatusCode`, `ReasonPhrase`, and the `RawResponse`, for better diagnosis of errors.

### Breaking Changes

- Simplified the `Response<T>` API surface to expose two public fields with direct access: `T Value` and a `unique_ptr` to an `Azure::Core::Http::RawResponse`.
- Renamed `Azure::Nullable<T>::GetValue()` to `Value()`.
- Removed from `Azure::Core::Http::Request`:
  - `SetUploadChunkSize()`.
  - `GetHTTPMessagePreBody()`.
  - `GetUploadChunkSize()`.
  - `GetHeadersAsString()`.
- Changes to `Azure::Core::Http::RawResponse`:
  - Removed `SetHeader(std::string const& header)`
  - Removed `SetHeader(uint8_t const* const first, uint8_t const* const last)`.
  - Removed `GetMajorVersion()`.
  - Removed `GetMinorVersion()`.
  - Renamed `GetBodyStream()` to `ExtractBodyStream()`.
- Changes to `Azure::Core::Context`:
  - Removed `Get()` and `HasKey()` in favor of a new method `TryGetValue()`.
  - Changed input parameter type of `WithDeadline()` to `Azure::DateTime`.
- Removed `Azure::Core::PackageVersion`.
- Removed from `Azure::Core::Http::Policies` namespace: `HttpPolicyOrder`, `TransportPolicy`, `RetryPolicy`, `RequestIdPolicy`, `TelemetryPolicy`, `BearerTokenAuthenticationPolicy`, `LogPolicy`.
- Removed `AppendQueryParameters()`, `GetUrlWithoutQuery()` and `GetUrlAuthorityWithScheme()` from `Azure::Core::Url`.
- Changed the `Azure::Core::Http::HttpMethod` regular enum into an extensible enum class and removed the `HttpMethodToString()` helper method.
- Introduced `Azure::Core::Context::Key` class which takes place of `std::string` used for `Azure::Core::Context` keys previously.
- Changed the casing of `SSL` in API names to `Ssl`:
  - Renamed type `Azure::Core::Http::CurlTransportSSLOptions` to `CurlTransportSslOptions`.
  - Renamed member `Azure::Core::Http::CurlTransportOptions::SSLOptions` to `SslOptions`.
  - Renamed member `Azure::Core::Http::CurlTransportOptions::SSLVerifyPeer` to `SslVerifyPeer`.

### Other Changes and Improvements

- Moved `Azure::Core::Http::Request` to its own header file from `http.hpp` to `inc/azure/core/http/raw_response.hpp`.
- Moved `Azure::Core::Http::HttpStatusCode` to its own header file from `http.hpp` to `inc/azure/core/http/http_status_code.hpp`.

## 1.0.0-beta.7 (2021-03-11)

### New Features

- Added `HttpPolicyOrder` for adding custom Http policies to SDK clients.
- Added `Azure::Core::Operation<T>::GetRawResponse()`.
- Added `Azure::Core::PackageVersion`.
- Added support for logging to console when `AZURE_LOG_LEVEL` environment variable is set.

### Breaking Changes

- Changes to `Azure::Core` namespace:
  - Removed `Response<void>`, `ValueBase`, and `ContextValue`.
  - Removed `Context::operator[]`, `Get()` introduced instead.
  - Renamed `Uuid::GetUuidString()` to `ToString()`.
  - Changed return type of `Operation<T>::Poll()` from `std::unique_ptr<RawResponse>` to `RawResponse const&`.
  - Moved `GetApplicationContext()` to `Context::GetApplicationContext()`
  - Moved the `Base64Encode()` and `Base64Decode()` functions to be static members of a `Convert` class.
  - Moved `Logging` namespace entities to `Diagnostics::Logger` class.
  - Moved `AccessToken`, `TokenCredential`, and `AuthenticationException` to `Credentials` namespace.
  - Moved `Context` to be the last parameter for consistency, instead of first in various azure-core types. For example:
    - `BodyStream::Read(uint8_t* buffer, int64_t count, Context const& context)`
    - `BodyStream::ReadToEnd(BodyStream& body, Context const& context)`
    - `HttpPolicy::Send(Request& request, NextHttpPolicy policy, Context const& context)`
    - `Operation<T>::PollUntilDone(std::chrono::milliseconds period, Context& context)`
    - `TokenCredential::GetToken(Http::TokenRequestOptions const& tokenRequestOptions, Context const& context)`
  - Moved from `Azure::Core` to `Azure` namespace:
    - `Response<T>`, `ETag`, and `Nullable<T>`.
    - Split `RequestConditions` into `MatchConditions` and `ModifiedConditions`.
    - Renamed `DateTime::GetString()` to `ToString()`, and removed `DateTime::GetRfc3339String()`.
- Changes to `Azure::Core::Http` namespace:
  - Removed `HttpPipeline`, `TransportKind`, `NullBodyStream`, and `LimitBodyStream`.
  - Removed `Request::StartTry()`.
  - Removed `InvalidHeaderException` and throw `std::invalid_argument` if the user provides invalid header arguments.
  - Renamed `CurlTransportSSLOptions::NoRevoke` to `EnableCertificateRevocationListCheck`.
  - Renamed `Range` to `HttpRange`.
  - Renamed `TokenRequestOptions` to `TokenRequestContext`, and moved it to `Azure::Core::Credentials` namespace.
  - Moved `Url` to `Azure::Core` namespace.
  - `Request` and `RawResponse`:
    - Renamed `AddHeader()` to `SetHeader()`.
    - Introduced `Azure::Core::CaseInsensitiveMap` which is now used to store headers.
  - `BodyStream` and the types that derive from it:
    - Moved to `Azure::Core::IO` namespace.
    - Changed the static methods `BodyStream::ReadToCount()` and `BodyStream::ReadToEnd()` into instance methods.
    - Changed the constructor of `FileBodyStream` to accept a file name directly and take ownership of opening/closing the file, instead of accepting a file descriptor, offset, and length.
  - HTTP policies and their options:
    - Moved to `Policies` namespace.
    - Renamed `TransportPolicyOptions` to `TransportOptions`.
    - Renamed `TelemetryPolicyOptions` to `TelemetryOptions`.
    - Changed type of `RetryOptions::StatusCodes` from `std::vector` to `std::set`.
    - Renamed `LoggingPolicy` to `LogPolicy`, and introduced `LogOptions` as mandatory parameter for the constructor.
- Moved header files:
  - Renamed `azure/core/credentials.hpp` to `azure/core/credentials/credentials.hpp`.
  - Renamed `azure/core/logger.hpp` to `azure/core/diagnostics/logger.hpp`.
  - Renamed `azure/core/http/body_stream.hpp` to `azure/core/io/body_stream.hpp`.
  - Renamed `azure/core/http/policy.hpp` to `azure/core/http/policies/policy.hpp`.
  - Renamed `azure/core/http/curl/curl.hpp` to `azure/core/http/curl_transport.hpp`.
  - Renamed `azure/core/http/winhttp/win_http_client.hpp` to `azure/core/http/win_http_transport.hpp`.

### Bug Fixes

- Make sure to rewind the body stream at the start of each request retry attempt, including the first.
- Connection pool resets when all connections are closed.
- Fix `Azure::Context` to support `std::unique_ptr`.
- Throw `std::runtime_error` from `Response<T>::GetRawResponse()` if the response was already extracted.

## 1.0.0-beta.6 (2021-02-09)

### New Features

- Added support for HTTP conditional requests `MatchConditions` and `RequestConditions`.
- Added the `Hash` base class and MD5 hashing APIs to the `Azure::Core::Cryptography` namespace available from `azure/core/cryptography/hash.hpp`.

### Breaking Changes

- Removed `Context::CancelWhen()`.
- Removed `LogClassification` and related functionality, added `LogLevel` instead.

### Bug Fixes

- Fixed computation of the token expiration time in `BearerTokenAuthenticationPolicy`. (A community contribution, courtesy of _[sjoubert](https://github.com/sjoubert)_)
- Fixed `Retry-After` HTTP header to be treated as case-insensitive. (A community contribution, courtesy of _[sjoubert](https://github.com/sjoubert)_)
- Fixed compilation dependency issue for MacOS when consuming the SDK from VcPkg.
- Fixed support for sending requests to endpoints with a custom port within the url on Windows when using the WinHttp transport.

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- Sylvain Joubert _([GitHub](https://github.com/sjoubert))_

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
- Throw `Azure::Core::Http::TransportException` if `curl_easy_init()` returns a null handle. (A community contribution, courtesy of _[ku-sourav](https://github.com/ku-sourav)_)

### Other Changes and Improvements

- Added support for distributing the C++ SDK as a source package via vcpkg.
- Fixed installation error when the SDK is being installed under a non-standard prefix. (A community contribution, courtesy of _[juchem](https://github.com/juchem)_)
- Fixed linker error when pthread is missing on Linux. (A community contribution, courtesy of _[lordgamez](https://github.com/lordgamez)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- Gabor Gyimesi _([GitHub](https://github.com/lordgamez))_
- Marcelo Juchem _([GitHub](https://github.com/juchem))_
- `ku-sourav` _([GitHub](https://github.com/ku-sourav))_

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

### Other Changes and Improvements

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

### Other Changes and Improvements

- Improved performance on windows when using libcurl.
- Pinned the version of package dependencies.
- Added NOTICE.txt for 3rd party dependencies.
- Added build instructions for running tests. (A community contribution, courtesy of _[ku-sourav](https://github.com/ku-sourav)_)
- Updated vcpkg build instructions. (A community contribution, courtesy of _[ku-sourav](https://github.com/ku-sourav)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure Core better with their contributions to this release:

- `ku-sourav` _([GitHub](https://github.com/ku-sourav))_

## 1.0.0-beta.1 (2020-09-09)

- Initial release
