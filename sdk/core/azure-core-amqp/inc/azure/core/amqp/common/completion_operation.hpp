// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <tuple>

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _internal {

  /** Abstracts an operation sent to azure-c-shared-utility/azure-uamqp.
   */
  template <typename CompleteFn, typename ArgumentRewriter> struct CompletionOperation
  {
    CompleteFn m_onOperationComplete;

    CompletionOperation(CompleteFn const& onOperationComplete)
        : m_onOperationComplete{onOperationComplete}
    {
    }

    template <typename... T> static void OnOperationFn(void* context, T... args)
    {
      // Capture the operation into a unique pointer so it will be freed even if the OnOperation
      // call throws.
      std::unique_ptr<CompletionOperation> operation;
      operation.reset(reinterpret_cast<CompletionOperation*>(context));
      operation->OnOperation(args...);
    }

    template <typename... T> void OnOperation(T... args)
    {
      if (m_onOperationComplete)
      {
        ArgumentRewriter::OnOperation(m_onOperationComplete, args...);
      }
    }
  };
}}}}} // namespace Azure::Core::Amqp::Common::_internal
