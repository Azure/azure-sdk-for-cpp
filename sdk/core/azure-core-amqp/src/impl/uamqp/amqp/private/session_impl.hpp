// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "connection_impl.hpp"

#include <azure_uamqp_c/session.h>

#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<SESSION_INSTANCE_TAG>
  {
    static void FreeAmqpSession(SESSION_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<SESSION_INSTANCE_TAG, FreeAmqpSession>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpSession = UniqueHandle<SESSION_INSTANCE_TAG>;

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
    // Internal seam for exercising native shutdown failures without a live broker.
    using EndSession = int (*)(SESSION_HANDLE, char const*, char const*);

    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::Endpoint& newEndpoint,
        _internal::SessionOptions const& options,
        _internal::SessionEvents* eventHandler);
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::SessionOptions const& options,
        _internal::SessionEvents* eventHandler = nullptr,
        EndSession endSession = session_end);
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;
    operator SESSION_HANDLE() const { return m_session.get(); }

    std::shared_ptr<_detail::ConnectionImpl> GetConnection() const { return m_connectionToPoll; }

    uint32_t GetIncomingWindow();
    uint32_t GetOutgoingWindow();
    uint32_t GetHandleMax();

    void Begin(Azure::Core::Context const&);
    void End(Azure::Core::Context const&);
    void End(
        std::string const& condition_value,
        std::string const& description,
        Azure::Core::Context const&);

    void SendDetach(
        _internal::LinkEndpoint const& linkEndpoint,
        bool closeLink,
        Models::_internal::AmqpError const& error) const;

  private:
    SessionImpl();
    EndSession m_endSession{session_end};
    bool m_connectionAsyncStarted{false};
    bool m_isBegun{false};
    // A failed protocol shutdown still requires native destruction and a polling release.
    bool m_endFailed{false};
    std::shared_ptr<_detail::ConnectionImpl> m_connectionToPoll;
    UniqueAmqpSession m_session;
    _internal::SessionOptions m_options;
    _internal::SessionEvents* m_eventHandler{};

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
