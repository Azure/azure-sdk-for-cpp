// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/map.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/httpapiex.h"

#ifdef USE_HTTP
static void test_http_proxy_io()
{
    const IO_INTERFACE_DESCRIPTION* interface_desc = http_proxy_io_get_interface_description();
    if (interface_desc == NULL)
    {
        LogError("Failed to create interface_desc.\n");
    }
}

static void http_examples()
{
    HTTPAPIEX_HANDLE handle = HTTPAPIEX_Create("www.bing.com");
    if (handle == NULL)
    {
        LogError("Failed creating httpApiEx handle");
    }
    else
    {
        HTTPAPIEX_Destroy(handle);
    }
}
#endif

static void show_sastoken_example()
{
    STRING_HANDLE sas_token = SASToken_CreateString("key", "scope", "name", 987654321);
    if (sas_token == NULL)
    {
        LogError("Failed to create SAS Token.\n");
    }
    else
    {
        STRING_delete(sas_token);
    }
}

static void show_platform_info()
{
    STRING_HANDLE platform_info = platform_get_platform_info(PLATFORM_INFO_OPTION_RETRIEVE_SQM);
    if (platform_info != NULL)
    {
        (void)printf("%s\r\n", STRING_c_str(platform_info));
        STRING_delete(platform_info);
    }
}

int main(int argc, char** argv)
{
    (void)argc, (void)argv;

    if (platform_init() != 0)
    {
        LogError("Cannot initialize platform.\n");
    }
    else
    {
        show_platform_info();
        show_sastoken_example();
#ifdef USE_HTTP
        http_examples();
        test_http_proxy_io();
#endif
        platform_deinit();
    }
    return 0;
}
