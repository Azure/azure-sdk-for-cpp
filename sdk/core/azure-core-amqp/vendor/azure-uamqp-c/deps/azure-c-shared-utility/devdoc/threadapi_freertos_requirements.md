threadapi_freertos
=================

## Overview

threadapi_freertos implements a wrapper function for the FreeRTOS `vTaskDelay` function. FreeRTOS is not guaranteed to support threading, so it returns an error for the other ThreadAPI functions.

## References

[vTaskDelay](http://www.freertos.org/a00127.html)

###   Exposed API

**SRS_THREADAPI_FREERTOS_30_001: [** The threadapi_freertos shall implement the method `ThreadAPI_Sleep` defined in `threadapi.h`.
```c
/**
 * @brief	Sleeps the current thread for the given number of milliseconds.
 *
 * @param	milliseconds	The number of milliseconds to sleep.
 */
void ThreadAPI_Sleep(unsigned int milliseconds);
```
 **]**


###   ThreadAPI_Sleep

```c
void ThreadAPI_Sleep(unsigned int milliseconds);
```

**SRS_THREADAPI_FREERTOS_30_002: [** The ThreadAPI_Sleep shall receive a time in milliseconds. **]**

**SRS_THREADAPI_FREERTOS_30_003: [** The ThreadAPI_Sleep shall stop the thread for the specified time. **]**  


###   ThreadAPI_Create

```c
THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle,  THREAD_START_FUNC func, void* arg);
```

**SRS_THREADAPI_FREERTOS_30_004: [** FreeRTOS is not guaranteed to support threading, so ThreadAPI_Create shall return THREADAPI_ERROR. **]**


###   ThreadAPI_Join

```c
THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res);
```

**SRS_THREADAPI_FREERTOS_30_005: [** FreeRTOS is not guaranteed to support threading, so ThreadAPI_Join shall return THREADAPI_ERROR. **]**


###   ThreadAPI_Exit

```c
void ThreadAPI_Exit(int res);
```

**SRS_THREADAPI_FREERTOS_30_006: [** FreeRTOS is not guaranteed to support threading, so ThreadAPI_Exit shall do nothing. **]**
