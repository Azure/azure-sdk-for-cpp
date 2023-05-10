// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "private/transport_impl.hpp"
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xio.h>
#include <azure_uamqp_c/header_detect_io.h>
#include <azure_uamqp_c/socket_listener.h>
#include <cassert>
#include <functional>
#include <stdexcept>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  namespace {
    void EnsureGlobalStateInitialized()
    {
      // Force the global instance to exist. This is required to ensure that uAMQP and
      // azure-c-shared-utility is properly initialized.
      auto globalInstance
          = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
      (void)globalInstance;
    }
  } // namespace

  SocketListener::SocketListener(uint16_t port, SocketListenerEvents* eventHandler)
      : m_eventHandler{eventHandler}, m_socket{socketlistener_create(port)}
  {
    EnsureGlobalStateInitialized();
  }

  SocketListener::~SocketListener()
  {
    if (m_socket)
    {
      socketlistener_destroy(m_socket);
      m_socket = nullptr;
    }
  }

  void SocketListener::OnSocketAcceptedFn(
      void* context,
      const IO_INTERFACE_DESCRIPTION* interfaceDescription,
      void* ioParameters)
  {
    SocketListener* listener = static_cast<SocketListener*>(context);
    if (listener->m_eventHandler)
    {
      auto transport{std::make_shared<Transport>(std::make_shared<_detail::TransportImpl>(
          xio_create(interfaceDescription, ioParameters), nullptr))};
      listener->m_eventHandler->OnSocketAccepted(transport);
    }
  }
  void SocketListener::Start()
  {
    if (m_started)
    {
      throw std::runtime_error("Already started.");
    }

    if (socketlistener_start(m_socket, SocketListener::OnSocketAcceptedFn, this))
    {
      auto err = errno;
      throw std::runtime_error(
          "Could not start listener. errno=" + std::to_string(err) + ", \"" + strerror(err)
          + "\".");
    }
    m_started = true;
  }

  void SocketListener::Stop()
  {
    if (m_started)
    {
      if (socketlistener_stop(m_socket))
      {
        auto err = errno;
        throw std::runtime_error(
            "Could not stop listener. errno=" + std::to_string(err) + ", \"" + strerror(err)
            + "\".");
      }
    }
    else
    {
      throw std::runtime_error("Socket listener not started.");
    }
    m_started = false;
  }

  void SocketListener::Poll() const { socketlistener_dowork(m_socket); }
}}}}} // namespace Azure::Core::Amqp::Network::_internal
