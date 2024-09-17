// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP
#include "transport.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

struct SOCKET_LISTENER_INSTANCE_TAG;
struct IO_INTERFACE_DESCRIPTION_TAG;

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
    static void OnSocketAcceptedFn(
        void* context,
        const IO_INTERFACE_DESCRIPTION_TAG* interfaceDescription,
        void* io_parameters);
    std::atomic_bool m_started{false};
    SocketListenerEvents* m_eventHandler{};
    SOCKET_LISTENER_INSTANCE_TAG* m_socket;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_detail
#endif
