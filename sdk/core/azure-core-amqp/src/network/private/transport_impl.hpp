// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/common/async_operation_queue.hpp"

#if ENABLE_UAMQP
#include <azure_c_shared_utility/xio.h>
#endif
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<XIO_INSTANCE_TAG>
  {
    static void FreeXio(XIO_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<XIO_INSTANCE_TAG, FreeXio>;
  };
#endif // ENABLE_UAMQP
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {
#if ENABLE_UAMQP
  using UniqueXioHandle = Amqp::_detail::UniqueHandle<XIO_INSTANCE_TAG>;

#endif // ENABLE_UAMQP

  struct TransportImpl : public std::enable_shared_from_this<TransportImpl>
  {
  public:
#if ENABLE_UAMQP
    TransportImpl(
        XIO_HANDLE instance,
        Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);
#endif
    TransportImpl(TransportImpl&& instance) = delete;
    TransportImpl(Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);

    virtual ~TransportImpl();
    virtual _internal::TransportOpenStatus Open(Context const& context);
    virtual void Close(Context const& context);
    virtual bool Send(uint8_t*, size_t, Network::_internal::Transport::TransportSendCompleteFn)
        const;
    void Poll() const;
#if ENABLE_UAMQP
    operator XIO_HANDLE() { return m_xioInstance.get(); }
    XIO_HANDLE Release() { return m_xioInstance.release(); }
#endif
    void SetEventHandler(Network::_internal::TransportEvents* eventHandler)
    {
      m_eventHandler = eventHandler;
    }

#if ENABLE_UAMQP
    static std::shared_ptr<TransportImpl> CreateFromXioHandle(
        XIO_HANDLE instance,
        Network::_internal::TransportEvents* eventHandler)
    {
      return std::shared_ptr<TransportImpl>(
          new TransportImpl(instance, eventHandler), [](TransportImpl* p) { delete p; });
    }
#endif

  private:
#if ENABLE_UAMQP
    UniqueXioHandle m_xioInstance{};
#endif
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<_internal::TransportOpenStatus>
        m_openCompleteQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_closeCompleteQueue;
    Network::_internal::TransportEvents* m_eventHandler;

    bool m_isOpen{false};
#if ENABLE_UAMQP
    static void OnOpenCompleteFn(void* context, IO_OPEN_RESULT_TAG openResult);
    static void OnCloseCompleteFn(void* context);
    static void OnBytesReceivedFn(void* context, const unsigned char* buffer, size_t size);
    static void OnIOErrorFn(void* context);
#endif // ENABLE_UAMQP
  };
}}}}} // namespace Azure::Core::Amqp::Network::_detail
