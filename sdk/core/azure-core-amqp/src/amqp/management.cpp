// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/management.hpp"

#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "connection_impl.hpp"
#include "management_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  /**
   * @brief Open the management instance.
   *
   * @returns A tuple consisting of the status code for the open and the description of the
   * status.
   */
  ManagementOpenStatus ManagementClient::Open(Context const& context)
  {
    return m_impl->Open(context);
  }

  /**
   * @brief Close the management instance.
   */
  void ManagementClient::Close(Context const& context) { m_impl->Close(context); }

  ManagementOperationResult ManagementClient::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    return m_impl->ExecuteOperation(
        operationToPerform, typeOfOperation, locales, messageToSend, context);
  }
}}}} // namespace Azure::Core::Amqp::_internal

