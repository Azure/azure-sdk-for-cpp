# memory data
====

## Overview

`memory_data` is a module that reads/writes 8/16/32/64 bytes from memory to/from int8_t/int16_t/int32_t/int64_t and their unsigned versions.
`memory_data` does not perform any NULL checks on the arguments. It is intended to behave like other memory manipulation functions from the C standard (memcpy, etc.) and thus the address 0 is not treated in any special way.

## Exposed API

```c
MOCKABLE_INTERFACE(memory_data,
    FUNCTION(, void, read_uint8_t,  const unsigned char*, source, uint8_t*, destination),
    FUNCTION(, void, read_uint16_t, const unsigned char*, source, uint16_t*, destination),
    FUNCTION(, void, read_uint32_t, const unsigned char*, source, uint32_t*, destination),
    FUNCTION(, void, read_uint64_t, const unsigned char*, source, uint64_t*, destination),

    FUNCTION(, void, read_int8_t,  const unsigned char*, source,  int8_t*,  destination),
    FUNCTION(, void, read_int16_t, const unsigned char*, source, int16_t*, destination),
    FUNCTION(, void, read_int32_t, const unsigned char*, source, int32_t*, destination),
    FUNCTION(, void, read_int64_t, const unsigned char*, source, int64_t*, destination),

    FUNCTION(, void, read_uuid_t, const unsigned char*, source, UUID_T*, destination),

    FUNCTION(, void, write_uint8_t,  unsigned char*, destination, uint8_t,  value),
    FUNCTION(, void, write_uint16_t, unsigned char*, destination, uint16_t, value),
    FUNCTION(, void, write_uint32_t, unsigned char*, destination, uint32_t, value),
    FUNCTION(, void, write_uint64_t, unsigned char*, destination, uint64_t, value),

    FUNCTION(, void, write_int8_t,  unsigned char*, destination, int8_t,  value),
    FUNCTION(, void, write_int16_t, unsigned char*, destination, int16_t, value),
    FUNCTION(, void, write_int32_t, unsigned char*, destination, int32_t, value),
    FUNCTION(, void, write_int64_t, unsigned char*, destination, int64_t, value),

    FUNCTION(, void, write_uuid_t, unsigned char*, destination, const UUID_T, value)
)
```

### read_uint8_t
```cs
FUNCTION(, void, read_uint8_t,  const unsigned char*, source, uint8_t*  destination);
```

`read_uint8_t` reads a uint8_t from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_041: [** `read_uint8_t` shall write in `destination` the byte at `source` **]**

### read_uint16_t
```c
FUNCTION(, void, read_uint16_t,  const unsigned char*, source, uint16_t*  destination);
```

`read_uint16_t` reads a uint16_t from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_042: [** `read_uint16_t` shall write in `destination` the bytes at `source` MSB first and return. **]**

### read_uint32_t
```c
FUNCTION(, void, read_uint32_t,  const unsigned char*, source, uint32_t*  destination);
```

`read_uint32_t` reads a uint32_t from `source` and writes it into `destination`. 

**SRS_MEMORY_DATA_02_043: [** `read_uint32_t` shall  write in `destination` the bytes at `source` MSB first. **]**

### read_uint64_t
```c
FUNCTION(, void, read_uint64_t,  const unsigned char*, source, uint64_t*  destination);
```

`read_uint64_t` reads a uint64_t from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_044: [** `read_uint64_t` shall write in `destination` the bytes at `source` MSB first. **]**

### read_int8_t
```c
FUNCTION(, void, read_int8_t,  const unsigned char*, source, int8_t*  destination);
```

`read_int8_t` reads a int8_t from `source` and writes it into `destination`.

 **SRS_MEMORY_DATA_02_045: [** `read_int8_t` shall  write in `destination` the signed byte at `source`. **]**

### read_int16_t
```c
FUNCTION(, void, read_int16_t,  const unsigned char*, source, int16_t*  destination);
```

`read_int16_t` reads a int16_t from `source` and writes it into `destination`. 

