// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "transport.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#if ENABLE_UAMQP
struct SOCKET_LISTENER_INSTANCE_TAG;
struct IO_INTERFACE_DESCRIPTION_TAG;
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {

  class SocketListenerEvents {
  protected:
    ~SocketListenerEvents() {}

  public:
    virtual void OnSocketAccepted(std::shared_ptr<_internal::Transport> newTransport) = 0;
  };

  class SocketListener final {
  public:
    SocketListener(uint16_t port, SocketListenerEvents* eventHandler);
    ~SocketListener();

    void Start();
    void Stop();

    void Poll() const;

  private:
#if ENABLE_UAMQP
    static void OnSocketAcceptedFn(
        void* context,
        const IO_INTERFACE_DESCRIPTION_TAG* interfaceDescription,
        void* io_parameters);
#endif
    std::atomic_bool m_started{false};
    SocketListenerEvents* m_eventHandler{};
#if ENABLE_UAMQP
    SOCKET_LISTENER_INSTANCE_TAG* m_socket;
#endif
  };

}}}}} // namespace Azure::Core::Amqp::Network::_detail
