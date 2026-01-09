// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/operation_status.hpp"

namespace Azure { namespace Core {

  AZ_CORE_DLLEXPORT const OperationStatus OperationStatus::NotStarted("NotStarted");
  AZ_CORE_DLLEXPORT const OperationStatus OperationStatus::Running{"Running"};
  AZ_CORE_DLLEXPORT const OperationStatus OperationStatus::Succeeded{"Succeeded"};
  AZ_CORE_DLLEXPORT const OperationStatus OperationStatus::Failed{"Failed"};
  AZ_CORE_DLLEXPORT const OperationStatus OperationStatus::Cancelled{"Cancelled"};

}} // namespace Azure::Core
