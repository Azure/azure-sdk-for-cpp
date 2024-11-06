// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words PCCERT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via WinHTTP.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/unique_handle.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"

#include <memory>
#include <string>
#include <vector>

/** 
 * @brief Declaration of a Windows PCCERT_CONTEXT structure from the Windows SDK.
 */
using PCCERT_CONTEXT = const struct _CERT_CONTEXT*;

namespace Azure { namespace Core { namespace Http {
  namespace _detail {
    class WinHttpTransportImpl;
    class WinHttpRequest;
  } // namespace _detail

  /**
   * @brief Sets the WinHTTP session and connection options used to customize the behavior of the
   * transport.
   */
  struct WinHttpTransportOptions final
  {
    /**
     * @brief When `true`, allows an invalid certificate authority.
     */
    bool IgnoreUnknownCertificateAuthority{false};

    /**
     * @brief When `true`, allows an invalid common name in a certificate.
     */
    bool IgnoreInvalidCertificateCommonName{false};

    /**
     * Proxy information.
     */

    /**
     * @brief If True, enables the use of the system default proxy.
     *
     * @remarks Set this to "true" if you would like to use a local HTTP proxy like "Fiddler" to
     * capture and analyze HTTP traffic.
     *
     * Set to "false" by default because it is not recommended to use a proxy for production and
     * Fiddler's proxy interferes with the HTTP functional tests.
     */
    bool EnableSystemDefaultProxy{false};

    /**
     * @brief If True, enables checks for certificate revocation.
     */
    bool EnableCertificateRevocationListCheck{false};

    /**
     * @brief Proxy information.
     *
     * @remark The Proxy Information string is composed of a set of elements
     * formatted as follows:
     *
     * @code
     * (\[\<scheme\>=\]\[\<scheme\>"://"\]\<server\>\[":"\<port\>\])
     * @endcode
     *
     * Each element should be separated with semicolons or whitespace.
     */
    std::string ProxyInformation;

    /**
     * @brief User name for proxy authentication.
     */
    Azure::Nullable<std::string> ProxyUserName;

    /**
     * @brief Password for proxy authentication.
     */
    Azure::Nullable<std::string> ProxyPassword;

    /**
     * @brief Array of Base64 encoded DER encoded X.509 certificate.  These certificates should
     * form a chain of certificates which will be used to validate the server certificate sent by
     * the server.
     */
    std::vector<std::string> ExpectedTlsRootCertificates;

    /**
     * @brief TLS Client Certificate Context, used when the TLS Server requests mTLS client
     * authentication.
     */
    PCCERT_CONTEXT TlsClientCertificate{nullptr};
  };

  /**
   * @brief Concrete implementation of an HTTP transport that uses WinHTTP when sending and
   * receiving requests and responses over the wire.
   */
  class WinHttpTransport : public HttpTransport {
  private:
    std::unique_ptr<_detail::WinHttpTransportImpl> m_impl;

  protected:
    /** @brief Callback to allow a derived transport to extract the request handle. Used for
     * WebSocket transports.
     *
     * @param request - Request which contains the WinHttp request handle.
     */
    virtual void OnUpgradedConnection(
        std::unique_ptr<_detail::WinHttpRequest> const& request) const;

  public:
    /**
     * @brief Constructs `%WinHttpTransport`.
     *
     * @param options Optional parameter to override the default settings.
     */
    WinHttpTransport(WinHttpTransportOptions const& options = WinHttpTransportOptions());

    /**
     * @brief Constructs `%WinHttpTransport` object based on common Azure HTTP Transport Options
     *
     */
    WinHttpTransport(Azure::Core::Http::Policies::TransportOptions const& options);

    /**
     * @brief Implements the HTTP transport interface to send an HTTP Request and produce an
     * HTTP RawResponse.
     *
     */
    virtual std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;

    // See also:
    // [Core Guidelines C.35: "A base class destructor should be either public
    // and virtual or protected and
    // non-virtual"](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c35-a-base-class-destructor-should-be-either-public-and-virtual-or-protected-and-non-virtual)
    virtual ~WinHttpTransport();
    friend _detail::WinHttpTransportImpl;
  };

}}} // namespace Azure::Core::Http
