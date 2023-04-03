// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/session.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class SessionImpl final : public std::enable_shared_from_this<SessionImpl> {
  public:
    using OnEndpointFrameReceivedCallback = std::function<
        void(AMQP_VALUE_DATA_TAG* performative, uint32_t framePayloadSize, uint8_t* payload)>;

    SessionImpl(
        _internal::Connection const& parentConnection,
        _internal::Endpoint& newEndpoint,
        _internal::SessionEvents* eventHandler);
    SessionImpl(
        _internal::Connection const& parentConnection,
        _internal::SessionEvents* eventHandler);
    ~SessionImpl() noexcept;

    SessionImpl(SessionImpl const&) = delete;
    SessionImpl& operator=(SessionImpl const&) = delete;
    SessionImpl(SessionImpl&&) noexcept = delete;
    SessionImpl& operator=(SessionImpl&&) noexcept = delete;
    operator SESSION_HANDLE() const { return m_session; }

    void SetIncomingWindow(uint32_t incomingWindow);
    uint32_t GetIncomingWindow();
    void SetOutgoingWindow(uint32_t outgoingWindow);
    uint32_t GetOutgoingWindow();
    void SetHandleMax(uint32_t handleMax);
    uint32_t GetHandleMax();

    void Begin();
    void End(std::string const& condition_value, std::string const& description);
    _internal::Endpoint CreateLinkEndpoint(std::string const& name);
    void DestroyLinkEndpoint(_internal::Endpoint& endpoint);
    void SetLinkEndpointCallback(
        _internal::Endpoint& endpoint,
        OnEndpointFrameReceivedCallback callback);
    void StartLinkEndpoint(_internal::Endpoint& endpoint, OnEndpointFrameReceivedCallback callback);
    void SendFlow(_internal::Endpoint& endpoint, _internal::Flow& flow);
    void SendAttach(_internal::Endpoint& endpoint, _internal::Attach& attach);
    void SendDisposition(_internal::Endpoint& endpoint, _internal::Disposition& disposition);
    void SendDetach(_internal::Endpoint& endpoint, _internal::Detach& detach);
    // SessionSendTransferResult SendTransfer(
    //     Endpoint& endpoint,
    //     Transfer& transfer,
    //     std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
    //     uint32_t* deliveryNumber,
    //     Azure::Core::Amqp::_internal::Network::Transport::TransportSendCompleteFn sendComplete);

  private:
    SessionImpl();
    SESSION_INSTANCE_TAG* m_session;
    _internal::Connection const& m_connectionToPoll;
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
