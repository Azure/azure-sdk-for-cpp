// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_H
#define TLSIO_H

#include <stdbool.h>
#include "xio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct TLSIO_CONFIG_TAG
{
    const char* hostname;
    int port;
    const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
    void* underlying_io_parameters;
    bool invoke_on_send_complete_callback_for_fragments;
} TLSIO_CONFIG;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_H */
