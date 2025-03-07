// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#if ENABLE_UAMQP
#include <azure/core/context.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _detail {
  struct TransportImpl;
}}}}} // namespace Azure::Core::Amqp::Network::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** @brief Status of a transport open operation.
   *
   *
   */
  enum class TransportOpenStatus
  {
    /// Transport is in invalid state.
    Invalid,
    /// Transport open successful.
    Ok,
    /// Transport open failed with an error.
    Error,
    /// Transport open was cancelled.
    Cancelled,
  };

  /** @brief Status of a transport send operation.
   *
   */
  enum class TransportSendStatus
  {
    /// Send status Invalid - this should never be returned.
    Invalid,

    /// Send status unknown
    Unknown,

    /// Send successful
    Ok,

    /// Send completed with an error.
    Error,

    /// Send was cancelled.
    Cancelled,
  };

  class Transport;

  /** Transport operational events.
   *
   * Normally a client will not have to register for these events, but they are available for
   * clients if necessary.
   */
  class TransportEvents {
  protected:
    ~TransportEvents() {}

  public:
    /** @brief Called when bytes are received on the transport.
     *
     * @param transport The transport on which bytes are received.
     * @param buffer The buffer containing the bytes received.
     * @param size The size of the buffer.
     *
     */
    virtual void OnBytesReceived(
        Transport const& transport,
        const unsigned char* buffer,
        size_t size)
        = 0;

    /** @brief Called when an error has occurred on the transport */
    virtual void OnIOError() = 0;
  };

  class Transport final {

  public:
    using TransportSendCompleteFn = std::function<void(TransportSendStatus)>;

    /** @brief Called when a transport object is destroyed. */
    ~Transport();

    /** @brief Open the transport.
     *
     * @param context The context for cancellation.
     * @return TransportOpenStatus
     */
    TransportOpenStatus Open(Context const& context = {});

    /** @brief Close the transport.
     *
     * @param context - The context for cancellation.
     *
     */
    void Close(Context const& context = {});

    /** @brief Send bytes on the transport.
     *
     * @param buffer The buffer containing the bytes to send.
     * @param size The size of the buffer.
     * @param callback The callback to be called when the send operation is completed.
     * @return bool
     */
    bool Send(uint8_t* buffer, size_t size, TransportSendCompleteFn callback) const;

    /** @brief Poll the transport for events.
     */
    void Poll() const;

    /** @brief Set the event handler for the transport.
     *
     * @param events The event handler to be set.
     *
     * @remarks Normally a client will not have to register for these events via this method, but
     * there may be circumstances where it is necessary (if, for instance the transport was created
     * internally as a result of a socket listener).
     */
    void SetEventHandler(TransportEvents* events);

    Transport(std::shared_ptr<_detail::TransportImpl> impl) : m_impl{impl} {}
    std::shared_ptr<_detail::TransportImpl> GetImpl() const { return m_impl; }

  private:
    std::shared_ptr<_detail::TransportImpl> m_impl;
  };
}}}}} // namespace Azure::Core::Amqp::Network::_internal
#endif
