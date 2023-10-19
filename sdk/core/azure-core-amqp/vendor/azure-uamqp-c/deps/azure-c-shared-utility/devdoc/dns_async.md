dns_resolver
=================

## Overview

**dns_resolver** performs an asynchronous lookup of a TCP IPv4 address given a host name.

This module is intended to locate IP addresses for an Azure server, and more flexible behavior is deliberately out-of-scope at this time. IPv6 address lookup is currently out-of-scope, although support for it may be added in the future via addition of a `dns_resolver_get_ipv6` call.

The present implementation will not actually provide asynchronous behavior, which is a feature to be added in the future.
## References

[dns_resolver.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/dns_resolver.h)  

###   Exposed API


**SRS_dns_resolver_30_001: [** The dns_resolver shall use the constants and types defined in `dns_resolver.h`.
```c
typedef void* DNSRESOLVER_HANDLE;

// If options are added in future, DNSRESOLVER_OPTIONS will become a struct containing the options
typedef void DNSRESOLVER_OPTIONS;
```
 **]**

**SRS_dns_resolver_30_002: [** The dns_resolver shall implement the methods defined in `dns_resolver.h`.
```c
DNSRESOLVER_HANDLE dns_resolver_create(const char* hostname, int port, const DNSRESOLVER_OPTIONS* options);
int dns_resolver_is_lookup_complete(DNSRESOLVER_HANDLE dns, bool* is_complete);
uint32_t dns_resolver_get_ipv4(DNSRESOLVER_HANDLE dns);
void dns_resolver_destroy(DNSRESOLVER_HANDLE dns);
```
 **]**


###   dns_resolver_create
`dns_resolver_create` begins a single attempt at asynchronous DNS lookup.
```c
DNSRESOLVER_HANDLE dns_resolver_create(const char* hostname, int port, const DNSRESOLVER_OPTIONS* options);
```

**SRS_dns_resolver_30_010: [** `dns_resolver_create` shall make a copy of the `hostname` parameter to allow immediate deletion by the caller. **]**

**SRS_dns_resolver_30_011: [** If the `hostname` parameter is `NULL`, `dns_resolver_create` shall log an error and return `NULL`. **]**

**SRS_dns_resolver_30_012: [** The optional `options` parameter shall be ignored. **]**

**SRS_dns_resolver_30_013: [** On success, `dns_resolver_create` shall return the created `DNSRESOLVER_HANDLE`. **]**

**SRS_dns_resolver_30_014: [** On any failure, `dns_resolver_create` shall log an error and return `NULL`. **]**


###   dns_resolver_is_lookup_complete
`dns_resolver_is_lookup_complete` tests whether `dns_resolver_create`'s single attempt at DNS lookup has been completed. To complete the lookup process, this method must be called repeatedly until it returns `true`.

```c
bool dns_resolver_is_create_complete(DNSRESOLVER_HANDLE dns);
```

**SRS_dns_resolver_30_020: [** If the `dns` parameter is NULL, `dns_resolver_is_create_complete` shall log an error and return `false`. **]**

**SRS_dns_resolver_30_021: [** `dns_resolver_is_create_complete` shall perform the asynchronous work of DNS lookup and log any errors. **]**

**SRS_dns_resolver_30_022: [** If the DNS lookup process has completed, `dns_resolver_is_create_complete` shall return `true`. **]**

**SRS_dns_resolver_30_023: [** If the DNS lookup process is not yet complete, `dns_resolver_is_create_complete` shall return `false`. **]**

**SRS_dns_resolver_30_024: [** If `dns_resolver_is_create_complete` has previously returned `true`, `dns_resolver_is_create_complete` shall do nothing and return `true`. **]**


###   dns_resolver_get_ipv4
`dns_resolver_get_ipv4` retrieves the IP address address after `dns_resolver_is_create_complete` indicates completion. A return value of 0 indicates failure.

```c
uint32_t dns_resolver_get_ipv4(DNSRESOLVER_HANDLE dns);
```

**SRS_dns_resolver_30_030: [** If the `dns` parameter is NULL, `dns_resolver_get_ipv4` shall log an error and return 0. **]**

**SRS_dns_resolver_30_031: [** If `dns_resolver_is_create_complete` has not yet returned `true`, `dns_resolver_get_ipv4` shall log an error and return 0. **]**

**SRS_dns_resolver_30_032: [** If `dns_resolver_is_create_complete` has returned `true` and the lookup process has succeeded, `dns_resolver_get_ipv4` shall return the discovered IPv4 address. **]**

**SRS_dns_resolver_30_033: [** If `dns_resolver_is_create_complete` has returned `true` and the lookup process has failed, `dns_resolver_get_ipv4` shall return 0. **]**


###   dns_resolver_destroy
 `dns_resolver_destroy` releases any resources acquired during the DNS lookup process.

 ```c
 void dns_resolver_destroy(DNSRESOLVER_HANDLE dns);
 ```

**SRS_dns_resolver_30_050: [** If the `dns` parameter is `NULL`, `dns_resolver_destroy` shall log an error and do nothing. **]**  

**SRS_dns_resolver_30_051: [** `dns_resolver_destroy` shall delete all acquired resources and delete the `DNSRESOLVER_HANDLE`. **]**  
