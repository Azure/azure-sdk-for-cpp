// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

extern "C"
{
  struct XIO_INSTANCE_TAG;
}

namespace Azure { namespace Core { namespace Amqp { namespace Network {
  namespace _detail {
    struct TransportImpl;
  }
  namespace _internal {
    enum class TransportState
    {
      Closed,
      Closing,
      Open,
      Opening,
      Unknown
    };

    enum class TransportOpenResult
    {
      Invalid,
      Ok,
      Error,
      Cancelled
    };
    enum class TransportSendResult
    {
      Invalid,
      Unknown,
      Ok,
      Error,
      Cancelled
    };

    class Transport;

    struct TransportEvents
    {
      virtual void OnOpenComplete(TransportOpenResult openResult) = 0;
      virtual void OnBytesReceived(
          Transport const& transport,
          const unsigned char* buffer,
          size_t size)
          = 0;
      virtual void OnIoError() = 0;
    };

    class Transport {

    public:
      using TransportCloseCompleteFn = std::function<void()>;
      using TransportSendCompleteFn = std::function<void(TransportSendResult)>;
      Transport(std::shared_ptr<_detail::TransportImpl> impl) : m_impl{impl} {}
      Transport(XIO_INSTANCE_TAG* xioInstance, TransportEvents* events);
      Transport(Transport&& instance) = delete;
      virtual ~Transport();
      virtual bool Open();
      virtual bool Close(TransportCloseCompleteFn);
      virtual bool Send(uint8_t*, size_t, TransportSendCompleteFn) const;
      void Poll() const;
      virtual std::shared_ptr<_detail::TransportImpl> GetImpl() const { return m_impl; }

    protected:
      Transport(TransportEvents* events);

      std::shared_ptr<_detail::TransportImpl> m_impl;
    };
  } // namespace _internal
}}}} // namespace Azure::Core::Amqp::Network
