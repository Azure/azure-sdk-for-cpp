// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "connection_impl.hpp"
#include "rust_amqp_wrapper.h"
#include "unique_handle.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using AmqpSessionImplementation = Azure::Core::Amqp::_detail::RustInterop::RustAmqpSession;
  using AmqpSessionOptions = Azure::Core::Amqp::_detail::RustInterop::RustAmqpSessionOptions;
  using AmqpSessionOptionsBuilder
      = Azure::Core::Amqp::_detail::RustInterop::RustAmqpSessionOptionsBuilder;

  template <> struct UniqueHandleHelper<AmqpSessionImplementation>
  {
    static void FreeAmqpSession(AmqpSessionImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<AmqpSessionImplementation, FreeAmqpSession>;
  };
  template <> struct UniqueHandleHelper<AmqpSessionOptions>
  {
    static void FreeAmqpSessionOptions(AmqpSessionOptions* obj);

    using type = Core::_internal::BasicUniqueHandle<AmqpSessionOptions, FreeAmqpSessionOptions>;
  };
  template <> struct UniqueHandleHelper<AmqpSessionOptionsBuilder>
  {
    static void FreeAmqpSessionOptionsBuilder(AmqpSessionOptionsBuilder* obj);

    using type = Core::_internal::
        BasicUniqueHandle<AmqpSessionOptionsBuilder, FreeAmqpSessionOptionsBuilder>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpSession = UniqueHandle<AmqpSessionImplementation>;
  using UniqueAmqpSessionOptions = UniqueHandle<AmqpSessionOptions>;
  using UniqueAmqpSessionOptionsBuilder = UniqueHandle<AmqpSessionOptionsBuilder>;

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
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::SessionOptions const& options);

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;

    ~SessionImpl() noexcept;
    std::shared_ptr<_detail::ConnectionImpl> GetConnection() const { return m_connection; }

    uint32_t GetIncomingWindow();
    uint32_t GetOutgoingWindow();
    uint32_t GetHandleMax();

    void Begin(Azure::Core::Context const& context);
    void End(Azure::Core::Context const& context);
    void End(
        std::string const& condition_value,
        std::string const& description,
        Azure::Core::Context const& context);
#if ENABLE_UAMQP
    void SendDetach(
        _internal::LinkEndpoint const& linkEndpoint,
        bool closeLink,
        Models::_internal::AmqpError const& error) const;
#endif
  private:
    SessionImpl();
    bool m_isBegun{false};
    UniqueAmqpSession m_session;
    std::shared_ptr<_detail::ConnectionImpl> m_connection;
    _internal::SessionOptions m_options;
  };
}}}} // namespace Azure::Core::Amqp::_detail
