// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>

extern "C"
{
  enum IO_OPEN_RESULT_TAG : int;
  struct XIO_INSTANCE_TAG;
}

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

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
    Ok,
    Error,
    Cancelled
  };
  enum class TransportSendResult
  {
    Unknown,
    Ok,
    Error,
    Cancelled
  };

  struct Transport
  {
    using TransportOpenCompleteFn = std::function<void(TransportOpenResult)>;
    using TransportCloseCompleteFn = std::function<void()>;
    using TransportSendCompleteFn = std::function<void(TransportSendResult)>;
    using TransportBytesReceivedCompleteFn
        = std::function<void(uint8_t const* buffer, size_t size)>;
    using TransportErrorCompleteFn = std::function<void()>;

  private:
    XIO_INSTANCE_TAG* m_xioInstance{};
    bool m_isOpen{false};

    static void OnOpenCompleteFn(void* context, IO_OPEN_RESULT_TAG openResult);
    virtual void OnOpenComplete(TransportOpenResult openResult) { (void)openResult; }
    static void OnBytesReceivedFn(void* context, const unsigned char* buffer, size_t size);
    virtual void OnBytesReceived(const unsigned char* buffer, size_t size)
    {
      (void)buffer, (void)size;
    };
    static void OnIoErrorFn(void* context);
    virtual void OnIoError(){};

  protected:
    Transport();

    void SetInstance(XIO_INSTANCE_TAG* instance);

  public:
    Transport(XIO_INSTANCE_TAG* instance);
    Transport(Transport&& instance) = delete;
    virtual ~Transport();
    virtual bool Open();
    virtual bool Close(TransportCloseCompleteFn);
    virtual bool Send(uint8_t*, size_t, TransportSendCompleteFn) const;
    void Poll() const;
    operator XIO_INSTANCE_TAG*() { return m_xioInstance; }
  };
}}}}} // namespace Azure::Core::_internal::Amqp::Network
