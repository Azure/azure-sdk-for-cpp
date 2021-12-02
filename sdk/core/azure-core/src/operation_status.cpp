// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/operation_status.hpp"

namespace Azure { namespace Core {

  const OperationStatus OperationStatus::NotStarted("NotStarted");
  const OperationStatus OperationStatus::Running{"Running"};
  const OperationStatus OperationStatus::Succeeded{"Succeeded"};
  const OperationStatus OperationStatus::Failed{"Failed"};
  const OperationStatus OperationStatus::Cancelled{"Cancelled"};

  // LCOV_EXCL_START
}} // namespace Azure::Core
// LCOV_EXCL_STOP
