// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/session.hpp"

#include <azure_uamqp_c/session.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

template <> struct Azure::Core::_internal::UniqueHandleHelper<SESSION_INSTANCE_TAG>
{
  static void FreeAmqpSession(SESSION_HANDLE obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<SESSION_INSTANCE_TAG, FreeAmqpSession>;
};

using UniqueAmqpSession = Azure::Core::_internal::UniqueHandle<SESSION_INSTANCE_TAG>;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class SessionImpl final : public std::enable_shared_from_this<SessionImpl> {
  public:
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::Endpoint& newEndpoint,
        _internal::SessionEvents* eventHandler);
    SessionImpl(
        std::shared_ptr<_detail::ConnectionImpl> parentConnection,
        _internal::SessionEvents* eventHandler);
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;
    operator SESSION_HANDLE() const { return m_session.get(); }

    std::shared_ptr<ConnectionImpl> GetConnectionToPoll() const { return m_connectionToPoll; }

    void SetIncomingWindow(uint32_t incomingWindow);
    uint32_t GetIncomingWindow();
    void SetOutgoingWindow(uint32_t outgoingWindow);
    uint32_t GetOutgoingWindow();
    void SetHandleMax(uint32_t handleMax);
    uint32_t GetHandleMax();

    void Begin();
    void End(std::string const& condition_value, std::string const& description);

  private:
    SessionImpl();
    std::shared_ptr<_detail::ConnectionImpl> m_connectionToPoll;
    UniqueAmqpSession m_session;
    _internal::SessionEvents* m_eventHandler{};

    //    Common::AsyncOperationQueue<std::unique_ptr<Link>> m_newLinkAttachedQueue;

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
