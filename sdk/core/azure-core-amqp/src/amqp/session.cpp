// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/session.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/link.hpp"
#include "azure/core/amqp/private/connection_impl.hpp"
#include "azure/core/amqp/private/session_impl.hpp"

#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  Endpoint::~Endpoint()
  {
    if (m_endpoint)
    {
      connection_destroy_endpoint(m_endpoint);
    }
  }

  Session::Session(
      Connection const& parentConnection,
      Endpoint& newEndpoint,
      SessionEvents* eventHandler)
      : m_impl{std::make_shared<_detail::SessionImpl>(parentConnection, newEndpoint, eventHandler)}
  {
  }

  Session::Session(Connection const& parentConnection, SessionEvents* eventHandler)
      : m_impl{std::make_shared<_detail::SessionImpl>(parentConnection, eventHandler)}
  {
  }

  Session::~Session() noexcept {}

  void Session::SetIncomingWindow(uint32_t incomingWindow)
  {
    m_impl->SetIncomingWindow(incomingWindow);
  }
  uint32_t Session::GetIncomingWindow() const { return m_impl->GetIncomingWindow(); }
  void Session::SetOutgoingWindow(uint32_t outgoingWindow)
  {
    m_impl->SetOutgoingWindow(outgoingWindow);
  }
  uint32_t Session::GetOutgoingWindow() const { return m_impl->GetOutgoingWindow(); }
  void Session::SetHandleMax(uint32_t handleMax) { m_impl->SetHandleMax(handleMax); }
  uint32_t Session::GetHandleMax() const { return m_impl->GetHandleMax(); }

  void Session::Begin() { m_impl->Begin(); }
  void Session::End(std::string const& condition_value, std::string const& description)
  {
    m_impl->End(condition_value, description);
  }
  // Endpoint Session::CreateLinkEndpoint(std::string const& name)
  //{
  //   return m_impl->CreateLinkEndpoint(name);
  // };
  // void Session::DestroyLinkEndpoint(Endpoint& endpoint)
  //{
  //   return m_impl->DestroyLinkEndpoint(endpoint);
  // }
  // void Session::SetLinkEndpointCallback(
  //     Endpoint& endpoint,
  //     OnEndpointFrameReceivedCallback callback)
  //{
  //   m_impl->SetLinkEndpointCallback(endpoint, callback);
  // }
  // void Session::StartLinkEndpoint(Endpoint& endpoint, OnEndpointFrameReceivedCallback callback)
  //{
  //   m_impl->StartLinkEndpoint(endpoint, callback);
  // }
  // void Session::SendFlow(Endpoint& endpoint, Flow& flow) { m_impl->SendFlow(endpoint, flow); }

  // void Session::SendAttach(Endpoint& endpoint, Attach& attach)
  //{
  //   m_impl->SendAttach(endpoint, attach);
  // }
  // void Session::SendDisposition(Endpoint& endpoint, Disposition& disposition)
  //{
  //   m_impl->SendDisposition(endpoint, disposition);
  // }
  // void Session::SendDetach(Endpoint& endpoint, Detach& detach)
  //{
  //   m_impl->SendDetach(endpoint, detach);
  // }
  // SessionSendTransferResult Session::SendTransfer(
  //     Endpoint& endpoint,
  //     Transfer& transfer,
  //     std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
  //     uint32_t* deliveryNumber,
  //     Azure::Core::_internal::Amqp::Network::Transport::TransportSendCompleteFn sendComplete)
  //{
  //   return m_impl->SendTransfer(endpoint, transfer, payloads, deliveryNumber, sendComplete);
  // }

  namespace _detail {

    SessionImpl::~SessionImpl() noexcept
    {
      // Release any queued links before destroying the session.
      //    m_newLinkAttachedQueue.Clear();
      if (m_session)
      {
        session_destroy(m_session);
        m_session = nullptr;
      }
    }

    SessionImpl::SessionImpl(
        Connection const& connection,
        Endpoint& endpoint,
        SessionEvents* eventHandler)
        : m_connectionToPoll(connection), m_eventHandler{eventHandler}
    {
      m_session = session_create_from_endpoint(
          *connection.GetImpl(), endpoint.Release(), SessionImpl::OnLinkAttachedFn, this);
    }

    SessionImpl::SessionImpl(Connection const& connection, SessionEvents* eventHandler)
        : m_connectionToPoll(connection), m_eventHandler{eventHandler}
    {
      m_session = session_create(*connection.GetImpl(), SessionImpl::OnLinkAttachedFn, this);
    }

    void SessionImpl::SetIncomingWindow(uint32_t window)
    {
      if (session_set_incoming_window(m_session, window))
      {
        throw std::runtime_error("Could not set incoming window");
      }
    }
    uint32_t SessionImpl::GetIncomingWindow()
    {
      uint32_t window;
      if (session_get_incoming_window(m_session, &window))
      {
        throw std::runtime_error("Could not get incoming window");
      }
      return window;
    }

    void SessionImpl::SetOutgoingWindow(uint32_t window)
    {
      if (session_set_outgoing_window(m_session, window))
      {
        throw std::runtime_error("Could not set outgoing window");
      }
    }

    uint32_t SessionImpl::GetOutgoingWindow()
    {
      uint32_t window;
      if (session_get_outgoing_window(m_session, &window))
      {
        throw std::runtime_error("Could not get outgoing window");
      }
      return window;
    }

    void SessionImpl::SetHandleMax(uint32_t max)
    {
      if (session_set_handle_max(m_session, max))
      {
        throw std::runtime_error("Could not set handle max.");
      }
    }
    uint32_t SessionImpl::GetHandleMax()
    {
      uint32_t max;
      if (session_get_handle_max(m_session, &max))
      {
        throw std::runtime_error("Could not get handle max.");
      }
      return max;
    }

    void SessionImpl::Begin()
    {
      if (session_begin(m_session))
      {
        throw std::runtime_error("Could not begin session");
      }
    }
    void SessionImpl::End(const std::string& condition, const std::string& description)
    {
      // When we end the session, it clears all the links, so we need to ensure that the
      //    m_newLinkAttachedQueue.Clear();
      if (session_end(
              m_session,
              condition.empty() ? nullptr : condition.c_str(),
              description.empty() ? nullptr : description.c_str()))
      {
        throw std::runtime_error("Could not begin session");
      }
    }

    bool SessionImpl::OnLinkAttachedFn(
        void* context,
        LINK_ENDPOINT_INSTANCE_TAG* newLinkEndpoint,
        const char* name,
        bool role,
        AMQP_VALUE_DATA_TAG* source,
        AMQP_VALUE_DATA_TAG* target,
        AMQP_VALUE_DATA_TAG* properties)
    {
      SessionImpl* session = static_cast<SessionImpl*>(context);
      LinkEndpoint linkEndpoint(newLinkEndpoint);
      if (session->m_eventHandler)
      {
        return session->m_eventHandler->OnLinkAttached(
            session->shared_from_this(),
            linkEndpoint,
            name,
            role == role_receiver ? Azure::Core::_internal::Amqp::SessionRole::Receiver
                                  : Azure::Core::_internal::Amqp::SessionRole::Sender,
            source,
            target,
            properties);
      }
      else
      {
        // Even if we don't have any actions to take, if we return false, the connection will be
        // aborted.
        return true;
      }
      static_cast<void>(role);
    }
  } // namespace _detail
}}}} // namespace Azure::Core::_internal::Amqp