// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLS_SERVER_IO_H
#define TLS_SERVER_IO_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"

typedef struct TLS_SERVER_IO_CONFIG_TAG
{
    const unsigned char* certificate;
    size_t certificate_size;
    const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
    void* underlying_io_parameters;
} TLS_SERVER_IO_CONFIG;

extern const IO_INTERFACE_DESCRIPTION* tls_server_io_get_interface_description(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLS_SERVER_IO_H */
