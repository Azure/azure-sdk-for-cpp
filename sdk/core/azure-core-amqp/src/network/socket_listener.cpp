// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/common/global_state.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xio.h>
#include <azure_uamqp_c/header_detect_io.h>
#include <azure_uamqp_c/socket_listener.h>
#include <cassert>
#include <functional>
#include <stdexcept>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  namespace {
    void EnsureGlobalStateInitialized()
    {
      // Force the global instance to exist. This is required to ensure that uAMQP and
      // azure-c-shared-utility is
      auto globalInstance = Common::_detail::GlobalState::GlobalStateInstance();
      (void)globalInstance;
    }
  } // namespace

  SocketListener::SocketListener(uint16_t port, SocketListenerEvents* eventHandler)
      : m_socket{socketlistener_create(port)}, m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();
  }

  SocketListener::~SocketListener()
  {
    if (m_socket)
    {
      // Note: We cannot tear down the socket listener at this point because any derived classes
      // have already been torn down. That means that if the work manager thread is still running,
      // we will close the underlying socket handle without warning.
      if (m_registered || m_started)
      {
        assert("Socket listener destroyed while still started.");
        abort();
      }

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
    XIO_HANDLE xio = xio_create(interfaceDescription, ioParameters);
    if (listener->m_eventHandler)
    {
      listener->m_eventHandler->OnSocketAccepted(xio);
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
      throw std::runtime_error("Could not start listener.");
    }
    m_started = true;

    // Don't register the instance until the socketlistener is started - before that, the socket may
    // not be created.
    m_registered = true;
  }

  void SocketListener::Stop()
  {
    if (m_registered)
    {
      m_registered = false;
    }
    if (m_started)
    {
      m_started = false;
    }
  }

  void SocketListener::Poll() const { socketlistener_dowork(m_socket); }
}}}}} // namespace Azure::Core::_internal::Amqp::Network
