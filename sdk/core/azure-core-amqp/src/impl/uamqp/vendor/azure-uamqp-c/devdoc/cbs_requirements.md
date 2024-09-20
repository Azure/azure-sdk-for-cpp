# cbs requirements

## Overview

`cbs` is module that implements the CBS draft specification.

##Exposed API

```c
#define CBS_OPERATION_RESULT_VALUES \
    CBS_OPERATION_RESULT_OK, \
    CBS_OPERATION_RESULT_CBS_ERROR, \
    CBS_OPERATION_RESULT_OPERATION_FAILED, \
    CBS_OPERATION_RESULT_INSTANCE_CLOSED

DEFINE_ENUM(CBS_OPERATION_RESULT, CBS_OPERATION_RESULT_VALUES)

#define CBS_OPEN_COMPLETE_RESULT_VALUES \
    CBS_OPEN_OK, \
    CBS_OPEN_ERROR, \
    CBS_OPEN_CANCELLED

DEFINE_ENUM(CBS_OPEN_COMPLETE_RESULT, CBS_OPEN_COMPLETE_RESULT_VALUES)

    typedef struct CBS_INSTANCE_TAG* CBS_HANDLE;
    typedef void(*ON_CBS_OPEN_COMPLETE)(void* context, CBS_OPEN_COMPLETE_RESULT open_complete_result);
    typedef void(*ON_CBS_ERROR)(void* context);
    typedef void(*ON_CBS_OPERATION_COMPLETE)(void* context, CBS_OPERATION_RESULT complete_result, unsigned int status_code, const char* status_description);

    MOCKABLE_FUNCTION(, CBS_HANDLE, cbs_create, SESSION_HANDLE, session);
    MOCKABLE_FUNCTION(, void, cbs_destroy, CBS_HANDLE, cbs);
    MOCKABLE_FUNCTION(, int, cbs_open_async, CBS_HANDLE, cbs, ON_CBS_OPEN_COMPLETE, on_cbs_open_complete, void*, on_cbs_open_complete_context, ON_CBS_ERROR, on_cbs_error, void*, on_cbs_error_context);
    MOCKABLE_FUNCTION(, int, cbs_close, CBS_HANDLE, cbs);
    MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, cbs_put_token_async, CBS_HANDLE, cbs, const char*, type, const char*, audience, const char*, token, ON_CBS_OPERATION_COMPLETE, on_cbs_put_token_complete, void*, on_cbs_put_token_complete_context);
    MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, cbs_delete_token_async, CBS_HANDLE, cbs, const char*, type, const char*, audience, ON_CBS_OPERATION_COMPLETE, on_cbs_delete_token_complete, void*, on_cbs_delete_token_complete_context);
    MOCKABLE_FUNCTION(, int, cbs_set_trace, CBS_HANDLE, cbs, bool, trace_on);
```

### cbs_create

```c
MOCKABLE_FUNCTION(, CBS_HANDLE, cbs_create, SESSION_HANDLE, session);
```

**SRS_CBS_01_001: [** `cbs_create` shall create a new CBS instance and on success return a non-NULL handle to it. **]**
**SRS_CBS_01_033: [** If `session` is NULL then `cbs_create` shall fail and return NULL. **]**
**SRS_CBS_01_097: [** `cbs_create` shall create a singly linked list for pending operations by calling `singlylinkedlist_create`. **]**
**SRS_CBS_01_101: [** If `singlylinkedlist_create` fails, `cbs_create` shall fail and return NULL. **]**
**SRS_CBS_01_076: [** If allocating memory for the new handle fails, `cbs_create` shall fail and return NULL. **]**
**SRS_CBS_01_034: [** `cbs_create` shall create an AMQP management handle by calling `amqp_management_create`. **]**
**SRS_CBS_01_035: [** If `amqp_management_create` fails then `cbs_create` shall fail and return NULL. **]**
**SRS_CBS_01_117: [** `cbs_create` shall set the override status code key name on the AMQP management handle to `status-code` by calling `amqp_management_set_override_status_code_key_name`. **]**
**SRS_CBS_01_118: [** `cbs_create` shall set the override status description key name on the AMQP management handle to `status-description` by calling `amqp_management_set_override_status_description_key_name`. **]**
**SRS_CBS_01_116: [** If setting the override key names fails, then `cbs_create` shall fail and return NULL. **]**

