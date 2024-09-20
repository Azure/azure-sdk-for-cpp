# sasl_plain requirements
 
## Overview

`sasl_plain` is a module that implements the sasl mechanism PLAIN so that it can be used through the `sasl_mechanism` interface. The spec conforms to https://tools.ietf.org/html/rfc4616.

## Exposed API

```C
	typedef struct SASL_PLAIN_CONFIG_TAG
	{
		const char* authcid;
		const char* passwd;
        const char* authzid;
	} SASL_PLAIN_CONFIG;

	MOCKABLE_FUNCTION(, const SASL_MECHANISM_INTERFACE_DESCRIPTION*, saslplain_get_interface);
```

### saslplain_create

```C
CONCRETE_SASL_MECHANISM_HANDLE saslplain_create(void* config);
```

`saslplain_create` is the implementation provided via `saslplain_get_interface` for the `concrete_sasl_mechanism_create` member.

**SRS_SASL_PLAIN_01_001: [**`saslplain_create` shall return on success a non-NULL handle to a new SASL plain mechanism.**]** 

**SRS_SASL_PLAIN_01_002: [**If allocating the memory needed for the saslplain instance fails then `saslplain_create` shall return NULL.**]** 

**SRS_SASL_PLAIN_01_003: [**If the `config` argument is NULL, then `saslplain_create` shall fail and return NULL.**]** 

**SRS_SASL_PLAIN_01_004: [**If either the `authcid` or `passwd` member of the `config` structure is NULL, then `saslplain_create` shall fail and return NULL.**]** 

**SRS_SASL_PLAIN_01_025: [** `authzid` shall be optional. **]**

### saslplain_destroy

```C
void saslplain_destroy(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism_concrete_handle);
```

`saslplain_destroy` is the implementation provided via `saslplain_get_interface` for the `concrete_sasl_mechanism_destroy` member.

**SRS_SASL_PLAIN_01_005: [**`saslplain_destroy` shall free all resources associated with the SASL mechanism.**]** 

**SRS_SASL_PLAIN_01_006: [**If the argument `concrete_sasl_mechanism` is NULL, `saslplain_destroy` shall do nothing.**]**

### saslplain_get_init_bytes

```C
int saslplain_get_init_bytes(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, INIT_BYTES* init_bytes);
```

`saslplain_get_init_bytes` is the implementation provided via `saslplain_get_interface` for the `concrete_sasl_mechanism_get_init_bytes` member.

**SRS_SASL_PLAIN_01_007: [**`saslplain_get_init_bytes` shall construct the initial bytes per the RFC 4616.**]** 

**SRS_SASL_PLAIN_01_008: [**On success `saslplain_get_init_bytes` shall return zero.**]** 

**SRS_SASL_PLAIN_01_009: [**If any argument is NULL, `saslplain_get_init_bytes` shall return a non-zero value.**]**

### saslplain_get_mechanism_name

```C
const char* saslplain_get_mechanism_name(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism);
```

`saslplain_get_mechanism_name` is the implementation provided via `saslplain_get_interface` for the `concrete_sasl_mechanism_get_mechanism_name` member.

**SRS_SASL_PLAIN_01_010: [**`saslplain_get_mechanism_name` shall validate the argument `concrete_sasl_mechanism` and on success it shall return a pointer to the string "PLAIN".**]** 

**SRS_SASL_PLAIN_01_011: [**If the argument `concrete_sasl_mechanism` is NULL, `saslplain_get_mechanism_name` shall return NULL.**]** 

### saslplain_challenge

```C
int saslplain_challenge(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, const SASL_MECHANISM_BYTES* challenge_bytes, SASL_MECHANISM_BYTES* response_bytes);
```

`saslplain_challenge` is the implementation provided via `saslplain_get_interface` for the `concrete_sasl_mechanism_challenge` member.

**SRS_SASL_PLAIN_01_012: [**`saslplain_challenge` shall set the `response_bytes` buffer to NULL and 0 size as the PLAIN SASL mechanism does not implement challenge/response.**]** 

**SRS_SASL_PLAIN_01_013: [**On success, `saslplain_challenge` shall return 0.**]** 

**SRS_SASL_PLAIN_01_014: [**If the `concrete_sasl_mechanism` or `response_bytes` argument is NULL then `saslplain_challenge` shall fail and return a non-zero value.**]** 

### saslplain_get_interface

```C
MOCKABLE_FUNCTION(, const SASL_MECHANISM_INTERFACE_DESCRIPTION*, saslplain_get_interface);
```

