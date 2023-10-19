
﻿# umockstring requirements

# Overview

umockstring is a module that provides the functionality of cloning a string (allocating memory and copying the chars).

# Exposed API

```c
char* umockstring_clone(const char* source);
```

## umockstring_clone

```c
char* umockstring_clone(const char* source);
```

**UMOCK_STRING_01_001: [** `umockstring_clone` shall allocate memory for the cloned string (including the NULL terminator). **]**

**UMOCK_STRING_01_002: [** `umockstring_clone` shall copy the string to the newly allocated memory (including the NULL terminator). **]**

**UMOCK_STRING_01_003: [** On success `umockstring_clone` shall return a pointer to the newly allocated memory containing the copy of the string. **]**

**UMOCK_STRING_01_004: [** If allocating the memory fails, `umockstring_clone` shall return NULL. **]**

**UMOCK_STRING_01_005: [** If `umockstring_clone` is called with a NULL `source`, it shall return NULL. **]**
