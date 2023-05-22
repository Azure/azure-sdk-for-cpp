// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/session.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <azure_uamqp_c/session.h>

template <> struct Azure::Core::_internal::UniqueHandleHelper<SESSION_INSTANCE_TAG>
{
  static void FreeAmqpSession(SESSION_HANDLE obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<SESSION_INSTANCE_TAG, FreeAmqpSession>;
};

using UniqueAmqpSession = Azure::Core::_internal::UniqueHandle<SESSION_INSTANCE_TAG>;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class SessionFactory {
  public:
    static Azure::Core::Amqp::_internal::Session CreateFromInternal(
        std::shared_ptr<SessionImpl> sessionImpl)
    {
      return Azure::Core::Amqp::_internal::Session(sessionImpl);
    }

    static std::shared_ptr<SessionImpl> GetImpl(
        Azure::Core::Amqp::_internal::Session const& session)
    {
      return session.m_impl;
    }
  };

  class SessionImpl final : public std::enable_shared_from_this<SessionImpl> {
  public:
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::Endpoint& newEndpoint,
        _internal::SessionOptions const& options,
        _internal::SessionEvents* eventHandler);
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> tokenCredential,
        _internal::SessionOptions const& options,
        _internal::SessionEvents* eventHandler);
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;
    operator SESSION_HANDLE() const { return m_session.get(); }

    std::shared_ptr<ConnectionImpl> GetConnection() const { return m_connectionToPoll; }

    uint32_t GetIncomingWindow();
    uint32_t GetOutgoingWindow();
    uint32_t GetHandleMax();

    void Begin();
    void End(std::string const& condition_value, std::string const& description);

    void AuthenticateIfNeeded(std::string const& audience, Azure::Core::Context const& context);
    std::string GetSecurityToken(std::string const& audience) const;

  private:
    SessionImpl();
    std::shared_ptr<_detail::ConnectionImpl> m_connectionToPoll;
    UniqueAmqpSession m_session;
    _internal::SessionOptions m_options;
    _internal::SessionEvents* m_eventHandler{};
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential{};
    std::shared_ptr<ClaimsBasedSecurityImpl> m_claimsBasedSecurity{};
    std::map<std::string, Azure::Core::Credentials::AccessToken> m_tokenStore;
    bool m_cbsOpen{false};

    void Authenticate(
        bool isSasToken,
        Azure::Core::Credentials::TokenRequestContext const& requestContext,
        std::string const& audience,
        Azure::Core::Context const& context);

    static bool OnLinkAttachedFn(
        void* context,
        LINK_ENDPOINT_HANDLE newLinkEndpoint,
        const char* name,
        bool role,
        AMQP_VALUE source,
        AMQP_VALUE target,
        AMQP_VALUE properties);
  };
}}}} // namespace Azure::Core::Amqp::_detail
