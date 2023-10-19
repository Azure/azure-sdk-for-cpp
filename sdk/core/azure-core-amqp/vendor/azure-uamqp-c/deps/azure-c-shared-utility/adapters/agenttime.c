// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <time.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/agenttime.h"

time_t get_time(time_t* p)
{
    return time(p);
}

struct tm* get_gmtime(time_t* currentTime)
{
    return gmtime(currentTime);
}

time_t get_mktime(struct tm* cal_time)
{
    return mktime(cal_time);
}

char* get_ctime(time_t* timeToGet)
{
    return ctime(timeToGet);
}

double get_difftime(time_t stopTime, time_t startTime)
{
    return difftime(stopTime, startTime);
}
