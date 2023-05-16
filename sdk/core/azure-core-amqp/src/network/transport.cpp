// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/transport.hpp"
#include "azure/core/amqp/common/completion_operation.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "private/transport_impl.hpp"
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xio.h>
#include <cassert>

void Azure::Core::_internal::UniqueHandleHelper<XIO_INSTANCE_TAG>::FreeXio(XIO_HANDLE value)
{
  xio_destroy(value);
}

namespace {
void EnsureGlobalStateInitialized()
{
  // Force the global instance to exist. This is required to ensure that uAMQP and
  // azure-c-shared-utility is
  auto globalInstance
      = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
  (void)globalInstance;
}
} // namespace

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {
  Transport::Transport(TransportEvents* eventHandler)
      : m_impl{std::make_shared<_detail::TransportImpl>(eventHandler)}
  {
  }

  Transport::~Transport() {}

  bool Transport::Open() { return m_impl->Open(); }
  bool Transport::Close(TransportCloseCompleteFn callback) { return m_impl->Close(callback); }

  bool Transport::Send(uint8_t* buffer, size_t size, TransportSendCompleteFn callback) const
  {
    return m_impl->Send(buffer, size, callback);
  }
  void Transport::Poll() const { return m_impl->Poll(); }

  void Transport::SetEventHandler(TransportEvents* eventHandler)
  {
    m_impl->SetEventHandler(eventHandler);
  }
}}}}} // namespace Azure::Core::Amqp::Network::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {

  TransportImpl::TransportImpl(Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler)
      : m_xioInstance(nullptr), m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();
  }

  // This constructor is used by the SocketTransport and TlsTransport classes to construct a
  // transport around an already constructed XIO transport.
  TransportImpl::TransportImpl(
      XIO_HANDLE handle,
      Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler)
      : m_xioInstance{handle}, m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();
  }

  void TransportImpl::SetInstance(XIO_HANDLE handle)
  {
    assert(m_xioInstance == nullptr);
    assert(handle != nullptr);
    m_xioInstance.reset(handle);

    EnsureGlobalStateInitialized();
  }

  TransportImpl::~TransportImpl() {}

  template <typename CompleteFn> struct CloseCallbackWrapper
  {
    static void OnOperation(CompleteFn onComplete) { onComplete(); }
  };

  bool TransportImpl::Close(
      Azure::Core::Amqp::Network::_internal::Transport::TransportCloseCompleteFn onCloseComplete)
  {
    if (!m_isOpen)
    {
      throw std::logic_error("Cannot close an unopened transport.");
    }
    m_isOpen = false;
    auto closeOperation
        = std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
            decltype(onCloseComplete),
            CloseCallbackWrapper<decltype(onCloseComplete)>>>(onCloseComplete);
    if (m_xioInstance)
    {
      if (xio_close(
              m_xioInstance.get(),
              std::remove_pointer<decltype(closeOperation.get())>::type::OnOperationFn,
              closeOperation.release()))
      {
        return false;
      }
      m_xioInstance = nullptr;
    }
    return true;
  }

  void TransportImpl::OnOpenCompleteFn(void* context, IO_OPEN_RESULT ioOpenResult)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    if (transport->m_eventHandler)
    {
      Azure::Core::Amqp::Network::_internal::TransportOpenResult openResult{
          Azure::Core::Amqp::Network::_internal::TransportOpenResult::Error};
      switch (ioOpenResult)
      {
          // LCOV_EXCL_START
        case IO_OPEN_RESULT_INVALID:
          openResult = Azure::Core::Amqp::Network::_internal::TransportOpenResult::Invalid;
          break;
        case IO_OPEN_CANCELLED:
          openResult = Azure::Core::Amqp::Network::_internal::TransportOpenResult::Cancelled;
          break;
        case IO_OPEN_ERROR:
          openResult = Azure::Core::Amqp::Network::_internal::TransportOpenResult::Error;
          break;
          // LCOV_EXCL_STOP
        case IO_OPEN_OK:
          openResult = Azure::Core::Amqp::Network::_internal::TransportOpenResult::Ok;
          break;
      }
      transport->m_eventHandler->OnOpenComplete(openResult);
    }
  }

  void TransportImpl::OnBytesReceivedFn(void* context, unsigned char const* buffer, size_t size)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    if (transport->m_eventHandler)
    {
      transport->m_eventHandler->OnBytesReceived(transport->shared_from_this(), buffer, size);
    }
  }

  // LCOV_EXCL_START
  void TransportImpl::OnIoErrorFn(void* context)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    if (transport->m_eventHandler)
    {

      transport->m_eventHandler->OnIoError();
    }
  }
  // LCOV_EXCL_STOP

  bool TransportImpl::Open()
  {
    if (m_isOpen)
    {
      throw std::logic_error("Cannot open an opened transport.");
    }

    if (xio_open(
            m_xioInstance.get(),
            OnOpenCompleteFn,
            this,
            OnBytesReceivedFn,
            this,
            OnIoErrorFn,
            this))
    {
      return false;
    }
    m_isOpen = true;
    return true;
  }

  template <typename CompleteFn> struct SendCallbackRewriter
  {
    static void OnOperation(CompleteFn onComplete, IO_SEND_RESULT sendResult)
    {
      Azure::Core::Amqp::Network::_internal::TransportSendResult result{
          Azure::Core::Amqp::Network::_internal::TransportSendResult::Ok};
      switch (sendResult)
      {
          // LCOV_EXCL_START
        case IO_SEND_RESULT_INVALID:
          result = Azure::Core::Amqp::Network::_internal::TransportSendResult::Invalid;
          break;
        case IO_SEND_CANCELLED:
          result = Azure::Core::Amqp::Network::_internal::TransportSendResult::Cancelled;
          break;
        case IO_SEND_ERROR:
          result = Azure::Core::Amqp::Network::_internal::TransportSendResult::Error;
          break;
          // LCOV_EXCL_STOP
        case IO_SEND_OK:
          result = Azure::Core::Amqp::Network::_internal::TransportSendResult::Ok;
          break;
      }
      onComplete(result);
    }
  };

  bool TransportImpl::Send(
      unsigned char* buffer,
      size_t size,
      Azure::Core::Amqp::Network::_internal::Transport::TransportSendCompleteFn sendComplete) const
  {
    auto operation{std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
        decltype(sendComplete),
        SendCallbackRewriter<decltype(sendComplete)>>>(sendComplete)};
    if (xio_send(
            m_xioInstance.get(),
            buffer,
            size,
            std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
            operation.release()))
    {
      return false;
    }
    return true;
  }

  void TransportImpl::Poll() const
  {
    if (m_xioInstance)
    {
      xio_dowork(m_xioInstance.get());
    }
  }
}}}}} // namespace Azure::Core::Amqp::Network::_detail
