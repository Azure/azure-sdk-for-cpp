// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef FREERTOS_H
#define FREERTOS_H

#ifdef __cplusplus

extern "C" {
#include <cstdint>
#else
#include <stdint.h>
#endif /* __cplusplus */

#define CONFIG_FREERTOS_HZ 100
#include "umock_c/umock_c_prod.h"

MOCKABLE_FUNCTION(, uint32_t, xTaskGetTickCount);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // FREERTOS_H