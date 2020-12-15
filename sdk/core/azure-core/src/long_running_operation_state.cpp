// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/long_running_operation_state.hpp"

namespace Azure { namespace Core {

  const LongRunningOperationState LongRunningOperationState::NotStarted("NOT_STARTED");
  const LongRunningOperationState LongRunningOperationState::InProgress{"IN_PROGRESS"};
  const LongRunningOperationState LongRunningOperationState::SuccessfullyCompleted{"SUCCESSFULLY_COMPLETED"};
  const LongRunningOperationState LongRunningOperationState::Failed{"FAILED"};
  const LongRunningOperationState LongRunningOperationState::UserCancelled{"USER_CANCELLED"};

}} // namespace Azure::Core
