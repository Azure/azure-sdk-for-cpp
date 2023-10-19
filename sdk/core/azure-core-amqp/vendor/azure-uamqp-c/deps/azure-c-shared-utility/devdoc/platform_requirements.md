# platform


## Overview

This document specifies the **platform** adapter for the Azure IoT C SDK. The purpose of _platform_ is
to provide any global init and de-init that may be required, such as `WSAStartup` and `WSACleanup`
for Windows. It also provides the SDK with the proper TLSIO adapter via `platform_get_default_tlsio`.

Although the platform adapter provides a mechanism for performing global init and de-init, device
implementers
may find it makes more sense to perform these operations outside of the scope of the Azure IoT SDK.
In that case, the `platform_init` and `platform_deinit` calls may be left empty.

### References 
[Azure IoT porting guide](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/porting_guide.md)<br/>
[platform.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/platform.h)<br/>
[xio.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)


###   Exposed API
The platform adapter must implement 3 of the 4 functions defined in
[platform.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/platform.h):
`platform_init`, `platform_deinit`, and `platform_get_default_tlsio`. The fourth function, 
`platform_get_platform_info`, is not used by the SDK and my be omitted.

###   platform_init

The `platform_init` call performs any global initialization necessary for a particular platform.

```c
int platform_init();
```

**SRS_PLATFORM_30_000: [** The `platform_init` call shall perform any global initialization needed by the platform and return 0 on success. **]**

**SRS_PLATFORM_30_001: [** On failure, `platform_init` shall return a non-zero value. **]**


###   platform_deinit

The `platform_deinit` call performs any global initialization necessary for a particular platform.

```c
void platform_deinit();
```

**SRS_PLATFORM_30_010: [** The `platform_deinit` call shall perform any global deinitialization needed by the platform. **]**


###   platform_get_default_tlsio

This call returns the `IO_INTERFACE_DESCRIPTION*` for the platform's tlsio as defined in
[xio.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h).



```c
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void);
```

**SRS_PLATFORM_30_020: [** The `platform_get_default_tlsio` call shall return the `IO_INTERFACE_DESCRIPTION*` for the platform's tlsio. **]**
