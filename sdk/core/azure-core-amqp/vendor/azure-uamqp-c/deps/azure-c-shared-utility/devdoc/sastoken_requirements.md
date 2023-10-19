sastoken requirements
================

## Overview

This function is used to create a SAS token.

## References
[Keyed-Hashing for Message Authentication, RFC2104](https://www.ietf.org/rfc/rfc2104.txt)
[SASToken format](https://azure.microsoft.com/documentation/articles/storage-dotnet-shared-access-signature-part-1/)


## Exposed API
```c
    MOCKABLE_FUNCTION(, bool, SASToken_Validate, STRING_HANDLE, sasToken);
    MOCKABLE_FUNCTION(, STRING_HANDLE, SASToken_Create, STRING_HANDLE, key, STRING_HANDLE, scope, STRING_HANDLE, keyName, size_t, expiry);
```

### SASToken_Create
```c
extern STRING_HANDLE SASToken_Create(STRING_HANDLE key, STRING_HANDLE scope, STRING_HANDLE keyName, size_t expiry);
```

**SRS_SASTOKEN_06_001: [** If key is NULL then SASToken_Create shall return NULL. **]**

**SRS_SASTOKEN_06_003: [** If scope is NULL then SASToken_Create shall return NULL. **]**

**SRS_SASTOKEN_06_007: [** keyName is optional and can be set to NULL. **]**

**SRS_SASTOKEN_06_029: [** The key parameter is decoded from base64. **]**

**SRS_SASTOKEN_06_030: [** If there is an error in the decoding then SASToken_Create shall return NULL. **]** The decoded value shall henceforth be known as decodedKey.
The expiry argument shall be converted to a char* by invoking size_tToString.

**SRS_SASTOKEN_06_026: [** If the conversion to string form fails for any reason then SASToken_Create shall return NULL. **]** The string shall be henceforth referred to as tokenExpirationTime.

**SRS_SASTOKEN_06_009: [** The scope is the basis for creating a STRING_HANDLE. **]**

**SRS_SASTOKEN_06_010: [** A "\n" is appended to that string. **]**

**SRS_SASTOKEN_06_011: [** tokenExpirationTime is appended to that string. **]** This is henceforth referred to as toBeHashed.

**SRS_SASTOKEN_06_012: [** An HMAC256 hash is calculated using the decodedKey, over toBeHashed. **]**

**SRS_SASTOKEN_06_013: [** If an error is returned from the HMAC256 function then NULL is returned from SASToken_Create. **]**

**SRS_SASTOKEN_06_014: [** If there are any errors from the following operations then NULL shall be returned. **]**

**SRS_SASTOKEN_06_015: [** The hash is base 64 encoded. **]** That (STRING_HANDLE) shall be called base64Signature.

**SRS_SASTOKEN_06_028: [** base64Signature shall be url encoded. **]** This (STRING_HANDLE) shall be called urlEncodedSignature.

**SRS_SASTOKEN_06_016: [** The string "SharedAccessSignature sr=" is the first part of the result of SASToken_Create. **]**

**SRS_SASTOKEN_06_017: [** The scope parameter is appended to result. **]**

**SRS_SASTOKEN_06_018: [** The string "&sig=" is appended to result. **]**

**SRS_SASTOKEN_06_019: [** The string urlEncodedSignature shall be appended to result. **]**

**SRS_SASTOKEN_06_020: [** The string "&se=" shall be appended to result. **]**

**SRS_SASTOKEN_06_021: [** tokenExpirationTime is appended to result. **]**

**SRS_SASTOKEN_06_022: [** If keyName is non-NULL, the string "&skn=" is appended to result. **]**

**SRS_SASTOKEN_06_023: [** If keyName is non-NULL, the argument keyName is appended to result. **]**
result is returned.

### SASToken_Validate
```c
extern bool SASToken_Validate(STRING_HANDLE handle);
```

**SRS_SASTOKEN_25_024: [** If handle is NULL then SASToken_Validate shall return false. **]**

**SRS_SASTOKEN_25_025: [** SASToken_Validate shall get the SASToken value by invoking STRING_c_str on the handle. **]**

**SRS_SASTOKEN_25_026: [** If STRING_c_str on handle return NULL then SASToken_Validate shall return false. **]**

**SRS_SASTOKEN_25_027: [** If SASTOKEN does not obey the SASToken format then SASToken_Validate shall return false. **]**

**SRS_SASTOKEN_25_028: [** SASToken_validate shall check for the presence of sr, se and sig from the token and return false if not found **]**

**SRS_SASTOKEN_25_029: [** SASToken_validate shall check for expiry time from token and if token has expired then would return false **]**

**SRS_SASTOKEN_25_030: [** SASToken_validate shall return true only if the format is obeyed and the token has not yet expired **]**

**SRS_SASTOKEN_25_031: [** If malloc fails during validation then SASToken_Validate shall return false. **]**
