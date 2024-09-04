// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/cancellable.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/async_operation.h>
#endif // ENABLE_UAMQP

#include <memory>
#if 0
namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  Cancellable::~Cancellable()
  {
    if (m_operation)
    {
#if ENABLE_UAMQP
      async_operation_destroy(m_operation);
#endif
      m_operation = nullptr;
    }
  }

  void Cancellable::Cancel()
  {
    if (m_operation)
    {
      async_operation_cancel(m_operation);
    }
  }

}}}} // namespace Azure::Core::Amqp::_internal
#endif
