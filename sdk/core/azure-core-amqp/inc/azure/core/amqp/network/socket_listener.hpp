// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <atomic>
#include <functional>
#include <thread>

struct SOCKET_LISTENER_INSTANCE_TAG;
struct IO_INTERFACE_DESCRIPTION_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  struct SocketListenerEvents
  {
    virtual void OnSocketAccepted(XIO_INSTANCE_TAG* newTransport) = 0;
  };
  class SocketListener final {
  public:
    using SocketAcceptedFn = std::function<void(std::shared_ptr<Transport>)>;
    SocketListener(uint16_t port, SocketListenerEvents* eventHandler);
    ~SocketListener();

    void Start();
    void Stop();

    void Poll() const;

  private:
    static void OnSocketAcceptedFn(
        void* context,
        const IO_INTERFACE_DESCRIPTION_TAG* interfaceDescription,
        void* io_parameters);

    std::atomic_bool m_started{false};
    SocketListenerEvents* m_eventHandler{};

    SocketAcceptedFn m_onConnection;
    SOCKET_LISTENER_INSTANCE_TAG* m_socket;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
