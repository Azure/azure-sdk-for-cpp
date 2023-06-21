// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "async_operation_queue.hpp"

#include <azure/core/context.hpp>

struct ASYNC_OPERATION_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {
  class QueuedOperationFactory;

  /** @brief In order to enable copying a QueuedOperation object, the actual queued operation is
   * represented as a std::shared_ptr to this implementation class. The implementation class is
   * declared in a public header but should never be referenced.
   */
  template <typename T> class QueuedOperationImpl {
  public:
    QueuedOperationImpl() = default;
    QueuedOperationImpl(QueuedOperationImpl const&) = delete;
    QueuedOperationImpl& operator=(QueuedOperationImpl const&) = delete;

    QueuedOperationImpl(QueuedOperationImpl&& other) noexcept = default;
    QueuedOperationImpl& operator=(QueuedOperationImpl&&) = default;

    /** Destroy a Queued Operation.
     *
     * The act of destroying a queued operation will block until the operation has been completed.
     */
    ~QueuedOperationImpl();

    /** Cancel a Queued Operation. */
    void Cancel() const;

    /** Wait for the operation to complete.
     *
     * @param context A context object to control the lifetime of the operation.
     * @tparam PArgs Pollable objects associated with the operation.
     * @return The result of the operation.
     */
    template <class... PArgs>
    T WaitForOperationResult(Azure::Core::Context const& context, PArgs&... arguments);

    void SetAsyncOperation(ASYNC_OPERATION_INSTANCE_TAG* asyncOperation)
    {
      m_operation = asyncOperation;
    }

    void Poll() const;

  private:
    const Azure::Core::Context* m_context{}; // Context associated with the operation, only valid
                                             // when WaitForOperationResult is called.

  protected:
    QueuedOperationImpl(ASYNC_OPERATION_INSTANCE_TAG* asyncOperation) : m_operation{asyncOperation}
    {
    }
    _internal::AsyncOperationQueue<T> m_queue;
    ASYNC_OPERATION_INSTANCE_TAG* m_operation{};

    friend class QueuedOperationFactory;
  };

}}}}} // namespace Azure::Core::Amqp::Common::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _internal {

  /** @brief A QueuedOperation represents an AMQP operation which has been queued but has not
   * necessarily been processed.
   *
   * The use pattern is a function such as `QueueSend` will return a QueuedOperation object. The
   * caller can eventually call `WaitForOperationResult` on the object. The caller can also cancel
   * the operation by calling the `Cancel` method.
   */
  template <typename T> class QueuedOperation final {

  public:
    QueuedOperation(QueuedOperation const&) = default;
    QueuedOperation& operator=(QueuedOperation const&) = default;

    QueuedOperation(QueuedOperation&& other) noexcept = default;
    QueuedOperation& operator=(QueuedOperation&&) = default;

    /** Destroy a Queued Operation.
     *
     * The act of destroying a queued operation will block until the operation has been completed.
     */
    ~QueuedOperation() = default;

    /** Cancel a Queued Operation. */
    void Cancel();

    /** Wait for the operation to complete.
     *
     * @param context A context object to control the lifetime of the operation.
     * @tparam PArgs Pollable objects associated with the operation.
     * @return The result of the operation.
     */
    template <class... PArgs>
    T WaitForOperationResult(Azure::Core::Context const& context, PArgs&... arguments);

  private:
    std::shared_ptr<_detail::QueuedOperationImpl<T>> m_impl;
    QueuedOperation(std::shared_ptr<_detail::QueuedOperationImpl<T>> implementation)
        : m_impl{implementation}
    {
    }
    friend class _detail::QueuedOperationFactory;
  };
}}}}} // namespace Azure::Core::Amqp::Common::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {
  class QueuedOperationFactory final {
  public:
    template <typename T>
    static _internal::QueuedOperation<T> CreateQueuedOperation(
        ASYNC_OPERATION_INSTANCE_TAG* asyncOperationHandle);
    template <typename T>
    static _internal::QueuedOperation<T> CreateQueuedOperation(
        std::shared_ptr<QueuedOperationImpl<T>> queuedOperation);
  };
}}}}} // namespace Azure::Core::Amqp::Common::_detail