**SRS_SASL_PLAIN_01_015: [**`saslplain_get_interface` shall return a pointer to a `SASL_MECHANISM_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `saslplain_create`, `saslplain_destroy`, `saslplain_get_init_bytes`, `saslplain_get_mechanism_name`, `saslplain_challenge`.**]** 

## RFC section

2.  PLAIN SASL Mechanism

**SRS_SASL_PLAIN_01_016: [**The mechanism consists of a single message, a string of [UTF-8] encoded [Unicode] characters, from the client to the server.**]** 
**SRS_SASL_PLAIN_01_017: [**The client presents the authorization identity (identity to act as), followed by a NUL (U+0000) character, followed by the authentication identity (identity whose password will be used), followed by a NUL (U+0000) character, followed by the clear-text password.**]** 
**SRS_SASL_PLAIN_01_018: [**As with other SASL mechanisms, the client does not provide an authorization identity when it wishes the server to derive an identity from the credentials and use that as the authorization identity.**]** 

The formal grammar for the client message using Augmented BNF [ABNF] follows.

**SRS_SASL_PLAIN_01_019: [**   message   = [authzid] UTF8NUL authcid UTF8NUL passwd**]** 
**SRS_SASL_PLAIN_01_020: [**   authcid   = 1*SAFE ; MUST accept up to 255 octets**]** 
**SRS_SASL_PLAIN_01_021: [**   authzid   = 1*SAFE ; MUST accept up to 255 octets**]** 
[SuppressMessage("Microsoft.Security", "CS002:SecretInNextLine", Justification="Not a password")]
**SRS_SASL_PLAIN_01_022: [**   passwd    = 1*SAFE ; MUST accept up to 255 octets**]** 
   UTF8NUL   = %x00 ; UTF-8 encoded NUL character

   SAFE      = UTF1 / UTF2 / UTF3 / UTF4
               ;; any UTF-8 encoded Unicode character except NUL

   UTF1      = %x01-7F ;; except NUL
   UTF2      = %xC2-DF UTF0
   UTF3      = %xE0 %xA0-BF UTF0 / %xE1-EC 2(UTF0) /
               %xED %x80-9F UTF0 / %xEE-EF 2(UTF0)
   UTF4      = %xF0 %x90-BF 2(UTF0) / %xF1-F3 3(UTF0) /
               %xF4 %x80-8F 2(UTF0)
   UTF0      = %x80-BF

**SRS_SASL_PLAIN_01_023: [**The authorization identity (authzid), authentication identity (authcid), password (passwd), and NUL character deliminators SHALL be transferred as [UTF-8**]** encoded strings of [Unicode] characters.] 
**SRS_SASL_PLAIN_01_024: [**As the NUL (U+0000) character is used as a deliminator, the NUL (U+0000) character MUST NOT appear in authzid, authcid, or passwd productions.**]** 

The form of the authzid production is specific to the application-level protocol's SASL profile [SASL].
The authcid and passwd productions are form-free.
Use of non-visible characters or characters that a user may be unable to enter on some keyboards is discouraged.

Servers MUST be capable of accepting authzid, authcid, and passwd productions up to and including 255 octets.
It is noted that the UTF-8 encoding of a Unicode character may be as long as 4 octets.

Upon receipt of the message, the server will verify the presented (in the message) authentication identity (authcid) and password (passwd) with the system authentication database, and it will verify that the authentication credentials permit the client to act as the (presented or derived) authorization identity (authzid).
If both steps succeed, the user is authenticated.

The presented authentication identity and password strings, as well as the database authentication identity and password strings, are to be prepared before being used in the verification process.
The [SASLPrep] profile of the [StringPrep] algorithm is the RECOMMENDED preparation algorithm. The SASLprep preparation algorithm is recommended to improve the likelihood that comparisons behave in an expected manner.
The SASLprep preparation algorithm is not mandatory so as to allow the server to employ other preparation algorithms (including none) when appropriate.
For instance, use of a different preparation algorithm may be necessary for the server to interoperate with an external system.

When preparing the presented strings using [SASLPrep], the presented strings are to be treated as "query" strings (Section 7 of [StringPrep]) and hence unassigned code points are allowed to appear in their prepared output.
When preparing the database strings using [SASLPrep], the database strings are to be treated as "stored" strings (Section 7 of [StringPrep]) and hence unassigned code points are prohibited from appearing in their prepared output.

Regardless of the preparation algorithm used, if the output of a non-invertible function (e.g., hash) of the expected string is stored, the string MUST be prepared before input to that function.

Regardless of the preparation algorithm used, if preparation fails or results in an empty string, verification SHALL fail.

When no authorization identity is provided, the server derives an authorization identity from the prepared representation of the provided authentication identity string.
This ensures that the derivation of different representations of the authentication identity produces the same authorization identity.

The server MAY use the credentials to initialize any new authentication database, such as one suitable for [CRAM-MD5] or [DIGEST-MD5].