### cbs_destroy

```c
MOCKABLE_FUNCTION(, void, cbs_destroy, CBS_HANDLE, cbs);
```

**SRS_CBS_01_036: [** `cbs_destroy` shall free all resources associated with the handle `cbs`. **]**
**SRS_CBS_01_037: [** If `cbs` is NULL, `cbs_destroy` shall do nothing. **]**
**SRS_CBS_01_100: [** If the CBS instance is not closed, all actions performed by `cbs_close` shall be performed. **]**
**SRS_CBS_01_099: [** All pending operations shall be freed. **]**
**SRS_CBS_01_098: [** `cbs_destroy` shall free the pending operations list by calling `singlylinkedlist_destroy`. **]**
**SRS_CBS_01_038: [** `cbs_destroy` shall free the AMQP management handle created in `cbs_create` by calling `amqp_management_destroy`. **]**

### cbs_open_async

```c
MOCKABLE_FUNCTION(, int, cbs_open_async, CBS_HANDLE, cbs, ON_CBS_OPEN_COMPLETE, on_cbs_open_complete, void*, on_cbs_open_complete_context, ON_CBS_ERROR, on_cbs_error, void*, on_cbs_error_context);
```

**SRS_CBS_01_039: [** `cbs_open_async` shall open the cbs communication by calling `amqp_management_open_async` on the AMQP management handle created in `cbs_create`. **]**
**SRS_CBS_01_077: [** On success, `cbs_open_async` shall return 0. **]**
**SRS_CBS_01_040: [** If any of the arguments `cbs`, `on_cbs_open_complete` or `on_cbs_error` is NULL, then `cbs_open_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_041: [** If `amqp_management_open_async` fails, shall fail and return a non-zero value. **]**
**SRS_CBS_01_042: [** `on_cbs_open_complete_context` and `on_cbs_error_context` shall be allowed to be NULL. **]**
**SRS_CBS_01_043: [** `cbs_open_async` while already open or opening shall fail and return a non-zero value. **]**
**SRS_CBS_01_078: [** The cbs instance shall be considered OPENING until the `on_amqp_management_open_complete` callback is called by the AMQP management instance indicating that the open has completed (succesfull or not). **]**

### cbs_close

```c
MOCKABLE_FUNCTION(, int, cbs_close, CBS_HANDLE, cbs);
```

**SRS_CBS_01_044: [** `cbs_close` shall close the CBS instance by calling `amqp_management_close` on the underlying AMQP management handle. **]**
**SRS_CBS_01_080: [** On success, `cbs_close` shall return 0. **]**
**SRS_CBS_01_079: [** If `cbs_close` is called while OPENING, the `on_cbs_open_complete` callback shall be called with `CBS_OPEN_CANCELLED`, while passing the `on_cbs_open_complete_context` as argument. **]**
**SRS_CBS_01_045: [** If the argument `cbs` is NULL, `cbs_close` shall fail and return a non-zero value. **]**
**SRS_CBS_01_046: [** If `amqp_management_close` fails, `cbs_close` shall fail and return a non-zero value. **]**
**SRS_CBS_01_047: [** `cbs_close` when closed shall fail and return a non-zero value. **]**
**SRS_CBS_01_048: [** `cbs_close` when not opened shall fail and return a non-zero value. **]**
**SRS_CBS_01_099: [** All pending operations shall be freed. **]**

### cbs_put_token_async

```c
MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, cbs_put_token_async, CBS_HANDLE, cbs, const char*, type, const char*, audience, const char*, token, ON_CBS_OPERATION_COMPLETE, on_cbs_put_token_complete, void*, on_cbs_put_token_complete_context);
```

**SRS_CBS_01_049: [** `cbs_put_token_async` shall construct a request message for the `put-token` operation. **]**
**SRS_CBS_01_081: [** On success `cbs_put_token_async` shall return an `ASYNC_OPERATION_HANDLE`. **]**
**SRS_CBS_09_001: [** The `ASYNC_OPERATION_HANDLE` cancel function shall cancel the underlying amqp management operation, remove this operation from the pending list, destroy this async operation. **]**
**SRS_CBS_01_050: [** If any of the arguments `cbs`, `type`, `audience`, `token` or `on_cbs_put_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_083: [** `on_cbs_put_token_complete_context` shall be allowed to be NULL. **]**
**SRS_CBS_01_051: [** `cbs_put_token_async` shall start the AMQP management operation by calling `amqp_management_execute_operation_async`, while passing to it: **]**
**SRS_CBS_01_052: [** The `amqp_management` argument shall be the one for the AMQP management instance created in `cbs_create`. **]**
**SRS_CBS_01_053: [** The `operation` argument shall be `put-token`. **]**
**SRS_CBS_01_054: [** The `type` argument shall be set to the `type` argument. **]**
**SRS_CBS_01_055: [** The `locales` argument shall be set to NULL. **]**
**SRS_CBS_01_056: [** The `message` argument shall be the message constructed earlier according to the CBS spec. **]**
**SRS_CBS_01_072: [** If constructing the message fails, `cbs_put_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_084: [** If `amqp_management_execute_operation_async` fails `cbs_put_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_057: [** The arguments `on_execute_operation_complete` and `context` shall be set to a callback that is to be called by the AMQP management module when the operation is complete. **]**
**SRS_CBS_01_058: [** If `cbs_put_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. **]**

### cbs_delete_token_async

```c
MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, cbs_delete_token_async, CBS_HANDLE, cbs, const char*, type, const char*, audience, ON_CBS_OPERATION_COMPLETE, on_cbs_delete_token_complete, void*, on_cbs_delete_token_complete_context);
```

**SRS_CBS_01_059: [** `cbs_delete_token_async` shall construct a request message for the `delete-token` operation. **]**
**SRS_CBS_01_082: [** On success `cbs_delete_token_async` shall return an `ASYNC_OPERATION_HANDLE`. **]**
**SRS_CBS_09_001: [** The `ASYNC_OPERATION_HANDLE` cancel function shall cancel the underlying amqp management operation, remove this operation from the pending list, destroy this async operation. **]**
**SRS_CBS_01_060: [** If any of the arguments `cbs`, `type`, `audience` or `on_cbs_delete_token_complete` is NULL `cbs_put_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_086: [** `on_cbs_delete_token_complete_context` shall be allowed to be NULL. **]**
**SRS_CBS_01_061: [** `cbs_delete_token_async` shall start the AMQP management operation by calling `amqp_management_execute_operation_async`, while passing to it: **]**
**SRS_CBS_01_085: [** The `amqp_management` argument shall be the one for the AMQP management instance created in `cbs_create`. **]**
**SRS_CBS_01_062: [** The `operation` argument shall be `delete-token`. **]**
**SRS_CBS_01_063: [** The `type` argument shall be set to the `type` argument. **]**
**SRS_CBS_01_064: [** The `locales` argument shall be set to NULL. **]**
**SRS_CBS_01_065: [** The `message` argument shall be the message constructed earlier according to the CBS spec. **]**
**SRS_CBS_01_071: [** If constructing the message fails, `cbs_delete_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_087: [** If `amqp_management_execute_operation_async` fails `cbs_put_token_async` shall fail and return a non-zero value. **]**
**SRS_CBS_01_066: [** The arguments `on_execute_operation_complete` and `context` shall be set to a callback that is to be called by the AMQP management module when the operation is complete. **]**
**SRS_CBS_01_067: [** If `cbs_delete_token_async` is called when the CBS instance is not yet open or in error, it shall fail and return `NULL`. **]**

### cbs_set_trace

```c
MOCKABLE_FUNCTION(, int, cbs_set_trace, CBS_HANDLE, cbs, bool, trace_on);
```

**SRS_CBS_01_088: [** `cbs_set_trace` shall enable or disable tracing by calling `amqp_management_set_trace` to pass down the `trace_on` value. **]**
**SRS_CBS_01_089: [** On success, `cbs_set_trace` shall return 0. **]**
**SRS_CBS_01_090: [** If the argument `cbs` is NULL, `cbs_set_trace` shall fail and return a non-zero value. **]**

### on_amqp_management_open_complete

```c
void on_amqp_management_open_complete(void* context, AMQP_MANAGEMENT_OPEN_RESULT open_result);
```

**SRS_CBS_01_105: [** When `on_amqp_management_open_complete` is called with NULL `context`, it shall do nothing. **]**
**SRS_CBS_01_106: [** If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_OK` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_OK` and the `on_cbs_open_complete_context` shall be passed as argument. **]**
**SRS_CBS_01_107: [** If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_ERROR` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_ERROR` and the `on_cbs_open_complete_context` shall be passed as argument. **]**
**SRS_CBS_01_108: [** If CBS is OPENING and `open_result` is `AMQP_MANAGEMENT_OPEN_CANCELLED` the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_CANCELLED` and the `on_cbs_open_complete_context` shall be passed as argument. **]**
**SRS_CBS_01_109: [** When `on_amqp_management_open_complete` is called when the CBS is OPEN, the callback `on_cbs_error` shall be called and the `on_cbs_error_context` shall be passed as argument. **]**
**SRS_CBS_01_113: [** When `on_amqp_management_open_complete` reports a failure, the underlying AMQP management shall be closed by calling `amqp_management_close`. **]**

```c
void on_amqp_management_error(void* context);
```

**SRS_CBS_01_110: [** When `on_amqp_management_error` is called with NULL `context`, it shall do nothing. **]**
**SRS_CBS_01_111: [** If CBS is OPENING the callback `on_cbs_open_complete` shall be called with `CBS_OPEN_ERROR` and the `on_cbs_open_complete_context` shall be passed as argument. **]**
**SRS_CBS_01_114: [** Additionally the underlying AMQP management shall be closed by calling `amqp_management_close`. **]**
**SRS_CBS_01_112: [** If CBS is OPEN the callback `on_cbs_error` shall be called and the `on_cbs_error_context` shall be passed as argument. **]**

### on_amqp_management_execute_operation_complete

```c
void on_amqp_management_execute_operation_complete(void* context, OPERATION_RESULT operation_result, unsigned int status_code, const char* status_description);
```

**SRS_CBS_01_091: [** When `on_amqp_management_execute_operation_complete` is called with a NULL context it shall do nothing. **]**
**SRS_CBS_01_103: [** The `context` shall be used to obtain the pending operation information stored in the pending operations linked list by calling `singlylinkedlist_item_get_value`. **]**
**SRS_CBS_01_104: [** If `singlylinkedlist_item_get_value` returns NULL, `on_amqp_management_execute_operation_complete` shall do nothing. **]**
**SRS_CBS_01_092: [** When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_OK`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OK` and passing the `on_cbs_put_token_complete_context` as the context argument. **]**
**SRS_CBS_01_093: [** When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_CBS_ERROR` and passing the `on_cbs_put_token_complete_context` as the context argument. **]**
**SRS_CBS_01_094: [** When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_OPERATION_FAILED` and passing the `on_cbs_put_token_complete_context` as the context argument. **]**
**SRS_CBS_01_115: [** When `on_amqp_management_execute_operation_complete` is called with `AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED`, the associated cbs operation complete callback shall be called with `CBS_OPERATION_RESULT_INSTANCE_CLOSED` and passing the `on_cbs_put_token_complete_context` as the context argument. **]**
**SRS_CBS_01_095: [** `status_code` and `status_description` shall be passed as they are to the cbs operation complete callback. **]**
**SRS_CBS_01_102: [** The pending operation shall be removed from the pending operations list by calling `singlylinkedlist_remove`. **]**
**SRS_CBS_01_096: [** The `context` for the operation shall also be freed. **]**

### Relevant parts from the CBS spec:

**SRS_CBS_01_002: [** Tokens are communicated between AMQP peers by sending specially-formatted AMQP messages to the Claims-based Security Node. **]**
**SRS_CBS_01_003: [** The mechanism follows the scheme defined in the AMQP Management specification [AMQPMAN]. **]**

4.1 Putting a Token

**SRS_CBS_01_004: [** A token is sent to the CBS Node by transferring a "put-token" message. **]**

4.1.1 Request Message

The request message has the following application-properties:

Key	Optional	Value Type	Value Contents
**SRS_CBS_01_005: [** operation	No	string	"put-token" **]**
**SRS_CBS_01_006: [** Type	No	string	The type of the token being put, e.g., "amqp:jwt". **]**
**SRS_CBS_01_007: [** name	No	string	The "audience" to which the token applies. **]**
**SRS_CBS_01_008: [** expiration	Yes	timestamp	The expiry time of the token. **]**

**SRS_CBS_01_009: [** The body of the message MUST contain the token. **]**
**SRS_CBS_01_010: [** The type of the body is dependent on the type of token being put. **]**

The table below lists the body types for common token types:

Token Type	Token Description	Body Type
amqp:jwt	JSON Web Token (JWT)	AMQP Value (string)
amqp:swt	Simple Web Token (SWT)	AMQP Value (string)

4.1.2 Response Message

**SRS_CBS_01_014: [** The response message has the following application-properties: **]**

Key	Optional	Value Type	Value Contents
**SRS_CBS_01_013: [** status-code	No	int	HTTP response code [RFC2616]. **]**
**SRS_CBS_01_015: [** status-description	Yes	string	Description of the status. **]**

**SRS_CBS_01_016: [** The body of the message MUST be empty. **]**

If the request was successful then the status-code MUST contain 200.
If the request was unsuccessful due to a processing error then the status-code SHOULD contain 500 and further information MAY be provided in the status-description.
For error conditions related to the content of the request, e.g., unsupported token type, malformed request etc., the status-code SHOULD contain 400 and a detailed description SHOULD NOT be provided in the status-description, in line with general best practice for security-related protocols.

4.2 Deleting a Token

**SRS_CBS_01_020: [** To instruct a peer to delete a token associated with a specific audience, a "delete-token" message can be sent to the CBS Node **]**

4.2.1 Request Message

**SRS_CBS_01_021: [** The request message has the following application-properties: **]**

Key	Mandatory	Value Type	Value Contents
**SRS_CBS_01_022: [** operation	Yes	string	"delete-token" **]**
**SRS_CBS_01_023: [** Type	Yes	string	The type of the token being deleted, e.g., "amqp:jwt". **]**
**SRS_CBS_01_024: [** name	Yes	string	The "audience" of the token being deleted. **]**

**SRS_CBS_01_025: [** The body of the message MUST be empty. **]**

4.2.2 Response Message

**SRS_CBS_01_026: [** The response message has the following application-properties: **]**

Key	Mandatory	Value Type	Value Contents
**SRS_CBS_01_027: [** status-code	Yes	int	HTTP response code [RFC2616]. **]**
**SRS_CBS_01_028: [** status-description	No	string	Description of the status. **]**

**SRS_CBS_01_029: [** The body of the message MUST be empty. **]**

If the request was successful then the status-code MUST contain 200.
If the request was unsuccessful due to a processing error then the status-code SHOULD contain 500 and further information MAY be provided in the status-description.
For error conditions related to the content of the request, the status-code SHOULD contain 400 and a detailed description SHOULD NOT be provided in the status-description, in line with general best practice for security-related protocols.

Note that a condition in which the token was not found should be treated as success.
