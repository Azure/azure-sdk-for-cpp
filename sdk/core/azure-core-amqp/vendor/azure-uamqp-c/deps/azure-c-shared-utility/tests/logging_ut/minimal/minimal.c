// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "windows.h"

#include "azure_c_shared_utility/xlogging.h"

int main(void)
{

    LogError("Hello World from LogError, here's a value: %d", 42);

    LogInfo("Hello World from LogInfo, here's a value: %d", 0x42);

    SetLastError(ERROR_ACCESS_DENIED);

    LogLastError("Hello World from LogLastError, some access was denied! here's a value: 0x%x", '3');

    SetLastError(ERROR_SUCCESS);

    LogLastError("Hello World from LogLastError, everything is fine now! here's a value: 0x%x", '3');

    return 0;
}
