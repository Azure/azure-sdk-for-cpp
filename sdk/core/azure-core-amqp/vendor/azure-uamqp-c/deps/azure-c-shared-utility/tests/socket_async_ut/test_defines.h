// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of socket_async_ut.c with a #include. It
// is broken out for readability.


// These definitions provide parameters and pass / fail values for unit testing
static int test_socket = (int)0x1;
static uint16_t test_port = 0x5566;
static uint32_t test_ipv4 = 0x11223344;

char test_msg[] = "Send this";

#define BAD_BUFFER_COUNT 10000
#define RECV_FAIL_RETURN -1
#define RECV_ZERO_FLAGS 0
#define SEND_FAIL_RETURN -1
#define SEND_ZERO_FLAGS 0
#define SELECT_FAIL_RETURN -1
#define SOCKET_FAIL_RETURN -1
#define SETSOCKOPT_FAIL_RETURN -1
#define BIND_FAIL_RETURN -1
#define EXTENDED_ERROR_FAIL EACCES
#define EXTENDED_ERROR_WAITING EAGAIN
#define EXTENDED_ERROR_CONNECT_WAITING EINPROGRESS
static size_t sizeof_int = sizeof(test_socket);

typedef struct
{
    char * buffer;
    size_t size;
    size_t *returned_count;
    const char* fail_msg;

} send_receive_parameters_t;

void populate_s_r_parameters(send_receive_parameters_t* p, char* buffer, size_t size, size_t* returned_count, const char* fail_msg)
{
    p->buffer = buffer;
    p->size = size;
    p->returned_count = returned_count;
    p->fail_msg = fail_msg;
}
