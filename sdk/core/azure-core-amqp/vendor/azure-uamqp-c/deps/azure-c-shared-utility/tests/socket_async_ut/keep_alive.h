// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of socket_async_ut.c with a #include. It
// is broken out for readability.


static int keep_alive;     // < 0 for system defaults, 0 to disable, > 0 to use supplied idle, interval, and count
static int keep_idle;      // seconds before first keepalive packet (ignored if keep_alive <= 0)
static int keep_interval;  // seconds between keepalive packets (ignored if keep_alive <= 0)
static int keep_count;     // number of times to try before declaring failure (ignored if keep_alive <= 0)
#define test_keep_alive 1
#define test_keep_alive_sys_default -1
#define test_keep_idle 22
#define test_keep_interval 33
#define test_keep_count 66
#define KEEP_ALIVE_UNDEFINED -1
#define KEEP_ALIVE_FALSE 0

static int my_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int value;
    (void)optlen;
    // All options are integers
    ASSERT_ARE_EQUAL(int, sockfd, test_socket);
    value = *((int*)optval);
    if (level == IPPROTO_TCP)
    {
        switch (optname)
        {
        case TCP_KEEPIDLE: keep_idle = value; break;
        case TCP_KEEPINTVL: keep_interval = value; break;
        case TCP_KEEPCNT: keep_count = value; break;
        default:
        {
            ASSERT_FAIL("Unexpected setsockopt IPPROTO_TCP parameters");
            break;
        }
        }
    }
    else if (level == SOL_SOCKET && optname == SO_KEEPALIVE)
    {
        keep_alive = value;
        ASSERT_IS_TRUE(value == 0 || value == 1, "Unexpected keep-alive value");
    }
    else
    {
        ASSERT_FAIL("Unexpected setsockopt parameters");
    }
    return 0;
}

static void init_keep_alive_values()
{
    keep_alive = KEEP_ALIVE_UNDEFINED;
    keep_idle = KEEP_ALIVE_UNDEFINED;
    keep_interval = KEEP_ALIVE_UNDEFINED;
    keep_count = KEEP_ALIVE_UNDEFINED;
}

static void ASSERT_KEEP_ALIVE_UNTOUCHED()
{
    ASSERT_ARE_EQUAL(int, keep_alive, KEEP_ALIVE_UNDEFINED, "keep_alive in ASSERT_KEEP_ALIVE_UNTOUCHED");
    ASSERT_ARE_EQUAL(int, keep_idle, KEEP_ALIVE_UNDEFINED, "keep_idle in ASSERT_KEEP_ALIVE_UNTOUCHED");
    ASSERT_ARE_EQUAL(int, keep_interval, KEEP_ALIVE_UNDEFINED, "keep_interval in ASSERT_KEEP_ALIVE_UNTOUCHED");
    ASSERT_ARE_EQUAL(int, keep_count, KEEP_ALIVE_UNDEFINED, "keep_count in ASSERT_KEEP_ALIVE_UNTOUCHED");
}

static void ASSERT_KEEP_ALIVE_FALSE()
{
    ASSERT_ARE_EQUAL(int, keep_alive, KEEP_ALIVE_FALSE, "keep_alive in ASSERT_KEEP_ALIVE_FALSE");
    ASSERT_ARE_EQUAL(int, keep_idle, KEEP_ALIVE_UNDEFINED, "keep_idle in ASSERT_KEEP_ALIVE_FALSE");
    ASSERT_ARE_EQUAL(int, keep_interval, KEEP_ALIVE_UNDEFINED, "keep_interval in ASSERT_KEEP_ALIVE_FALSE");
    ASSERT_ARE_EQUAL(int, keep_count, KEEP_ALIVE_UNDEFINED, "keep_count in ASSERT_KEEP_ALIVE_FALSE");
}

/* Tests_SRS_SOCKET_ASYNC_30_014: [ If the optional options parameter is non-NULL and is_UDP is false, socket_async_create shall set the socket options to the provided values. ]*/
static void ASSERT_KEEP_ALIVE_SET()
{
    ASSERT_ARE_EQUAL(int, keep_alive, test_keep_alive, "keep_count in ASSERT_KEEP_ALIVE_SET");
    ASSERT_ARE_EQUAL(int, keep_idle, test_keep_idle, "keep_idle in ASSERT_KEEP_ALIVE_SET");
    ASSERT_ARE_EQUAL(int, keep_interval, test_keep_interval, "keep_interval in ASSERT_KEEP_ALIVE_SET");
    ASSERT_ARE_EQUAL(int, keep_count, test_keep_count, "keep_count in ASSERT_KEEP_ALIVE_SET");
}


