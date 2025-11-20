// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via CURL.
 */

#pragma once

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/nullable.hpp"
#include <functional>
#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Http {
  class CurlNetworkConnection;

  namespace _detail {
    /**
     * @brief Default maximum time in milliseconds that you allow the connection phase to the server
     * to take.
     *
     */
    constexpr std::chrono::milliseconds DefaultConnectionTimeout = std::chrono::minutes(5);
  } // namespace _detail

  /**
   * @brief The available options to set libcurl SSL options.
   *
   * @remark The SDK will map the enum option to libcurl's specific option. See more info here:
   * https://curl.se/libcurl/c/CURLOPT_SSL_OPTIONS.html
   *
   */
  struct CurlTransportSslOptions final
  {
    /**
     * @brief This option can enable the revocation list check.
     *
     * @remark Libcurl does revocation list check by default for SSL backends that supports this
     * feature. However, the Azure SDK overrides libcurl's behavior and disables the revocation list
     * check by default. This ensures that the libcurl behavior matches the WinHTTP behavior.
     */
    bool EnableCertificateRevocationListCheck = false;

    /**
     * @brief This option allows SSL connections to proceed even if there is an error retrieving the
     * Certificate Revocation List.
     *
     * @remark Note that this only works when libcurl is configured to use OpenSSL as its TLS
     * provider. That functionally limits this check to Linux only, and only when openssl is
     * configured (the default).
     */
    bool AllowFailedCrlRetrieval = false;

    /**
     * @brief A set of PEM encoded X.509 certificates and CRLs describing the certificates used to
     * validate the server.
     *
     * @remark The Azure SDK will not directly validate these certificates.
     *
     * @remark More about this option:
     * https://curl.se/libcurl/c/CURLOPT_CAINFO_BLOB.html
     *
     * @warning Requires libcurl >= 7.44.0
     *
     */
    std::string PemEncodedExpectedRootCertificates;
  };

  /**
   * @brief Set the libcurl connection options like a proxy and CA path.
   */
  struct CurlTransportOptions
  {
    /**
     * @brief The string for the proxy is passed directly to the libcurl handle without any parsing.
     *
     * @details libcurl will use system's environment proxy configuration (if it is set) when the \p
     * Proxy setting is not set (is null). Setting an empty string will make libcurl to ignore any
     * proxy settings from the system (use no proxy).
     *
     * @remark No validation for the string is done by the Azure SDK. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_PROXY.html.
     *
     * @remark The default value is an empty string (no proxy).
     *
     */
    Azure::Nullable<std::string> Proxy;

    /**
     * @brief Username to be used for proxy connections.
     *
     * @remark No validation for the string is done by the Azure SDK. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_PROXYUSERNAME.html.
     *
     * @remark The default value is an empty string (no proxy).
     *
     */
    Azure::Nullable<std::string> ProxyUsername;

    /**
     * @brief Password to be used for proxy connections.
     *
     * @remark No validation for the string is done by the Azure SDK. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_PROXYPASSWORD.html.
     *
     * @remark If a value is provided, the value will be used (this allows the caller to provide an
     * empty password)
     *
     */
    Azure::Nullable<std::string> ProxyPassword;
    /**
     * @brief Path to a PEM encoded file containing the certificate authorities sent to libcurl
     * handle directly.
     *
     * @remark The Azure SDK will not check if the path is valid or not.
     *
     * @remark The default is the built-in system specific path. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_CAINFO.html
     *
     * @remark This option is known to only work on Linux and might throw if set on other platforms.
     *
     */
    std::string CAInfo;

    /**
     * @brief Path to a directory which holds PEM encoded file, containing the certificate
     * authorities sent to libcurl handle directly.
     *
     * @remark The Azure SDK will not check if the path is valid or not.
     *
     * @remark The default is the built-in system specific path. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_CAPATH.html
     *
     */
    std::string CAPath;

    /**
     * @brief All HTTP requests will keep the connection channel open to the service.
     *
     * @remark The channel might be closed by the server if the server response has an error code.
     * A connection won't be re-used if it is abandoned in the middle of an operation.
     * operation.
     *
     * @remark This option is managed directly by the Azure SDK. No option is set for the curl
     * handle. It is `true` by default.
     */
    bool HttpKeepAlive = true;

    /**
     * @brief This option determines whether libcurl verifies the authenticity of the peer's
     * certificate.
     *
     * @remark The default value is `true`. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
     *
     */
    bool SslVerifyPeer = true;

    /**
     * @brief Define the SSL options for the libcurl handle.
     *
     * @remark See more info here: https://curl.se/libcurl/c/CURLOPT_SSL_OPTIONS.html.
     * The default option is all options `false`.
     *
     */
    CurlTransportSslOptions SslOptions;

    /**
     * @brief When true, libcurl will not use any functions that install signal handlers or any
     * functions that cause signals to be sent to the process.
     *
     * @details This option is here to allow multi-threaded unix applications to still set/use all
     * timeout options etc, without risking getting signals.
     *
     */
    bool NoSignal = false;

    /**
     * @brief Contain the maximum time that you allow the connection phase to the server to take.
     *
     * @details This only limits the connection phase, it has no impact once it has connected.
     *
     * @remarks The default timeout is 300 seconds and using `0` would set this default value.
     *
     */
    std::chrono::milliseconds ConnectionTimeout = _detail::DefaultConnectionTimeout;

    /**
     * @brief If set, integrates libcurl's internal tracing with Azure logging.
     */
    bool EnableCurlTracing = false;

    /**
     * @brief If set, enables libcurl's internal SSL session caching.
     */
    bool EnableCurlSslCaching = true;

    /**
     * @brief Optional callback to customize CURL handle before request execution.
     * @details Allows setting additional CURL options per request, such as CURLOPT_INTERFACE
     * for network interface binding. The callback receives the CURL* handle (as void*) and can
     * call curl_easy_setopt() directly to configure request-specific options.
     * @remark This callback is invoked just before curl_easy_perform() is called.
     */
    std::function<void(void*)> CurlOptionsCallback;

    /**
     * @brief Maximum number of simultaneously open persistent connections that libcurl may cache.
     *
     * @details This option sets the size of libcurl's internal connection cache. When the cache
     * is full, the least recently used connection is closed to make room for new ones. Increasing
     * this value can improve performance for workloads with high connection concurrency.
     *
     * @remark Set to 0 to disable connection caching entirely (not recommended for performance).
     * Set to -1 to use libcurl's default (typically 5 connections).
     * For high-throughput scenarios, values of 50-100+ are recommended.
     *
     * @remark The default value is 100 (optimized for high concurrency). More about this option:
     * https://curl.se/libcurl/c/CURLOPT_MAXCONNECTS.html
     */
    long MaxConnectionsCache = 100;

    /**
     * @brief DNS cache timeout in seconds.
     *
     * @details Sets the life-time for DNS cache entries. DNS lookups are cached by libcurl to
     * reduce latency on subsequent requests to the same host. This setting controls how long
     * these cached entries remain valid.
     *
     * @remark Set to 0 to disable DNS caching completely.
     * Set to -1 to cache DNS entries forever (or until the application terminates).
     *
     * @remark The default value is 60 seconds. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_DNS_CACHE_TIMEOUT.html
     */
    long DnsCacheTimeout = 60;

    /**
     * @brief Enable HTTP/2 protocol for multiplexed connections.
     *
     * @details HTTP/2 allows multiple requests to share a single TCP connection via multiplexing,
     * dramatically reducing connection count for high-concurrency workloads. When enabled,
     * libcurl will negotiate HTTP/2 with servers that support it, falling back to HTTP/1.1.
     *
     * @remark HTTP/2 is disabled by default for compatibility with older servers and to match
     * historical SDK behavior. Enable for significant performance gains with Azure services
     * (which fully support HTTP/2).
     *
     * @remark Default: false (HTTP/1.1 only). Setting to true enables CURL_HTTP_VERSION_2_0.
     */
    bool EnableHttp2 = false;

    /**
     * @brief Download buffer size in bytes for libcurl to use.
     *
     * @details Sets the preferred size (in bytes) for the receive buffer used by libcurl.
     * Larger buffers can improve throughput on high-speed connections by reducing the number
     * of read callbacks and system calls required. The default libcurl buffer size is ~16KB,
     * which can be a bottleneck for high-bandwidth transfers.
     *
     * @remark Set to 0 to use libcurl's default buffer size (~16KB).
     * For high-speed transfers (>100 Mbps), consider values like 512KB or 1MB.
     * libcurl will clamp values to implementation-defined limits.
     *
     * @remark Default: 524288 (512KB) for high-throughput optimization. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_BUFFERSIZE.html
     */
    long BufferSize = 524288; // 512KB for high-speed downloads

    /**
     * @brief Upload buffer size in bytes for libcurl to use.
     *
     * @details Sets the preferred size (in bytes) for the upload buffer used by libcurl.
     * Larger buffers can improve upload throughput on high-speed connections by reducing
     * the number of write callbacks and system calls. The default libcurl buffer size is ~64KB,
     * which can be a bottleneck for high-bandwidth uploads.
     *
     * @remark Set to 0 to use libcurl's default buffer size (~64KB).
     * For high-speed uploads (>100 Mbps), consider values like 512KB or 1MB.
     * libcurl will clamp values to implementation-defined limits.
     *
     * @remark Default: 524288 (512KB) for high-throughput optimization. More about this option:
     * https://curl.se/libcurl/c/CURLOPT_UPLOAD_BUFFERSIZE.html
     */
    long UploadBufferSize = 524288; // 512KB for high-speed uploads

    /**
     * @brief Enable TCP_NODELAY to disable Nagle's algorithm.
     *
     * @details When enabled, sets the TCP_NODELAY socket option which disables Nagle's algorithm.
     * Nagle's algorithm batches small TCP packets to improve network efficiency, but can add
     * 40-200ms latency for request-response patterns. Disabling it sends data immediately,
     * which is typically better for HTTP request/response workloads.
     *
     * @remark Most HTTP workloads benefit from TCP_NODELAY=1 (Nagle disabled) to reduce latency.
     * Set to false only if you're on a high-latency, low-bandwidth network where Nagle's
     * batching would help.
     *
     * @remark Default: true (Nagle's algorithm disabled for lower latency). More about this option:
     * https://curl.se/libcurl/c/CURLOPT_TCP_NODELAY.html
     */
    bool TcpNoDelay = true;

    /**
     * @brief Poll interval in milliseconds for socket readiness checks.
     *
     * @details When using CURLOPT_CONNECT_ONLY mode, libcurl requires manual polling to check
     * socket readiness. This setting controls how frequently poll() is called to check for
     * cancellation. Lower values reduce latency but increase CPU usage slightly.
     *
     * @remark The original default was 1000ms (1 second), which caused up to 1 second latency
     * on small operations like HEAD/PUT. The new default of 10ms dramatically reduces this
     * overhead while still checking for cancellation frequently.
     *
     * @remark For extremely latency-sensitive workloads, consider 1-5ms.
     * For throughput-focused workloads where latency matters less, use 50-100ms.
     *
     * @remark Default: 10 milliseconds (low-latency optimization).
     */
    long PollIntervalMs = 10;
  };

  /**
   * @brief Concrete implementation of an HTTP Transport that uses libcurl.
   */
  class CurlTransport : public HttpTransport {
  private:
    CurlTransportOptions m_options;

    /**
     * @brief Called when an HTTP response indicates the connection should be upgraded to
     * a websocket. Takes ownership of the CurlNetworkConnection object.
     */
    virtual void OnUpgradedConnection(std::unique_ptr<CurlNetworkConnection>&&){};

  public:
    /**
     * @brief Construct a new CurlTransport object.
     *
     * @param options Optional parameter to override the default options.
     */
    CurlTransport(CurlTransportOptions const& options = CurlTransportOptions()) : m_options(options)
    {
    }

    /**
     * @brief Construct a new CurlTransport object based on common Azure HTTP Transport Options
     *
     * @param options Common Azure Core Transport Options.
     */
    CurlTransport(Azure::Core::Http::Policies::TransportOptions const& options);

    /**
     * @brief Destroys a CurlTransport object.
     *
     * See also:
     * [Core Guidelines C.35: "A base class destructor should be either public
     * and virtual or protected and
     * non-virtual"](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c35-a-base-class-destructor-should-be-either-public-and-virtual-or-protected-and-non-virtual)
     */
    virtual ~CurlTransport() = default;

    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse
     *
     * @param request an HTTP Request to be send.
     * @param context A context to control the request lifetime.
     *
     * @return unique ptr to an HTTP RawResponse.
     */
    std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;
  };

}}} // namespace Azure::Core::Http

