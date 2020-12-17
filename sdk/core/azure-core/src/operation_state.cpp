// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/operation_state.hpp"

namespace Azure { namespace Core {

  const OperationState OperationState::NotStarted("NotStarted");
  const OperationState OperationState::Running{"Running"};
  const OperationState OperationState::Succeeded{"Succeeded"};
  const OperationState OperationState::Failed{"Failed"};
  const OperationState OperationState::Cancelled{"Cancelled"};

}} // namespace Azure::Core
