// Copyright (c) Microsoft Corporation.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/cancellable.hpp"

#include <azure_uamqp_c/async_operation.h>

#include <memory>
#if 0
namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  Cancellable::~Cancellable()
  {
    if (m_operation)
    {
      async_operation_destroy(m_operation);
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
