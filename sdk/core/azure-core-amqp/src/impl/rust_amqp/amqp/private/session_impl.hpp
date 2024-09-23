// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "connection_impl.hpp"
#include "unique_handle.hpp"
#include "rust_amqp_wrapper.h"

#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using AmqpSessionImplementation = Azure::Core::Amqp::_detail::RustAmqpSession;
  template <> struct UniqueHandleHelper<AmqpSessionImplementation>
  {
    static void FreeAmqpSession(AmqpSessionImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<AmqpSessionImplementation, FreeAmqpSession>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpSession = UniqueHandle<AmqpSessionImplementation>;

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
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::SessionOptions const& options);
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;

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
    bool m_isBegun{false};
    std::shared_ptr<_detail::ConnectionImpl> m_connectionToPoll;
    _internal::SessionOptions m_options;
  };
}}}} // namespace Azure::Core::Amqp::_detail
