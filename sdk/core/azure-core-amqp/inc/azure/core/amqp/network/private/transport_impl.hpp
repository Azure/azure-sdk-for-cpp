// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>

#include <azure_c_shared_utility/xio.h>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {

  struct TransportImpl : public std::enable_shared_from_this<TransportImpl>
  {

  private:
    XIO_HANDLE m_xioInstance{};
    Azure::Core::Amqp::Network::_internal::TransportEvents* m_eventHandler;
    bool m_isOpen{false};

    static void OnOpenCompleteFn(void* context, IO_OPEN_RESULT_TAG openResult);
    static void OnBytesReceivedFn(void* context, const unsigned char* buffer, size_t size);
    static void OnIoErrorFn(void* context);

  public:
    TransportImpl(
        XIO_INSTANCE_TAG* instance,
        Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);
    TransportImpl(TransportImpl&& instance) = delete;
    TransportImpl(Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);

    virtual ~TransportImpl();
    virtual bool Open();
    virtual bool Close(Azure::Core::Amqp::Network::_internal::Transport::TransportCloseCompleteFn);
    virtual bool Send(
        uint8_t*,
        size_t,
        Azure::Core::Amqp::Network::_internal::Transport::TransportSendCompleteFn) const;
    void Poll() const;
    operator XIO_HANDLE() { return m_xioInstance; }
    void SetInstance(XIO_INSTANCE_TAG* instance);
  };
}}}}} // namespace Azure::Core::Amqp::Network::_detail
