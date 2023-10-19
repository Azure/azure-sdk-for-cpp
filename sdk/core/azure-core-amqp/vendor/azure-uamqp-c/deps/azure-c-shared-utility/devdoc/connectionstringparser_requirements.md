#connectionstringparser requirements
====================================

## Overview

connection_parser is module that parses key/value pairs from a connection string.
The format of the connection string is:
[key1]=[value1];[key2]=[value2]; ...

## Exposed API

```c
extern MAP_HANDLE connectionstringparser_parse_from_char(const char* connection_string);
extern MAP_HANDLE connectionstringparser_parse(STRING_HANDLE connection_string);
extern int connectionstringparser_splitHostName_from_char(const char* hostName, STRING_HANDLE nameString, STRING_HANDLE suffixString);
extern int connectionstringparser_splitHostName(STRING_HANDLE hostNameString, STRING_HANDLE nameString, STRING_HANDLE suffixString);
```

### connectionstringparser_parse

```c
extern MAP_HANDLE connectionstringparser_parse(STRING_HANDLE connection_string);
```

**SRS_CONNECTIONSTRINGPARSER_01_001: [** connectionstringparser_parse shall parse all key value pairs from the connection_string passed in as argument and return a new map that holds the key/value pairs. **]**

**SRS_CONNECTIONSTRINGPARSER_01_018: [** If creating the result map fails, then connectionstringparser_parse shall return NULL. **]**

**SRS_CONNECTIONSTRINGPARSER_01_002: [** If connection_string is NULL then connectionstringparser_parse shall fail and return NULL. **]**

**SRS_CONNECTIONSTRINGPARSER_01_003: [** connectionstringparser_parse shall create a STRING tokenizer to be used for parsing the connection string, by calling STRING_TOKENIZER_create. **]**

**SRS_CONNECTIONSTRINGPARSER_01_015: [** If STRING_TOKENIZER_create fails, connectionstringparser_parse shall fail and return NULL. **]**

**SRS_CONNECTIONSTRINGPARSER_01_004: [** connectionstringparser_parse shall start scanning at the beginning of the connection string. **]**

**SRS_CONNECTIONSTRINGPARSER_01_016: [** 2 STRINGs shall be allocated in order to hold the to be parsed key and value tokens. **]**

**SRS_CONNECTIONSTRINGPARSER_01_017: [** If allocating the STRINGs fails connectionstringparser_parse shall fail and return NULL. **]**

**SRS_CONNECTIONSTRINGPARSER_01_005: [** The following actions shall be repeated until parsing is complete: **]**

**SRS_CONNECTIONSTRINGPARSER_01_006: [** connectionstringparser_parse shall find a token (the key of the key/value pair) delimited by the `=` character, by calling STRING_TOKENIZER_get_next_token. **]**

**SRS_CONNECTIONSTRINGPARSER_01_007: [** If STRING_TOKENIZER_get_next_token fails, parsing shall be considered complete. **]**

**SRS_CONNECTIONSTRINGPARSER_01_008: [** connectionstringparser_parse shall find a token (the value of the key/value pair) delimited by the `;` character, by calling STRING_TOKENIZER_get_next_token. **]**

**SRS_CONNECTIONSTRINGPARSER_01_009: [** If STRING_TOKENIZER_get_next_token fails, connectionstringparser_parse shall fail and return NULL (freeing the allocated result map). **]**

**SRS_CONNECTIONSTRINGPARSER_01_010: [** The key and value shall be added to the result map by using Map_Add. **]**

**SRS_CONNECTIONSTRINGPARSER_01_011: [** The C strings for the key and value shall be extracted from the previously parsed STRINGs by using STRING_c_str. **]**

**SRS_CONNECTIONSTRINGPARSER_01_019: [** If the key length is zero then connectionstringparser_parse shall fail and return NULL (freeing the allocated result map). **]**

**SRS_CONNECTIONSTRINGPARSER_01_012: [** If Map_Add fails connectionstringparser_parse shall fail and return NULL (freeing the allocated result map). **]**

**SRS_CONNECTIONSTRINGPARSER_01_013: [** If STRING_c_str fails then connectionstringparser_parse shall fail and return NULL (freeing the allocated result map). **]**

**SRS_CONNECTIONSTRINGPARSER_01_014: [** After the parsing is complete the previously allocated STRINGs and STRING tokenizer shall be freed by calling STRING_TOKENIZER_destroy. **]**  


### connectionstringparser_parse_from_char

```c
extern MAP_HANDLE connectionstringparser_parse_from_char(const char* connection_string);
```

**SRS_CONNECTIONSTRINGPARSER_21_020: [** connectionstringparser_parse_from_char shall create a STRING_HANDLE from the connection_string passed in as argument and parse it using the connectionstringparser_parse. **]**

**SRS_CONNECTIONSTRINGPARSER_21_021: [** If connectionstringparser_parse_from_char get error creating a STRING_HANDLE, it shall return NULL. **]**  


### connectionstringparser_splitHostName_from_char

```c
extern int connectionstringparser_splitHostName_from_char(const char* hostName, STRING_HANDLE nameString, STRING_HANDLE suffixString);
```

**SRS_CONNECTIONSTRINGPARSER_21_022: [** connectionstringparser_splitHostName_from_char shall split the provided hostName in name and suffix. **]**

**SRS_CONNECTIONSTRINGPARSER_21_023: [** connectionstringparser_splitHostName_from_char shall copy all characters, from the beginning of the hostName to the first `.` to the nameString. **]**

**SRS_CONNECTIONSTRINGPARSER_21_024: [** connectionstringparser_splitHostName_from_char shall copy all characters, from the first `.` to the end of the hostName, to the suffixString. **]**

**SRS_CONNECTIONSTRINGPARSER_21_025: [** If connectionstringparser_splitHostName_from_char get success splitting the hostName, it shall return 0. **]**

**SRS_CONNECTIONSTRINGPARSER_21_026: [** If the hostName is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_027: [** If the hostName is an empty string, connectionstringparser_splitHostName_from_char shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_028: [** If the nameString is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_029: [** If the suffixString is NULL, connectionstringparser_splitHostName_from_char shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_030: [** If the hostName is not a valid host name, connectionstringparser_splitHostName_from_char shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_031: [** If connectionstringparser_splitHostName_from_char get error copying the name to the nameString, it shall return MU_FAILURE. **]**

**SRS_CONNECTIONSTRINGPARSER_21_032: [** If connectionstringparser_splitHostName_from_char get error copying the suffix to the suffixString, it shall return MU_FAILURE. **]**  


### connectionstringparser_splitHostName

```c
extern int connectionstringparser_splitHostName(STRING_HANDLE hostNameString, STRING_HANDLE nameString, STRING_HANDLE suffixString);
```

**SRS_CONNECTIONSTRINGPARSER_21_033: [** connectionstringparser_splitHostName shall convert the hostNameString to a connection_string passed in as argument, and call connectionstringparser_splitHostName_from_char. **]**

**SRS_CONNECTIONSTRINGPARSER_21_034: [** If the hostNameString is NULL, connectionstringparser_splitHostName shall return MU_FAILURE. **]**  
