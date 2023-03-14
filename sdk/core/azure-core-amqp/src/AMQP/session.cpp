// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/Session.hpp"
#include "azure/core/amqp/Connection.hpp"
#include "azure/core/amqp/claims_based_security.hpp"
#include <azure_uamqp_c/session.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  Session::~Session() noexcept
  {
    // Release any queued links before destroying the session.
    //    m_newLinkAttachedQueue.Clear();
    if (m_session)
    {
      session_destroy(m_session);
      m_session = nullptr;
    }
  }

  Session::Session(Connection const& connection, Endpoint& endpoint, SessionEvents* eventHandler)
      : m_connectionToPoll(connection), m_eventHandler{eventHandler}
  {
    m_session = session_create_from_endpoint(
        connection, endpoint.Release(), Session::OnLinkAttachedFn, this);
  }

  Session::Session(Connection const& connection, SessionEvents* eventHandler)
      : m_connectionToPoll(connection), m_eventHandler{eventHandler}
  {
    m_session = session_create(connection, Session::OnLinkAttachedFn, this);
  }

  Endpoint::~Endpoint()
  {
    if (m_endpoint)
    {
      connection_destroy_endpoint(m_endpoint);
    }
  }

  void Session::SetIncomingWindow(uint32_t window)
  {
    if (session_set_incoming_window(m_session, window))
    {
      throw std::runtime_error("Could not set incoming window");
    }
  }
  uint32_t Session::GetIncomingWindow()
  {
    uint32_t window;
    if (session_get_incoming_window(m_session, &window))
    {
      throw std::runtime_error("Could not get incoming window");
    }
    return window;
  }

  void Session::SetOutgoingWindow(uint32_t window)
  {
    if (session_set_outgoing_window(m_session, window))
    {
      throw std::runtime_error("Could not set outgoing window");
    }
  }

  uint32_t Session::GetOutgoingWindow()
  {
    uint32_t window;
    if (session_get_outgoing_window(m_session, &window))
    {
      throw std::runtime_error("Could not get outgoing window");
    }
    return window;
  }

  void Session::SetHandleMax(uint32_t max)
  {
    if (session_set_handle_max(m_session, max))
    {
      throw std::runtime_error("Could not set handle max.");
    }
  }
  uint32_t Session::GetHandleMax()
  {
    uint32_t max;
    if (session_get_handle_max(m_session, &max))
    {
      throw std::runtime_error("Could not get handle max.");
    }
    return max;
  }

  void Session::Begin()
  {
    if (session_begin(m_session))
    {
      throw std::runtime_error("Could not begin session");
    }
  }
  void Session::End(const std::string& condition, const std::string& description)
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

  bool Session::OnLinkAttachedFn(
      void* context,
      LINK_ENDPOINT_INSTANCE_TAG* newLinkEndpoint,
      const char* name,
      bool role,
      AMQP_VALUE_DATA_TAG* source,
      AMQP_VALUE_DATA_TAG* target,
      AMQP_VALUE_DATA_TAG* properties)
  {
    Session* session = static_cast<Session*>(context);
    LinkEndpoint linkEndpoint(newLinkEndpoint);
    if (session->m_eventHandler)
    {
      return session->m_eventHandler->OnLinkAttached(
          *session,
          linkEndpoint,
          name,
          //          role ? _detail::SessionRole::Receiver : _detail::SessionRole::Sender,
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
}}}} // namespace Azure::Core::_internal::Amqp
