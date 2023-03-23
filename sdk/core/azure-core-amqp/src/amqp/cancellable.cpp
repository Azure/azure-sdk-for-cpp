// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/cancellable.hpp"
#include <memory>

#include <azure_uamqp_c/async_operation.h>

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
