sntp_lwip
=================

## Overview

**sntp_lwip** implements Azure IoT C Shared Utility `sntp` adapter on devices running lwIP.
The `sntp` adapter is intentionally lightweight
and aimed at small devices because NTP's time setting function is taken care of by the OS on larger machines.

## References

[lwIP](http://savannah.nongnu.org/projects/lwip/)

###   Exposed API

**SRS_SNTP_LWIP_30_001: [** The ntp_lwip shall implement the methods defined in `sntp.h`.
```c
int SNTP_SetServerName(const char* serverName);
int SNTP_Init();
void SNTP_Deinit();
```
 **]**


###   SNTP_SetServerName
 `SNTP_SetServerName` must be called before `SNTP_Init`. The character array pointed to by `serverName` parameter must persist between calls to `SNTP_SetServerName` and `SNTP_Deinit` because the char* is stored and no copy of the string is made.

 `SNTP_SetServerName` is a wrapper for the lwIP call `sntp_setservername` and defers parameter validation to the lwIP library.

 Future implementations of this adapter may allow multiple calls to `SNTP_SetServerName` in order to support multiple servers.

```c
int SNTP_SetServerName(const char* serverName);
```

**SRS_SNTP_LWIP_30_002: [** The `serverName` parameter shall be an NTP server URL which shall not be validated. (Validation is deferred to the underlying library.) **]**

**SRS_SNTP_LWIP_30_003: [** The `SNTP_SetServerName` shall set the NTP server
to be used by ntp_lwip and return 0 to indicate success. (lwIP has no failure path.) **]**  



###   SNTP_Init

`SNTP_Init` initializes the SNTP client and begins the process of contacting the NTP server. It blocks until NTP time has been successfully received.

```c
int SNTP_Init();
```
**SRS_SNTP_LWIP_30_004: [** `SNTP_Init` shall initialize the SNTP client, contact the NTP server to set system time, then return 0 to indicate success (lwIP has no failure path). **]**


###   SNTP_Denit

`SNTP_Denit` deinitializes the SNTP client.

```c
void SNTP_Denit();
```

**SRS_SNTP_LWIP_30_005: [** `SNTP_Denit` shall deinitialize the SNTP client. **]**
