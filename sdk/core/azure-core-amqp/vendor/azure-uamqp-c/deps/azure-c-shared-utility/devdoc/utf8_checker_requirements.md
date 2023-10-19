# utf8_checker requirements

## Overview

utf8_checker is module that provides basic validation whether a string is a UTF-8 string.

## References

[Unicode spec chapter 3.9](http://www.unicode.org/versions/Unicode9.0.0/ch03.pdf#G7404)

## Exposed API

```c
MOCKABLE_FUNCTION(, bool, utf8_checker_is_valid_utf8, const unsigned char*, utf8_str, size_t, length);
```

###  utf8_checker_is_valid_utf8

```c
extern bool utf8_checker_is_valid_utf8(const unsigned char* utf8_str, size_t length);
```

**SRS_UTF8_CHECKER_01_001: [** `utf8_checker_is_valid_utf8` shall verify that the sequence of chars pointed to by `utf8_str` represent UTF-8 encoded codepoints. **]**

**SRS_UTF8_CHECKER_01_005: [** On success it shall return true. **]**

**SRS_UTF8_CHECKER_01_002: [** If `utf8_checker_is_valid_utf8` is called with NULL `utf8_str` it shall return false. **]**

**SRS_UTF8_CHECKER_01_003: [** If `length` is 0, `utf8_checker_is_valid_utf8` shall consider `utf8_str` to be valid UTF-8 and return true. **]**

###  Relevant Unicode spec table

Scalar Value First Byte Second Byte Third Byte Fourth Byte
**SRS_UTF8_CHECKER_01_006: [** 00000000 0xxxxxxx 0xxxxxxx **]**

**SRS_UTF8_CHECKER_01_007: [** 00000yyy yyxxxxxx 110yyyyy 10xxxxxx **]**

**SRS_UTF8_CHECKER_01_008: [** zzzzyyyy yyxxxxxx 1110zzzz 10yyyyyy 10xxxxxx **]**

**SRS_UTF8_CHECKER_01_009: [** 000uuuuu zzzzyyyy yyxxxxxx 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx **]**
