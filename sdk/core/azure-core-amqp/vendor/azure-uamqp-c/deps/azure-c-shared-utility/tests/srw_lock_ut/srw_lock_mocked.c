// Copyright (c) Microsoft. All rights reserved.

#include "windows.h"

#define InitializeSRWLock mocked_InitializeSRWLock
#define AcquireSRWLockExclusive mocked_AcquireSRWLockExclusive
#define ReleaseSRWLockExclusive mocked_ReleaseSRWLockExclusive
#define AcquireSRWLockShared mocked_AcquireSRWLockShared
#define ReleaseSRWLockShared mocked_ReleaseSRWLockShared

#ifdef __cplusplus
extern "C" {
#endif

void mocked_InitializeSRWLock(PSRWLOCK SRWLock);
void mocked_AcquireSRWLockExclusive(PSRWLOCK SRWLock);
void mocked_ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
void mocked_AcquireSRWLockShared(PSRWLOCK SRWLock);
void mocked_ReleaseSRWLockShared(PSRWLOCK SRWLock);

#ifdef __cplusplus
}
#endif


#include "..\..\..\adapters\srw_lock.c"

