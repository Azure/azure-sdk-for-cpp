tickcounter_freertos
=========

## Overview

tickcounter_freertos implements the Azure IoT C Shared Utility tickcounter adapter for devices running FreeRTOS.

FreeRTOS tick counting is provided by the FreeRTOS call `xTaskGetTickCount`, which requires the FreeRTOS call `vTaskStartScheduler` to be called beforehand. The call to `vTaskStartScheduler` happens during initialization for any realistic FreeRTOS application, so further system initialization in the tickcounter_freertos adapter is not required.

FreeRTOS can be configured to use 16-bit ticks by setting the configUSE_16_BIT_TICKS value to '0'. This could lead to incorrect behavior due to overflow in some situations, so use caution when employing 16-bit ticks.  


## References
[Azure IoT C Shared Utility tickcounter adapter](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/porting_guide.md#tickcounter-adapter)  
[tickcounter.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/tickcounter.h)



## Exposed API

**SRS_TICKCOUNTER_FREERTOS_30_001: [** The tickcounter_freertos adapter shall use the following data types as defined in tickcounter.h.
```c
// uint_fast32_t is a 32 bit uint
typedef uint_fast32_t tickcounter_ms_t;
// TICK_COUNTER_INSTANCE_TAG* is an opaque handle type
typedef struct TICK_COUNTER_INSTANCE_TAG* TICK_COUNTER_HANDLE;
```
 **]**  

**SRS_TICKCOUNTER_FREERTOS_30_002: [** The tickcounter_freertos adapter shall implement the API calls defined in tickcounter.h:
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```
 **]**  


###   tickcounter_create
The `tickcounter_create` call allocates and initializes an internally defined TICK_COUNTER_INSTANCE structure and returns its pointer as type TICK_COUNTER_HANDLE.
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
```

**SRS_TICKCOUNTER_FREERTOS_30_003: [** `tickcounter_create` shall allocate and initialize an internally-defined TICK_COUNTER_INSTANCE structure and return its pointer on success. **]**

**SRS_TICKCOUNTER_FREERTOS_30_004: [** If allocation of the internally-defined TICK_COUNTER_INSTANCE structure fails,  `tickcounter_create` shall return NULL. (Initialization failure is not possible for FreeRTOS.) **]**  


###   tickcounter_destroy
The `tickcounter_destroy` call releases all resources acquired by the `tickcounter_create` call.
```c
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
```

**SRS_TICKCOUNTER_FREERTOS_30_005: [** `tickcounter_destroy` shall delete the internally-defined TICK_COUNTER_INSTANCE structure specified by the `tick_counter` parameter. (This call has no failure case.) **]**

**SRS_TICKCOUNTER_FREERTOS_30_006: [** If the `tick_counter` parameter is NULL, `tickcounter_destroy` shall do nothing. **]**  


###   tickcounter_get_current_ms
The `tickcounter_get_current_ms` call returns the number of milleconds elapsed since the `tickcounter_create` call.
```c
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```

**SRS_TICKCOUNTER_FREERTOS_30_007: [** If the `tick_counter` parameter is NULL, `tickcounter_get_current_ms` shall return a non-zero value to indicate error. **]**

**SRS_TICKCOUNTER_FREERTOS_30_008: [** If the `current_ms` parameter is NULL, `tickcounter_get_current_ms` shall return a non-zero value to indicate error. **]**

**SRS_TICKCOUNTER_FREERTOS_30_009:  [** `tickcounter_get_current_ms` shall set `*current_ms` to the number of milliseconds elapsed since the `tickcounter_create` call for the specified `tick_counter` and return 0 to indicate success (In FreeRTOS this call has no failure case.) **]**

**SRS_TICKCOUNTER_FREERTOS_30_010: [** If the FreeRTOS call `xTaskGetTickCount` experiences a single overflow between the calls to `tickcounter_create` and `tickcounter_get_current_ms`, the `tickcounter_get_current_ms` call shall still return the correct interval. **]**  
