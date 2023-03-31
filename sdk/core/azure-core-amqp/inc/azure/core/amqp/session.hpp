// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "models/amqp_value.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
  struct SESSION_INSTANCE_TAG;
  struct ENDPOINT_INSTANCE_TAG;
  struct LINK_ENDPOINT_INSTANCE_TAG;
}

namespace Azure { namespace Core { namespace Amqp {
  namespace _detail {
    class SessionImpl;
  } // namespace _detail
  namespace _internal {

    class Connection;
    class Cbs;

    // Dummy operations for operations that should never be used.
    class Flow;
    class Attach;
    class Disposition;
    class Detach;
    class Transfer;
    enum class SessionRole;

    struct SessionOptions
    {
      uint32_t IncomingWindow;
      uint32_t OutgoingWindow;
      uint32_t MaxLinks;
    };

    enum class ExpiryPolicy
    {
      LinkDetach,
      SessionEnd,
      ConnectionClose,
      Never
    };

    // An "Endpoint" is an intermediate type used to create sessions in an OnNewSession callback.
    struct Endpoint
    {
      ENDPOINT_INSTANCE_TAG* m_endpoint;
      Endpoint(ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};
      ~Endpoint();
      Endpoint(Endpoint const&) = delete;
      Endpoint& operator=(Endpoint const&) = delete;

      Endpoint(Endpoint&& other) noexcept : m_endpoint{other.m_endpoint}
      {
        other.m_endpoint = nullptr;
      }
      Endpoint& operator=(Endpoint&& other);
      ENDPOINT_INSTANCE_TAG* Release()
      {
        ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
        m_endpoint = nullptr;
        return rv;
      }
    };

    // A "Link Endpoint" is an intermediate type used to create new Links in an OnLinkAttached
    // callback. Note that LinkEndpoints do not support copy semantics, and the only way to
    // retrieve the underlying LINK_ENDPOINT_INSTANCE_TAG is to call Release(). That is because
    // the primary use scenario for a LinkEndpoint is to call link_create_from_endpoint, and
    // link_create_from_endpoint takes ownership of the underlying LINK_ENDPOINT object.
    struct LinkEndpoint
    {
      LINK_ENDPOINT_INSTANCE_TAG* m_endpoint;
      LinkEndpoint(LINK_ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};
      /* NOTE: We do *NOT* own a LinkEndpoint object, it is completely controlled by uAMQP-c. As
       * such, we are not allowed to free it.*/
      ~LinkEndpoint(){};
      LinkEndpoint(Endpoint const&) = delete;
      LinkEndpoint& operator=(LinkEndpoint const&) = delete;

      LinkEndpoint(LinkEndpoint&& other) noexcept : m_endpoint{other.m_endpoint}
      {
        other.m_endpoint = nullptr;
      }
      LinkEndpoint& operator=(Endpoint&& other);
      LINK_ENDPOINT_INSTANCE_TAG* Release()
      {
        LINK_ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
        m_endpoint = nullptr;
        return rv;
      }
    };

    enum class SessionState
    {
      Unmapped,
      BeginSent,
      BeginReceived,
      Mapped,
      EndSent,
      EndReceived,
      Discarding,
      Error,
    };

    enum class SessionSendTransferResult
    {
      Ok,
      Error,
      Busy,
    };

    class Session;
    struct SessionEvents
    {
      virtual bool OnLinkAttached(
          Session const& session,
          LinkEndpoint& newLink,
          std::string const& name,
          SessionRole role,
          Azure::Core::Amqp::Models::Value source,
          Azure::Core::Amqp::Models::Value target,
          Azure::Core::Amqp::Models::Value properties)
          = 0;
    };

    class Session final {
    public:
      using OnEndpointFrameReceivedCallback = std::function<
          void(AMQP_VALUE_DATA_TAG* performative, uint32_t framePayloadSize, uint8_t* payload)>;

      Session(
          Connection const& parentConnection,
          Endpoint& newEndpoint,
          SessionEvents* eventHandler);
      Session(Connection const& parentConnection, SessionEvents* eventHandler);
      Session(std::shared_ptr<Azure::Core::Amqp::_detail::SessionImpl> impl) : m_impl{impl} {}
      ~Session() noexcept;

      Session(Session const&) = delete;
      Session& operator=(Session const&) = delete;
      Session(Session&&) noexcept = delete;
      Session& operator=(Session&&) noexcept = delete;
      //    operator SESSION_INSTANCE_TAG*() const { return m_session; }
      std::shared_ptr<Azure::Core::Amqp::_detail::SessionImpl> GetImpl() const { return m_impl; }
      void SetIncomingWindow(uint32_t incomingWindow);
      uint32_t GetIncomingWindow() const;
      void SetOutgoingWindow(uint32_t outgoingWindow);
      uint32_t GetOutgoingWindow() const;
      void SetHandleMax(uint32_t handleMax);
      uint32_t GetHandleMax() const;

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
      // SessionSendTransferResult SendTransfer(
      //     Endpoint& endpoint,
      //     Transfer& transfer,
      //     std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
      //     uint32_t* deliveryNumber,
      //     Azure::Core::Amqp::_internal::Network::Transport::TransportSendCompleteFn
      //     sendComplete);

    private:
      std::shared_ptr<Azure::Core::Amqp::_detail::SessionImpl> m_impl;
    };
  } // namespace _internal
}}} // namespace Azure::Core::Amqp
