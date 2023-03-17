// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "azure/core/amqp/session.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  class SessionImpl final : public std::enable_shared_from_this<SessionImpl> {
  public:
    using OnEndpointFrameReceivedCallback = std::function<
        void(AMQP_VALUE_DATA_TAG* performative, uint32_t framePayloadSize, uint8_t* payload)>;

    SessionImpl(
        Connection const& parentConnection,
        Endpoint& newEndpoint,
        SessionEvents* eventHandler);
    SessionImpl(Connection const& parentConnection, SessionEvents* eventHandler);
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
    Endpoint CreateLinkEndpoint(std::string const& name);
    void DestroyLinkEndpoint(Endpoint& endpoint);
    void SetLinkEndpointCallback(Endpoint& endpoint, OnEndpointFrameReceivedCallback callback);
    void StartLinkEndpoint(Endpoint& endpoint, OnEndpointFrameReceivedCallback callback);
    void SendFlow(Endpoint& endpoint, Flow& flow);
    void SendAttach(Endpoint& endpoint, Attach& attach);
    void SendDisposition(Endpoint& endpoint, Disposition& disposition);
    void SendDetach(Endpoint& endpoint, Detach& detach);
    //SessionSendTransferResult SendTransfer(
    //    Endpoint& endpoint,
    //    Transfer& transfer,
    //    std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
    //    uint32_t* deliveryNumber,
    //    Azure::Core::_internal::Amqp::Network::Transport::TransportSendCompleteFn sendComplete);

  private:
    SessionImpl();
    SESSION_INSTANCE_TAG* m_session;
    Connection const& m_connectionToPoll;
    SessionEvents* m_eventHandler{};
    std::shared_ptr<Cbs> m_claimsBasedSecurity;

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
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