**SRS_MEMORY_DATA_02_046: [** `read_int16_t` shall write in `destination` the bytes at `source` MSB first. **]**

### read_int32_t
```c
FUNCTION(, void, read_int32_t,  const unsigned char*, source, int32_t*  destination);
```

`read_int32_t` reads a int32_t from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_047: [** `read_int32_t` shall write in `destination` the bytes at `source` MSB first. **]**

### read_int64_t
```c
FUNCTION(, void, read_int64_t,  const unsigned char*, source, int64_t*  destination);
```

`read_int64_t` reads a int64_t from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_048: [** `read_int64_t` shall write in `destination` the bytes at `source` MSB first. **]**

### read_uuid_t

```c
FUNCTION(, void, read_uuid_t, unsigned char*, source, UUID_T*, destination);
```

`read_uuid_t` reads a UUID_T from `source` and writes it into `destination`.

**SRS_MEMORY_DATA_02_049: [** `read_uuid_t` shall write in `destination` the bytes at `source`. **]**

### write_uint8_t
```c
FUNCTION(, void,  write_uint8_t,  unsigned char*, destination, uint8_t  value);
```

`write_uint8_t` writes at `destination` the byte of `value`.

**SRS_MEMORY_DATA_02_050: [** `write_uint8_t` shall write in `destination` the byte of `value`. **]**

### write_uint16_t
```c
FUNCTION(, void,  write_uint16_t,  unsigned char*, destination, uint16_t  value);
```

`write_uint16_t` writes at `destination` the bytes of `value` MSB first.

**SRS_MEMORY_DATA_02_051: [** `write_uint16_t` shall write in `destination` the bytes of value MSB first. **]**

### write_uint32_t
```c
FUNCTION(, void,  write_uint32_t,  unsigned char*, destination, uint32_t  value);
```

`write_uint32_t` writes at `destination` the bytes of `value` MSB first.

**SRS_MEMORY_DATA_02_052: [** `write_uint32_t` shall write in `destination` the bytes of value MSB first. **]**

### write_uint64_t
```c
FUNCTION(, void,  write_uint64_t,  unsigned char*, destination, uint64_t  value);
```

`write_uint64_t` writes at `destination` the bytes of `value` MSB first.

**SRS_MEMORY_DATA_02_053: [** `write_uint64_t` shall write in `destination` the bytes of value MSB first. **]**

### write_int8_t
```c
FUNCTION(, void,  write_int8_t,  unsigned char*, destination, int8_t  value);
```

`write_int8_t` writes a int8_t at `destination`.

**SRS_MEMORY_DATA_02_054: [** `write_int8_t` shall write at `destination` the byte of `value`. **]**
    
### write_int16_t
```c
FUNCTION(, void,  write_int16_t,  unsigned char*, destination, int16_t  value);
```

`write_int16_t` writes a int16_t at `destination`.

**SRS_MEMORY_DATA_02_055: [** `write_int16_t` shall write at `destination` the bytes of `value` starting with MSB. **]**

### write_int32_t
```c
FUNCTION(, void,  write_int32_t,  unsigned char*, destination, int32_t  value);
```

`write_int32_t` writes a int32_t at `destination`.

**SRS_MEMORY_DATA_02_056: [** `write_int32_t` shall write at `destination` the bytes of `value` starting with MSB **]**
    
### write_int64_t
```c
FUNCTION(, void,  write_int64_t,  unsigned char*, destination, int64_t  value);
```

`write_int64_t` writes a int64_t at `destination`.

**SRS_MEMORY_DATA_02_057: [** `write_int64_t` shall write at `destination` the bytes of `value` starting with MSB. **]**
    
### write_uuid_t

```c
FUNCTION(, void,  write_uuid_t, unsigned char*, destination, const UUID_T, value);
```

`write_uuid_t` writes a UUID_T at `destination`.

**SRS_MEMORY_DATA_02_058: [** `write_uuid_t` shall write at `destination` the bytes of `value` **]**
