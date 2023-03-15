// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/network/Transport.hpp"
#include "azure/core/amqp/common/completion_operation.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include <cassert>

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xio.h>

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

  Transport::Transport() : m_xioInstance(nullptr) { EnsureGlobalStateInitialized(); }
  Transport::Transport(XIO_HANDLE handle) : m_xioInstance{handle} {}

  void Transport::SetInstance(XIO_HANDLE handle)
  {
    assert(m_xioInstance == nullptr);
    m_xioInstance = handle;

    EnsureGlobalStateInitialized();
  }

  Transport::~Transport()
  {
    if (m_xioInstance)
    {
      if (m_isOpen)
      {
      }
      xio_destroy(m_xioInstance);
      m_xioInstance = nullptr;
    }
  }

  template <typename CompleteFn> struct CloseCallbackWrapper
  {
    static void OnOperation(CompleteFn onComplete) { onComplete(); }
  };

  bool Transport::Close(TransportCloseCompleteFn onCloseComplete)
  {
    if (!m_isOpen)
    {
      throw std::logic_error("Cannot close an unopened transport.");
    }
    auto closeOperation
        = std::make_unique<Azure::Core::_internal::Amqp::Common::CompletionOperation<
            decltype(onCloseComplete),
            CloseCallbackWrapper<decltype(onCloseComplete)>>>(onCloseComplete);
    if (m_xioInstance)
    {
      if (xio_close(
              m_xioInstance,
              std::remove_pointer<decltype(closeOperation.get())>::type::OnOperationFn,
              closeOperation.release()))
      {
        return false;
      }
      m_xioInstance = nullptr;
    }
    return true;
  }

  void Transport::OnOpenCompleteFn(void* context, IO_OPEN_RESULT ioOpenResult)
  {
    Transport* transport = reinterpret_cast<Transport*>(context);
    TransportOpenResult openResult{TransportOpenResult::Error};
    switch (ioOpenResult)
    {
      case IO_OPEN_OK:
        openResult = TransportOpenResult::Ok;
        break;
      case IO_OPEN_CANCELLED:
        openResult = TransportOpenResult::Cancelled;
        break;
      case IO_OPEN_ERROR:
        openResult = TransportOpenResult::Error;
        break;
    }
    transport->OnOpenComplete(openResult);
  }

  void Transport::OnBytesReceivedFn(void* context, unsigned char const* buffer, size_t size)
  {
    Transport* transport = reinterpret_cast<Transport*>(context);
    transport->OnBytesReceived(buffer, size);
  }

  void Transport::OnIoErrorFn(void* context)
  {
    Transport* transport = reinterpret_cast<Transport*>(context);
    transport->OnIoError();
  }

  bool Transport::Open()
  {
    if (m_isOpen)
    {
      throw std::logic_error("Cannot open an opened transport.");
    }

    if (xio_open(m_xioInstance, OnOpenCompleteFn, this, OnBytesReceivedFn, this, OnIoErrorFn, this))
    {
      return false;
    }
    m_isOpen = true;
    return true;
  }

  template <typename CompleteFn> struct SendCallbackWrapper
  {
    static void OnOperation(CompleteFn onComplete, IO_SEND_RESULT sendResult)
    {
      TransportSendResult result{TransportSendResult::Ok};
      switch (sendResult)
      {
        case IO_SEND_OK:
          result = TransportSendResult::Ok;
          break;
        case IO_SEND_CANCELLED:
          result = TransportSendResult::Cancelled;
          break;
        case IO_SEND_ERROR:
          result = TransportSendResult::Error;
          break;
      }
      onComplete(result);
    }
  };

  bool Transport::Send(unsigned char* buffer, size_t size, TransportSendCompleteFn sendComplete)
      const
  {
    auto operation{std::make_unique<Azure::Core::_internal::Amqp::Common::CompletionOperation<
        decltype(sendComplete),
        SendCallbackWrapper<decltype(sendComplete)>>>(sendComplete)};
    if (xio_send(
            m_xioInstance,
            buffer,
            size,
            std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
            operation.release()))
    {
      return false;
    }
    return true;
  }

  void Transport::Poll() const
  {
    if (m_xioInstance)
    {
      xio_dowork(m_xioInstance);
    }
  }
}}}}} // namespace Azure::Core::_internal::Amqp::Network
