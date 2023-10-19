AgentTime
================

## Overview

AgentTime exports platform independent time related functions. It is a platform abstraction and it requires a specific implementation for each platform.
Most of the times these functions can simply call the C standard time functions.

Most implementations of the C `time()` function return seconds since 00:00 hours, Jan 1, 1970 UTC. Implementations
which do not must convert the output of `time()` to conform to this spec.

###### Header files
- [agenttime.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/agenttime.h)<br/>


## Exposed API
**SRS_AGENT_TIME_99_001: [** AGENT_TIME shall have the following interface **]**
```c
/* same functionality as most implementations time() of standard C function */
time_t get_time(time_t* p);

/*the same as C's difftime*/
extern double get_difftime(time_t stopTime, time_t startTime);
```

**SRS_AGENT_TIME_30_002: [** The `time_t` values in this interface shall be seconds since 00:00 hours, Jan 1, 1970 UTC. **]**

**SRS_AGENT_TIME_30_003: [** The `get_gmtime`,  `get_mktime`, and  `get_ctime` functions in are deprecated and shall not be used. **]**
