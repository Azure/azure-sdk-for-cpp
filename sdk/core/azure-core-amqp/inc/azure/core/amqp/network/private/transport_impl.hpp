// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>

#include <azure_c_shared_utility/xio.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  namespace _detail {

    struct TransportImpl : public std::enable_shared_from_this<TransportImpl>
    {
      using TransportOpenCompleteFn = std::function<void(TransportOpenResult)>;
      using TransportCloseCompleteFn = std::function<void()>;
      using TransportSendCompleteFn = std::function<void(TransportSendResult)>;
      using TransportBytesReceivedCompleteFn
          = std::function<void(uint8_t const* buffer, size_t size)>;
      using TransportErrorCompleteFn = std::function<void()>;

    private:
      XIO_HANDLE m_xioInstance{};
      Azure::Core::_internal::Amqp::Network::TransportEvents* m_eventHandler;
      bool m_isOpen{false};

      static void OnOpenCompleteFn(void* context, IO_OPEN_RESULT_TAG openResult);
      static void OnBytesReceivedFn(void* context, const unsigned char* buffer, size_t size);
      static void OnIoErrorFn(void* context);

    public:
      TransportImpl(
          XIO_INSTANCE_TAG* instance,
          Azure::Core::_internal::Amqp::Network::TransportEvents* eventHandler);
      TransportImpl(Transport&& instance) = delete;
      TransportImpl(Azure::Core::_internal::Amqp::Network::TransportEvents* eventHandler);

      virtual ~TransportImpl();
      virtual bool Open();
      virtual bool Close(TransportCloseCompleteFn);
      virtual bool Send(uint8_t*, size_t, TransportSendCompleteFn) const;
      void Poll() const;
      operator XIO_HANDLE() { return m_xioInstance; }
      void SetInstance(XIO_INSTANCE_TAG* instance);
    };
}}}}}} // namespace Azure::Core::_internal::Amqp::Network::_detail
