// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <tuple>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Common {

  /** Abstracts an operation sent to azure-c-shared-utility/azure-uamqp.
   */
  template <typename CompleteFn, typename WrapperClass> struct CompletionOperation
  {
    CompleteFn m_onOperationComplete;

    CompletionOperation(CompleteFn onOperationComplete) : m_onOperationComplete{onOperationComplete}
    {
    }

    template <typename... Args2> static void OnOperationFn(void* context, Args2... args)
    {
      auto operation = reinterpret_cast<CompletionOperation*>(context);
      operation->OnOperation(args...);
      delete operation;
    }

    template <typename... Args3> void OnOperation(Args3... args)
    {
      if (m_onOperationComplete)
      {
        WrapperClass::OnOperation(m_onOperationComplete, args...);
      }
    }
  };
}}}}} // namespace Azure::Core::_internal::Amqp::Common
