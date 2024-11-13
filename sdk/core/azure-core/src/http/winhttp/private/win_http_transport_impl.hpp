// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <wincrypt.h>

#include <wil/resource.h>

namespace Azure { namespace Core { namespace Http { namespace _detail {
  class WinHttpTransportImpl : HttpTransport {
  private:
    WinHttpTransport const* m_parent;

    WinHttpTransportOptions m_options;
    // m_sessionhandle is const to ensure immutability.
    const Azure::Core::_internal::UniqueHandle<void*> m_sessionHandle;
    wil::unique_cert_context m_tlsClientCertificate;

    Azure::Core::_internal::UniqueHandle<void*> CreateSessionHandle();
    Azure::Core::_internal::UniqueHandle<void*> CreateConnectionHandle(
        Azure::Core::Url const& url,
        Azure::Core::Context const& context);

    std::unique_ptr<_detail::WinHttpRequest> CreateRequestHandle(
        Azure::Core::_internal::UniqueHandle<void*> const& connectionHandle,
        Azure::Core::Url const& url,
        Azure::Core::Http::HttpMethod const& method);

    // Callback to allow a derived transport to extract the request handle. Used for WebSocket
    // transports.
    virtual void OnUpgradedConnection(std::unique_ptr<_detail::WinHttpRequest> const& request)
    {
      m_parent->OnUpgradedConnection(request);
    };

  public:
    /**
     * @brief Constructs `%WinHttpTransport`.
     *
     * @param options Optional parameter to override the default settings.
     */
    WinHttpTransportImpl(
        WinHttpTransport const* parent,
        WinHttpTransportOptions const& options = WinHttpTransportOptions());

    /**
     * @brief Constructs `%WinHttpTransport`.
     *
     * @param options Optional parameter to override the default settings.
     */
    /**
     * @brief Constructs `%WinHttpTransport` object based on common Azure HTTP Transport Options
     *
     */
    WinHttpTransportImpl(
        WinHttpTransport const* parent,
        Azure::Core::Http::Policies::TransportOptions const& options);

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
    virtual ~WinHttpTransportImpl();
  };
}}}} // namespace Azure::Core::Http::_detail
