// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"

#include "target.h"
#include "callee.h"

static CALLEE_HANDLE s_handle = NULL;

TARGET_RESULT target_create(size_t size)
{
    TARGET_RESULT result;

    if (s_handle == NULL)
    {
        /* Codes_SRS_TEMPLATE_21_001: [ The target_create shall call callee_open to do stuff and allocate the memory. ]*/
        s_handle = callee_open(size);

        if (s_handle == NULL)
        {
            /* Codes_SRS_TEMPLATE_21_002: [ If callee_open return error, the target_create shall return TARGET_RESULT_FAIL. ]*/
            LogError("callee open failed");
            result = TARGET_RESULT_FAIL;
        }
        else
        {
            char* a = (char*)malloc(100);
            if (a == NULL)
            {
                /* Codes_SRS_TEMPLATE_21_003: [ If there is no memory to control the target_create information, it shall return TARGET_RESULT_OUT_OF_MEMORY. ]*/
                LogError("There is no enough memory");
                result = TARGET_RESULT_OUT_OF_MEMORY;
                target_destroy();
            }
            else
            {
                free(a);
                /* Codes_SRS_TEMPLATE_21_008: [ If callee_open got success, it shall return TARGET_RESULT_OK. ]*/
                result = TARGET_RESULT_OK;
            }
        }
    }
    else
    {
        /* Codes_SRS_TEMPLATE_21_009: [ If callee_open is called but the connection is already created, it shall return TARGET_RESULT_OK. ]*/
        result = TARGET_RESULT_OK;
    }

    return result;
}


void target_destroy(void)
{
    if (s_handle == NULL)
    {
        /* Codes_SRS_TEMPLATE_21_007: [ If target_destroy is called but the connection is not created, the target_destroy shall not do anything. ]*/
        LogError("try to destroy a connection that was not created");
    }
    else
    {
        /* Codes_SRS_TEMPLATE_21_006: [ The target_destroy shall call callee_close to do stuff and free the memory. ]*/
        callee_close(s_handle);
        s_handle = NULL;
    }
}

TARGET_RESULT target_foo(void)
{
    TARGET_RESULT result;

    if (s_handle == NULL)
    {
        /* Codes_SRS_TEMPLATE_21_005: [ If target_foo is called but the connection is not created, the target_foo shall return TARGET_RESULT_FAIL. ]*/
        LogError("try to call foo in a connection that was not created");
        result = TARGET_RESULT_FAIL;
    }
    else
    {
        /* Codes_SRS_TEMPLATE_21_004: [ The target_foo shall do stuff calling callee_bar_1 and callee_bar_2. ]*/
        (void)callee_bar_1();

        if (callee_bar_2('a') == CALLEE_RESULT_OK)
        {
            result = TARGET_RESULT_OK;
        }
        else
        {
            /* Codes_SRS_TEMPLATE_21_010: [ If target_foo cannot execute foo, the target_foo shall return TARGET_RESULT_FAIL. ]*/
            result = TARGET_RESULT_FAIL;
        }
    }

    return result;
}

