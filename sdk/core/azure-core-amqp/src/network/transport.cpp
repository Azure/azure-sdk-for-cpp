// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#if ENABLE_UAMQP

#include "azure/core/amqp/internal/network/transport.hpp"

#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "private/transport_impl.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xio.h>

#include <cassert>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<XIO_INSTANCE_TAG>::FreeXio(XIO_HANDLE value) { xio_destroy(value); }
}}}} // namespace Azure::Core::Amqp::_detail

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
  Transport::~Transport() {}

  TransportOpenStatus Transport::Open(Context const& context) { return m_impl->Open(context); }
  void Transport::Close(Context const& context) { return m_impl->Close(context); }

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

  TransportImpl::TransportImpl(Network::_internal::TransportEvents* eventHandler)
      : m_xioInstance(nullptr), m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();
  }

  // This constructor is used by the SocketTransport and TlsTransport classes to construct a
  // transport around an already constructed XIO transport.
  TransportImpl::TransportImpl(XIO_HANDLE handle, Network::_internal::TransportEvents* eventHandler)
      : m_xioInstance{handle}, m_eventHandler{eventHandler}
  {
    assert(handle != nullptr);
    EnsureGlobalStateInitialized();
  }

  TransportImpl::~TransportImpl() {}

  template <typename CompleteFn> struct CloseCallbackWrapper
  {
    static void OnOperation(CompleteFn onComplete) { onComplete(); }
  };

  void TransportImpl::OnCloseCompleteFn(void* context)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    transport->m_closeCompleteQueue.CompleteOperation(true);
  }
  void TransportImpl::Close(Context const& context)
  {
    if (!m_isOpen)
    {
      throw std::logic_error("Cannot close an unopened transport.");
    }
    if (m_xioInstance)
    {
      if (xio_close(m_xioInstance.get(), OnCloseCompleteFn, this))
      {
        throw std::runtime_error("Failed to close the transport.");
      }
      m_xioInstance = nullptr;
    }
    auto result = m_closeCompleteQueue.WaitForPolledResult(context, *this);
    if (!result)
    {
      throw Azure::Core::OperationCancelledException("Close operation was cancelled.");
    }
    m_isOpen = false;
  }

  void TransportImpl::OnOpenCompleteFn(void* context, IO_OPEN_RESULT ioOpenResult)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    Network::_internal::TransportOpenStatus openResult{
        Network::_internal::TransportOpenStatus::Error};
    switch (ioOpenResult)
    {

      case IO_OPEN_RESULT_INVALID:
        openResult = Network::_internal::TransportOpenStatus::Invalid;
        break;
      case IO_OPEN_CANCELLED:
        openResult = Network::_internal::TransportOpenStatus::Cancelled;
        break;
      case IO_OPEN_ERROR:
        openResult = Network::_internal::TransportOpenStatus::Error;
        break;

      case IO_OPEN_OK:
        openResult = Network::_internal::TransportOpenStatus::Ok;
        break;
    }
    transport->m_openCompleteQueue.CompleteOperation(openResult);
  }

  void TransportImpl::OnBytesReceivedFn(void* context, unsigned char const* buffer, size_t size)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    if (transport->m_eventHandler)
    {
      transport->m_eventHandler->OnBytesReceived(transport->shared_from_this(), buffer, size);
    }
  }

  void TransportImpl::OnIOErrorFn(void* context)
  {
    TransportImpl* transport = reinterpret_cast<TransportImpl*>(context);
    if (transport->m_eventHandler)
    {

      transport->m_eventHandler->OnIOError();
    }
  }

  _internal::TransportOpenStatus TransportImpl::Open(Context const& context)
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
            OnIOErrorFn,
            this))
    {
      return _internal::TransportOpenStatus::Error;
    }
    m_isOpen = true;
    auto result = m_openCompleteQueue.WaitForPolledResult(context, *this);
    if (result)
    {
      return std::get<0>(*result);
    }
    throw Azure::Core::OperationCancelledException("Open operation was cancelled.");
  }

  template <typename CompleteFn> struct SendCallbackRewriter
  {
    static void OnOperation(CompleteFn onComplete, IO_SEND_RESULT sendResult)
    {
      Network::_internal::TransportSendStatus result{Network::_internal::TransportSendStatus::Ok};
      switch (sendResult)
      {

        case IO_SEND_RESULT_INVALID:
          result = Network::_internal::TransportSendStatus::Invalid;
          break;
        case IO_SEND_CANCELLED:
          result = Network::_internal::TransportSendStatus::Cancelled;
          break;
        case IO_SEND_ERROR:
          result = Network::_internal::TransportSendStatus::Error;
          break;

        case IO_SEND_OK:
          result = Network::_internal::TransportSendStatus::Ok;
          break;
      }
      onComplete(result);
    }
  };

  bool TransportImpl::Send(
      unsigned char* buffer,
      size_t size,
      Network::_internal::Transport::TransportSendCompleteFn sendComplete) const
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
#endif // ENABLE_UAMQP
