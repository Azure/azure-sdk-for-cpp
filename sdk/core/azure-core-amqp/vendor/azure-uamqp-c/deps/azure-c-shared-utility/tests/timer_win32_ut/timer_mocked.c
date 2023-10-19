// Copyright (c) Microsoft. All rights reserved.

#include "Windows.h"


#define QueryPerformanceFrequency mocked_QueryPerformanceFrequency
#define QueryPerformanceCounter mocked_QueryPerformanceCounter


extern BOOLEAN mocked_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
extern BOOLEAN mocked_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);

#include "../../adapters/timer.c"
