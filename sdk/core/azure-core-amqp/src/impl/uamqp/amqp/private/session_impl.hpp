// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "connection_impl.hpp"
#include "../../../../amqp/private/unique_handle.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/session.h>
#endif

#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<SESSION_INSTANCE_TAG>
  {
    static void FreeAmqpSession(SESSION_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<SESSION_INSTANCE_TAG, FreeAmqpSession>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpSession = UniqueHandle<SESSION_INSTANCE_TAG>;
#endif

  class SessionFactory final {
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
#if ENABLE_UAMQP
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::Endpoint& newEndpoint,
        _internal::SessionOptions const& options,
        _internal::SessionEvents* eventHandler);
#endif
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::SessionOptions const& options
#if ENABLE_UAMQP
        ,
        _internal::SessionEvents* eventHandler = nullptr
#endif
    );
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;
#if ENABLE_UAMQP
    operator SESSION_HANDLE() const { return m_session.get(); }
#endif

    std::shared_ptr<_detail::ConnectionImpl> GetConnection() const { return m_connectionToPoll; }

    uint32_t GetIncomingWindow();
    uint32_t GetOutgoingWindow();
    uint32_t GetHandleMax();

    void Begin();
    void End(std::string const& condition_value, std::string const& description);

    void SendDetach(
        _internal::LinkEndpoint const& linkEndpoint,
        bool closeLink,
        Models::_internal::AmqpError const& error) const;

  private:
    SessionImpl();
#if ENABLE_UAMQP
    bool m_connectionAsyncStarted{false};
#endif
    bool m_isBegun{false};
    std::shared_ptr<_detail::ConnectionImpl> m_connectionToPoll;
#if ENABLE_UAMQP
    UniqueAmqpSession m_session;
#endif
    _internal::SessionOptions m_options;
#if ENABLE_UAMQP
    _internal::SessionEvents* m_eventHandler{};
#endif

#if ENABLE_UAMQP
    static bool OnLinkAttachedFn(
        void* context,
        LINK_ENDPOINT_HANDLE newLinkEndpoint,
        const char* name,
        bool role,
        AMQP_VALUE source,
        AMQP_VALUE target,
        AMQP_VALUE properties);
#endif
  };
}}}} // namespace Azure::Core::Amqp::_detail
