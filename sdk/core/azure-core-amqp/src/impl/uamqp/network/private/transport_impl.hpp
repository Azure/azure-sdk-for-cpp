// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/network/transport.hpp"

#include <azure_c_shared_utility/xio.h>
#include <azure_uamqp_c/sasl_mechanism.h>

#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<XIO_INSTANCE_TAG>
  {
    static void FreeXio(XIO_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<XIO_INSTANCE_TAG, FreeXio>;
  };

  template <> struct UniqueHandleHelper<SASL_MECHANISM_INSTANCE_TAG>
  {
    static void FreeSaslMechanism(SASL_MECHANISM_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<SASL_MECHANISM_INSTANCE_TAG, FreeSaslMechanism>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {
  using UniqueXioHandle = Amqp::_detail::UniqueHandle<XIO_INSTANCE_TAG>;
  using UniqueSaslMechanismHandle = Amqp::_detail::UniqueHandle<SASL_MECHANISM_INSTANCE_TAG>;

  struct TransportImpl : public std::enable_shared_from_this<TransportImpl>
  {
  public:
    TransportImpl(
        XIO_HANDLE instance,
        Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);
    TransportImpl(TransportImpl&& instance) = delete;
    TransportImpl(Azure::Core::Amqp::Network::_internal::TransportEvents* eventHandler);

    virtual ~TransportImpl();
    virtual _internal::TransportOpenStatus Open(Context const& context);
    virtual void Close(Context const& context);
    virtual bool Send(uint8_t*, size_t, Network::_internal::Transport::TransportSendCompleteFn)
        const;
    void Poll() const;
    operator XIO_HANDLE() { return m_xioInstance.get(); }
    XIO_HANDLE Release()
    {
      // The new owner would also need the IOs under this layer. No caller nests layers, so
      // refuse here instead of moving a chain of underlying IOs.
      if (m_underlyingXio || m_saslMechanism)
      {
        throw std::logic_error("Cannot release a layered transport.");
      }
      return m_xioInstance.release();
    }
    void AdoptUnderlyingXio(UniqueXioHandle underlyingXio)
    {
      m_underlyingXio = std::move(underlyingXio);
    }
    void AdoptSaslMechanism(UniqueSaslMechanismHandle saslMechanism)
    {
      m_saslMechanism = std::move(saslMechanism);
    }
    void SetEventHandler(Network::_internal::TransportEvents* eventHandler)
    {
      m_eventHandler = eventHandler;
    }

    static std::shared_ptr<TransportImpl> CreateFromXioHandle(
        XIO_HANDLE instance,
        Network::_internal::TransportEvents* eventHandler)
    {
      return std::shared_ptr<TransportImpl>(
          new TransportImpl(instance, eventHandler), [](TransportImpl* p) { delete p; });
    }

  private:
    // A layered uAMQP IO does not destroy the IO or the SASL mechanism that it uses, so this
    // transport owns them too. Members destroy in reverse order, so the layer goes first:
    // header_detect_io_destroy can close its underlying IO.
    UniqueSaslMechanismHandle m_saslMechanism{};
    UniqueXioHandle m_underlyingXio{};
    UniqueXioHandle m_xioInstance{};
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<_internal::TransportOpenStatus>
        m_openCompleteQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_closeCompleteQueue;
    Network::_internal::TransportEvents* m_eventHandler;

    bool m_isOpen{false};
    static void OnOpenCompleteFn(void* context, IO_OPEN_RESULT_TAG openResult);
    static void OnCloseCompleteFn(void* context);
    static void OnBytesReceivedFn(void* context, const unsigned char* buffer, size_t size);
    static void OnIOErrorFn(void* context);
  };
}}}}} // namespace Azure::Core::Amqp::Network::_detail
