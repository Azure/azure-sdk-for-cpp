// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define SECURITY_WIN32
#ifdef WINCE
#define UNICODE // Only Unicode version of secur32.lib functions supported on Windows CE
#define SCH_USE_STRONG_CRYPTO  0x00400000 // not defined in header file
#define SEC_TCHAR   SEC_WCHAR
#else
#define SEC_TCHAR   SEC_CHAR
#endif

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "tls_server_io.h"
#include "azure_c_shared_utility/socketio.h"
#include "windows.h"
#include "sspi.h"
#include "schannel.h"
#include "azure_macro_utils/macro_utils.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/x509_schannel.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

#define TLS_SERVER_IO_STATE_VALUES                        \
    TLS_SERVER_IO_STATE_NOT_OPEN,                         \
    TLS_SERVER_IO_STATE_OPENING_UNDERLYING_IO,            \
    TLS_SERVER_IO_STATE_WAITING_FOR_CLIENT_HELLO,         \
    TLS_SERVER_IO_STATE_IN_HANDSHAKE,                     \
    TLS_SERVER_IO_STATE_OPEN,                             \
    TLS_SERVER_IO_STATE_CLOSING,                          \
    TLS_SERVER_IO_STATE_ERROR

MU_DEFINE_ENUM(TLS_SERVER_IO_STATE, TLS_SERVER_IO_STATE_VALUES);
MU_DEFINE_ENUM_STRINGS(TLS_SERVER_IO_STATE, TLS_SERVER_IO_STATE_VALUES);

typedef struct PENDING_SEND_TAG
{
    unsigned char* bytes;
    size_t length;
    ON_SEND_COMPLETE on_send_complete;
    void* on_send_complete_context;
} PENDING_SEND;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE socket_io;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_bytes_received_context;
    void* on_io_error_context;
    CtxtHandle security_context;
    TLS_SERVER_IO_STATE tlsio_state;
    SEC_TCHAR* host_name;
    CredHandle credential_handle;
    bool credential_handle_allocated;
    unsigned char* received_bytes;
    size_t received_byte_count;
    size_t buffer_size;
    size_t needed_bytes;
    char* x509certificate;
    char* x509privatekey;
    X509_SCHANNEL_HANDLE x509_schannel_handle;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
    PCCERT_CONTEXT cert_context;
} TLS_IO_INSTANCE;

static int tls_server_io_schannel_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value);

/*this function will clone an option given by name and value*/
static void* tls_server_io_schannel_CloneOption(const char* name, const void* value)
{
    void* result;
    if (
        (name == NULL) || (value == NULL)
        )
    {
        LogError("invalid parameter detected: const char* name = %p, const void* value = %p", name, value);
        result = NULL;
    }
    else
    {
        if (strcmp(name, "x509certificate") == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, (const char *) value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509certificate value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else if (strcmp(name, "x509privatekey") == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, (const char *) value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509privatekey value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            LogError("not handled option : %s", name);
            result = NULL;
        }
    }
    return result;
}

/*this function destroys an option previously created*/
static void tls_server_io_schannel_DestroyOption(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/
    if (
        (name == NULL) || (value == NULL)
        )
    {
        LogError("invalid parameter detected: const char* name = %p, const void* value = %p", name, value);
    }
    else
    {
        if (
            (strcmp(name, "x509certificate") == 0) ||
            (strcmp(name, "x509privatekey") == 0)
            )
        {
            free((void*)value);
        }
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}

static OPTIONHANDLER_HANDLE tls_server_io_schannel_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle = %p", handle);
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(tls_server_io_schannel_CloneOption, tls_server_io_schannel_DestroyOption, tls_server_io_schannel_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            /*this layer cares about the certificates and the x509 credentials*/
            TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)handle;
            if (
                (tls_io_instance->x509certificate != NULL) &&
                (OptionHandler_AddOption(result, "x509certificate", tls_io_instance->x509certificate) != 0)
                )
            {
                LogError("unable to save x509certificate option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (
                (tls_io_instance->x509privatekey != NULL) &&
                (OptionHandler_AddOption(result, "x509privatekey", tls_io_instance->x509privatekey) != 0)
                )
            {
                LogError("unable to save x509privatekey option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
    return result;
}

static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->on_io_error != NULL)
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

static int resize_receive_buffer(TLS_IO_INSTANCE* tls_io_instance, size_t needed_buffer_size)
{
    int result;

    if (needed_buffer_size > tls_io_instance->buffer_size)
    {
        unsigned char* new_buffer = (unsigned char*) realloc(tls_io_instance->received_bytes, needed_buffer_size);
        if (new_buffer == NULL)
        {
            LogError("realloc failed");
            result = MU_FAILURE;
        }
        else
        {
            tls_io_instance->received_bytes = new_buffer;
            tls_io_instance->buffer_size = needed_buffer_size;
            result = 0;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}

static void on_underlying_io_close_complete(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    if (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_CLOSING)
    {
        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_NOT_OPEN;
        if (tls_io_instance->on_io_close_complete != NULL)
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }

        /* Free security context resources corresponding to creation with open */
        DeleteSecurityContext(&tls_io_instance->security_context);

        if (tls_io_instance->credential_handle_allocated)
        {
            (void)FreeCredentialHandle(&tls_io_instance->credential_handle);
            tls_io_instance->credential_handle_allocated = false;
        }
    }
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT io_open_result)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state != TLS_SERVER_IO_STATE_OPENING_UNDERLYING_IO)
    {
        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
        indicate_error(tls_io_instance);
    }
    else
    {
        if (io_open_result != IO_OPEN_OK)
        {
            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_NOT_OPEN;
            if (tls_io_instance->on_io_open_complete != NULL)
            {
                tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
            }
        }
        else
        {
            SECURITY_STATUS status;
            SCHANNEL_CRED auth_data;
            PCCERT_CONTEXT certContext;
            auth_data.dwVersion = SCHANNEL_CRED_VERSION;
            if (tls_io_instance->x509_schannel_handle != NULL)
            {
                certContext = x509_schannel_get_certificate_context(tls_io_instance->x509_schannel_handle);
                auth_data.cCreds = 1;
                auth_data.paCred = &certContext;
            }
            else
            {

                certContext = tls_io_instance->cert_context;
                auth_data.cCreds = 1;
                auth_data.paCred = &certContext;

            }

            auth_data.hRootStore = NULL;
            auth_data.cSupportedAlgs = 0;
            auth_data.palgSupportedAlgs = NULL;
#if defined(SP_PROT_TLS1_2_SERVER)
            auth_data.grbitEnabledProtocols = SP_PROT_TLS1_2_SERVER;
#else
            auth_data.grbitEnabledProtocols = SP_PROT_TLS1_SERVER;
#endif
            auth_data.dwMinimumCipherStrength = 0;
            auth_data.dwMaximumCipherStrength = 0;
            auth_data.dwSessionLifespan = 0;
            auth_data.dwFlags = SCH_CRED_NO_SYSTEM_MAPPER;
            auth_data.dwCredFormat = 0;

            status = AcquireCredentialsHandle(NULL, UNISP_NAME, SECPKG_CRED_INBOUND, NULL,
                &auth_data, NULL, NULL, &tls_io_instance->credential_handle, NULL);
            if (status != SEC_E_OK)
            {
                tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                indicate_error(tls_io_instance);
            }
            else
            {
                tls_io_instance->credential_handle_allocated = true;
                tls_io_instance->needed_bytes = 1;
                tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_WAITING_FOR_CLIENT_HELLO;
            }
        }
    }
}

static int set_receive_buffer(TLS_IO_INSTANCE* tls_io_instance, size_t buffer_size)
{
    int result;

    unsigned char* new_buffer = (unsigned char*) realloc(tls_io_instance->received_bytes, buffer_size);
    if (new_buffer == NULL)
    {
        LogError("realloc failed");
        result = MU_FAILURE;
    }
    else
    {
        tls_io_instance->received_bytes = new_buffer;
        tls_io_instance->buffer_size = buffer_size;
        result = 0;
    }

    return result;
}

static int send_chunk(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((tls_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        LogError("invalid argument detected: CONCRETE_IO_HANDLE tls_io = %p, const void* buffer = %p, size_t size = %lu", tls_io, buffer, (unsigned long)size);
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        if (tls_io_instance->tlsio_state != TLS_SERVER_IO_STATE_OPEN)
        {
            LogError("invalid tls_io_instance->tlsio_state: %" PRI_MU_ENUM "", MU_ENUM_VALUE(TLS_SERVER_IO_STATE, tls_io_instance->tlsio_state));
            result = MU_FAILURE;
        }
        else
        {
            SecPkgContext_StreamSizes  sizes;
            SECURITY_STATUS status = QueryContextAttributes(&tls_io_instance->security_context, SECPKG_ATTR_STREAM_SIZES, &sizes);
            if (status != SEC_E_OK)
            {
                LogError("QueryContextAttributes failed: %x", status);
                result = MU_FAILURE;
            }
            else
            {
                SecBuffer security_buffers[4];
                SecBufferDesc security_buffers_desc;
                size_t needed_buffer = sizes.cbHeader + size + sizes.cbTrailer;
                unsigned char* out_buffer = (unsigned char*)malloc(needed_buffer);
                if (out_buffer == NULL)
                {
                    LogError("malloc failed");
                    result = MU_FAILURE;
                }
                else
                {
                    (void)memcpy(out_buffer + sizes.cbHeader, buffer, size);

                    security_buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
                    security_buffers[0].cbBuffer = sizes.cbHeader;
                    security_buffers[0].pvBuffer = out_buffer;
                    security_buffers[1].BufferType = SECBUFFER_DATA;
                    security_buffers[1].cbBuffer = (unsigned long)size;
                    security_buffers[1].pvBuffer = out_buffer + sizes.cbHeader;
                    security_buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
                    security_buffers[2].cbBuffer = sizes.cbTrailer;
                    security_buffers[2].pvBuffer = out_buffer + sizes.cbHeader + size;
                    security_buffers[3].cbBuffer = 0;
                    security_buffers[3].BufferType = SECBUFFER_EMPTY;
                    security_buffers[3].pvBuffer = 0;

                    security_buffers_desc.cBuffers = sizeof(security_buffers) / sizeof(security_buffers[0]);
                    security_buffers_desc.pBuffers = security_buffers;
                    security_buffers_desc.ulVersion = SECBUFFER_VERSION;

                    status = EncryptMessage(&tls_io_instance->security_context, 0, &security_buffers_desc, 0);
                    if (FAILED(status))
                    {
                        LogError("EncryptMessage failed: %x", status);
                        result = MU_FAILURE;
                    }
                    else
                    {
                        if (xio_send(tls_io_instance->socket_io, out_buffer, security_buffers[0].cbBuffer + security_buffers[1].cbBuffer + security_buffers[2].cbBuffer, on_send_complete, callback_context) != 0)
                        {
                            LogError("xio_send failed");
                            result = MU_FAILURE;
                        }
                        else
                        {
                            result = 0;
                        }
                    }

                    free(out_buffer);
                }
            }
        }
    }

    return result;
}

static int internal_send(TLS_IO_INSTANCE* tls_io_instance, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    while (size > 0)
    {
        size_t to_send = 16 * 1024;
        if (to_send > size)
        {
            to_send = size;
        }

        if (send_chunk(tls_io_instance, buffer, to_send, (to_send == size) ? on_send_complete : NULL, callback_context) != 0)
        {
            break;
        }

        size -= to_send;
        buffer = ((const unsigned char*)buffer) + to_send;
    }

    if (size > 0)
    {
        LogError("send_chunk failed");
        result = MU_FAILURE;
    }
    else
    {
        result = 0;
    }

    return result;
}

// This callback usage needs to be either verified and commented or integrated into
// the state machine.
static void unchecked_on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    size_t consumed_bytes;

    if (resize_receive_buffer(tls_io_instance, tls_io_instance->received_byte_count + size) == 0)
    {
        (void)memcpy(tls_io_instance->received_bytes + tls_io_instance->received_byte_count, buffer, size);
        tls_io_instance->received_byte_count += size;

        if (size > tls_io_instance->needed_bytes)
        {
            tls_io_instance->needed_bytes = 0;
        }
        else
        {
            tls_io_instance->needed_bytes -= size;
        }

        /* Drain what we received */
        while (tls_io_instance->needed_bytes == 0)
        {
            if ((tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_IN_HANDSHAKE) ||
                (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_WAITING_FOR_CLIENT_HELLO))
            {
                SecBuffer input_buffers[2];
                SecBuffer output_buffers[2];
                ULONG context_attributes;
                SECURITY_STATUS status;
                SecBufferDesc input_buffers_desc;
                SecBufferDesc output_buffers_desc;

                input_buffers[0].cbBuffer = (unsigned long)tls_io_instance->received_byte_count;
                input_buffers[0].BufferType = SECBUFFER_TOKEN;
                input_buffers[0].pvBuffer = (void*)tls_io_instance->received_bytes;
                input_buffers[1].cbBuffer = 0;
                input_buffers[1].BufferType = SECBUFFER_EMPTY;
                input_buffers[1].pvBuffer = 0;

                input_buffers_desc.cBuffers = 2;
                input_buffers_desc.pBuffers = input_buffers;
                input_buffers_desc.ulVersion = SECBUFFER_VERSION;

                output_buffers[0].cbBuffer = 0;
                output_buffers[0].BufferType = SECBUFFER_TOKEN;
                output_buffers[0].pvBuffer = NULL;
                output_buffers[1].cbBuffer = 0;
                output_buffers[1].BufferType = SECBUFFER_EMPTY;
                output_buffers[1].pvBuffer = 0;

                output_buffers_desc.cBuffers = 2;
                output_buffers_desc.pBuffers = output_buffers;
                output_buffers_desc.ulVersion = SECBUFFER_VERSION;

                status = AcceptSecurityContext(&tls_io_instance->credential_handle, (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_WAITING_FOR_CLIENT_HELLO) ? NULL : &tls_io_instance->security_context, &input_buffers_desc, ASC_REQ_ALLOCATE_MEMORY, SECURITY_NETWORK_DREP, (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_WAITING_FOR_CLIENT_HELLO) ? &tls_io_instance->security_context : NULL,
                    &output_buffers_desc, &context_attributes, NULL);

                switch (status)
                {
                case SEC_E_INCOMPLETE_MESSAGE:
                    if (input_buffers[1].BufferType != SECBUFFER_MISSING)
                    {
                        //If SECBUFFER_MISSING not sent, try to read byte by byte.
                        tls_io_instance->needed_bytes = 1;
                    }
                    else
                    {
                        tls_io_instance->needed_bytes = input_buffers[1].cbBuffer;
                    }

                    if (resize_receive_buffer(tls_io_instance, tls_io_instance->received_byte_count + tls_io_instance->needed_bytes) != 0)
                    {
                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        if (tls_io_instance->on_io_open_complete != NULL)
                        {
                            tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                        }
                    }

                    break;
                case SEC_E_OK:
                    if ((output_buffers[0].cbBuffer > 0) && xio_send(tls_io_instance->socket_io, output_buffers[0].pvBuffer, output_buffers[0].cbBuffer, unchecked_on_send_complete, NULL) != 0)
                    {
                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        if (tls_io_instance->on_io_open_complete != NULL)
                        {
                            tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                        }
                    }
                    else
                    {
                        consumed_bytes = tls_io_instance->received_byte_count;
                        /* Any extra bytes left over or did we fully consume the receive buffer? */
                        if (input_buffers[1].BufferType == SECBUFFER_EXTRA)
                        {
                            consumed_bytes -= input_buffers[1].cbBuffer;
                            (void)memmove(tls_io_instance->received_bytes, tls_io_instance->received_bytes + consumed_bytes, tls_io_instance->received_byte_count - consumed_bytes);
                        }
                        tls_io_instance->received_byte_count -= consumed_bytes;

                        /* if nothing more to consume, set the needed bytes to 1, to get on the next byte how many we actually need */
                        tls_io_instance->needed_bytes = tls_io_instance->received_byte_count == 0 ? 1 : 0;

                        if (set_receive_buffer(tls_io_instance, tls_io_instance->needed_bytes + tls_io_instance->received_byte_count) != 0)
                        {
                            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                            if (tls_io_instance->on_io_open_complete != NULL)
                            {
                                tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                            }
                        }
                        else
                        {
                            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_OPEN;

                            if (tls_io_instance->on_io_open_complete != NULL)
                            {
                                tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_OK);
                            }
                        }
                    }
                    break;
                case SEC_I_COMPLETE_NEEDED:
                case SEC_I_CONTINUE_NEEDED:
                case SEC_I_COMPLETE_AND_CONTINUE:
                    if ((output_buffers[0].cbBuffer > 0) && xio_send(tls_io_instance->socket_io, output_buffers[0].pvBuffer, output_buffers[0].cbBuffer, unchecked_on_send_complete, NULL) != 0)
                    {
                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        if (tls_io_instance->on_io_open_complete != NULL)
                        {
                            tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                        }
                    }
                    else
                    {
                        consumed_bytes = tls_io_instance->received_byte_count;
                        /* Any extra bytes left over or did we fully consume the receive buffer? */
                        if (input_buffers[1].BufferType == SECBUFFER_EXTRA)
                        {
                            consumed_bytes -= input_buffers[1].cbBuffer;
                            (void)memmove(tls_io_instance->received_bytes, tls_io_instance->received_bytes + consumed_bytes, tls_io_instance->received_byte_count - consumed_bytes);
                        }
                        tls_io_instance->received_byte_count -= consumed_bytes;

                        /* if nothing more to consume, set the needed bytes to 1, to get on the next byte how many we actually need */
                        tls_io_instance->needed_bytes = tls_io_instance->received_byte_count == 0 ? 1 : 0;

                        if (set_receive_buffer(tls_io_instance, tls_io_instance->needed_bytes + tls_io_instance->received_byte_count) != 0)
                        {
                            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                            if (tls_io_instance->on_io_open_complete != NULL)
                            {
                                tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                            }
                        }
                        else
                        {
                            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_IN_HANDSHAKE;
                        }
                    }
                    break;
                default:
                {
                    LPVOID srcText = NULL;
                    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                        status, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)srcText, 0, NULL) > 0)
                    {
                        LogError("[%#x] %s", status, (LPTSTR)srcText);
                        LocalFree(srcText);
                    }
                    else
                    {
                        LogError("[%#x]", status);
                    }

                    tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                    if (tls_io_instance->on_io_open_complete != NULL)
                    {
                        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
                    }
                }
                break;
                }
            }
            else if (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_OPEN)
            {
                SecBuffer security_buffers[4];
                SecBufferDesc security_buffers_desc;
                SECURITY_STATUS status;

                security_buffers[0].BufferType = SECBUFFER_DATA;
                security_buffers[0].pvBuffer = tls_io_instance->received_bytes;
                security_buffers[0].cbBuffer = (unsigned long)tls_io_instance->received_byte_count;
                security_buffers[1].BufferType = SECBUFFER_EMPTY;
                security_buffers[2].BufferType = SECBUFFER_EMPTY;
                security_buffers[3].BufferType = SECBUFFER_EMPTY;

                security_buffers_desc.cBuffers = sizeof(security_buffers) / sizeof(security_buffers[0]);
                security_buffers_desc.pBuffers = security_buffers;
                security_buffers_desc.ulVersion = SECBUFFER_VERSION;

                status = DecryptMessage(&tls_io_instance->security_context, &security_buffers_desc, 0, NULL);
                switch (status)
                {
                case SEC_E_INCOMPLETE_MESSAGE:
                    if (security_buffers[1].BufferType != SECBUFFER_MISSING)
                    {
                        //If SECBUFFER_MISSING not sent, try to read byte by byte.
                        tls_io_instance->needed_bytes = 1;
                    }
                    else
                    {
                        tls_io_instance->needed_bytes = security_buffers[1].cbBuffer;
                    }

                    if (resize_receive_buffer(tls_io_instance, tls_io_instance->received_byte_count + tls_io_instance->needed_bytes) != 0)
                    {
                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        indicate_error(tls_io_instance);
                    }
                    break;
                case SEC_E_OK:
                    if (security_buffers[1].BufferType != SECBUFFER_DATA)
                    {
                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        indicate_error(tls_io_instance);
                    }
                    else
                    {
                        size_t i;

                        /* notify of the received data */
                        if (tls_io_instance->on_bytes_received != NULL)
                        {
                            tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, (const unsigned char *) security_buffers[1].pvBuffer, security_buffers[1].cbBuffer);
                        }

                        consumed_bytes = tls_io_instance->received_byte_count;

                        for (i = 0; i < sizeof(security_buffers) / sizeof(security_buffers[0]); i++)
                        {
                            /* Any extra bytes left over or did we fully consume the receive buffer? */
                            if (security_buffers[i].BufferType == SECBUFFER_EXTRA)
                            {
                                consumed_bytes -= security_buffers[i].cbBuffer;
                                (void)memmove(tls_io_instance->received_bytes, tls_io_instance->received_bytes + consumed_bytes, tls_io_instance->received_byte_count - consumed_bytes);
                                break;
                            }
                        }
                        tls_io_instance->received_byte_count -= consumed_bytes;

                        /* if nothing more to consume, set the needed bytes to 1, to get on the next byte how many we actually need */
                        tls_io_instance->needed_bytes = tls_io_instance->received_byte_count == 0 ? 1 : 0;

                        if (set_receive_buffer(tls_io_instance, tls_io_instance->needed_bytes + tls_io_instance->received_byte_count) != 0)
                        {
                            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                            indicate_error(tls_io_instance);
                        }
                    }
                    break;

                default:
                    {
                        LPVOID srcText = NULL;
                        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                            status, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)srcText, 0, NULL) > 0)
                        {
                            LogError("[%#x] %s", status, (LPTSTR)srcText);
                            LocalFree(srcText);
                        }
                        else
                        {
                            LogError("[%#x]", status);
                        }

                        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
                        indicate_error(tls_io_instance);
                    }
                    break;
                }
            }
            else
            {
                /* Received data in error or other state */
                break;
            }
        }

    }
}

static void on_underlying_io_error(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    switch (tls_io_instance->tlsio_state)
    {
    default:
    case TLS_SERVER_IO_STATE_NOT_OPEN:
    case TLS_SERVER_IO_STATE_ERROR:
        break;

    case TLS_SERVER_IO_STATE_OPENING_UNDERLYING_IO:
    case TLS_SERVER_IO_STATE_IN_HANDSHAKE:
        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
        if (tls_io_instance->on_io_open_complete != NULL)
        {
            tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
        }
        break;

    case TLS_SERVER_IO_STATE_CLOSING:
        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
        if (tls_io_instance->on_io_close_complete != NULL)
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }
        break;

    case TLS_SERVER_IO_STATE_OPEN:
        tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_ERROR;
        indicate_error(tls_io_instance);
        break;
    }
}

CONCRETE_IO_HANDLE tls_server_io_schannel_create(void* io_create_parameters)
{
    TLS_SERVER_IO_CONFIG* tls_server_io_config = (TLS_SERVER_IO_CONFIG*)io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_server_io_config == NULL)
    {
        LogError("invalid argument detected: void* io_create_parameters = %p", tls_server_io_config);
        result = NULL;
    }
    else
    {
        result = (TLS_IO_INSTANCE*)malloc(sizeof(TLS_IO_INSTANCE));
        if (result == NULL)
        {
            LogError("malloc failed");
        }
        else
        {
            HCERTSTORE cert_store_handle;
            const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
            void* io_interface_parameters;

            result->on_bytes_received = NULL;
            result->on_io_open_complete = NULL;
            result->on_io_close_complete = NULL;
            result->on_io_error = NULL;
            result->on_io_open_complete_context = NULL;
            result->on_io_close_complete_context = NULL;
            result->on_bytes_received_context = NULL;
            result->on_io_error_context = NULL;
            result->credential_handle_allocated = false;
            result->x509_schannel_handle = NULL;
            result->needed_bytes = 0;

            /* A cert store called "My" has to be available for use ...
            This is because this is only used in a test now ... */
            cert_store_handle = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING,
                0,
                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                L"MY");

            if (cert_store_handle == NULL)
            {
                result->cert_context = NULL;
                LogError("Error opening store for server.");
            }
            else
            {
                result->cert_context = CertFindCertificateInStore(cert_store_handle,
                    X509_ASN_ENCODING,
                    0,
                    CERT_FIND_SUBJECT_STR_A,
                    "localhost", // use appropriate subject name
                    NULL
                );

                if (!CertCloseStore(cert_store_handle, 0))
                {
                    LogError("Error closing store.");
                }
            }

            underlying_io_interface = tls_server_io_config->underlying_io_interface;
            io_interface_parameters = tls_server_io_config->underlying_io_parameters;

            if (underlying_io_interface == NULL)
            {
                LogError("socketio_get_interface_description failed");
                free(result->host_name);
                free(result);
                result = NULL;
            }
            else
            {
                result->socket_io = xio_create(underlying_io_interface, io_interface_parameters);
                if (result->socket_io == NULL)
                {
                    LogError("xio_create failed");
                    free(result->host_name);
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->pending_io_list = singlylinkedlist_create();
                    if (result->pending_io_list == NULL)
                    {
                        LogError("Failed creating pending IO list.");
                        xio_destroy(result->socket_io);
                        free(result->host_name);
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        result->received_bytes = NULL;
                        result->received_byte_count = 0;
                        result->buffer_size = 0;
                        result->tlsio_state = TLS_SERVER_IO_STATE_NOT_OPEN;
                        result->x509certificate = NULL;
                        result->x509privatekey = NULL;
                        result->x509_schannel_handle = NULL;
                    }
                }
            }
        }
    }

    return result;
}

void tls_server_io_schannel_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        LIST_ITEM_HANDLE first_pending_io;
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        if (tls_io_instance->credential_handle_allocated)
        {
            (void)FreeCredentialHandle(&tls_io_instance->credential_handle);
            tls_io_instance->credential_handle_allocated = false;
        }

        if (tls_io_instance->received_bytes != NULL)
        {
            free(tls_io_instance->received_bytes);
        }

        if (tls_io_instance->x509_schannel_handle != NULL)
        {
            x509_schannel_destroy(tls_io_instance->x509_schannel_handle);
        }

        if (tls_io_instance->x509certificate != NULL)
        {
            free(tls_io_instance->x509certificate);
        }

        if (tls_io_instance->x509privatekey != NULL)
        {
            free(tls_io_instance->x509privatekey);
        }

        xio_destroy(tls_io_instance->socket_io);
        free(tls_io_instance->host_name);

        first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_SEND* pending_send = (PENDING_SEND*)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_send == NULL)
            {
                LogError("Failure: retrieving socket from list");
                indicate_error(tls_io_instance);
                break;
            }
            else
            {
                if (pending_send->on_send_complete != NULL)
                {
                    pending_send->on_send_complete(pending_send->on_send_complete_context, IO_SEND_CANCELLED);
                }

                if (singlylinkedlist_remove(tls_io_instance->pending_io_list, first_pending_io) != 0)
                {
                    LogError("Failure: removing pending IO from list");
                }
            }
        }

        if (tls_io_instance->cert_context != NULL)
        {
            if (!CertFreeCertificateContext(tls_io_instance->cert_context))
            {
                LogError("Failure freeing certificate context");
            }
        }

        singlylinkedlist_destroy(tls_io_instance->pending_io_list);
        free(tls_io);
    }
}

int tls_server_io_schannel_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if (tls_io == NULL)
    {
        LogError("invalid argument detected: CONCRETE_IO_HANDLE tls_io = %p", tls_io);
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLS_SERVER_IO_STATE_NOT_OPEN)
        {
            LogError("invalid tls_io_instance->tlsio_state = %" PRI_MU_ENUM "", MU_ENUM_VALUE(TLS_SERVER_IO_STATE, tls_io_instance->tlsio_state));
            result = MU_FAILURE;
        }
        else
        {
            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_OPENING_UNDERLYING_IO;

            if (xio_open(tls_io_instance->socket_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
            {
                LogError("xio_open failed");
                tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_NOT_OPEN;
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tls_server_io_schannel_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("invalid argument detected: tls_io = %p", tls_io);
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLS_SERVER_IO_STATE_CLOSING))
        {
            LogError("invalid tls_io_instance->tlsio_state = %" PRI_MU_ENUM "", MU_ENUM_VALUE(TLS_SERVER_IO_STATE, tls_io_instance->tlsio_state));
            result = MU_FAILURE;
        }
        else
        {
            tls_io_instance->tlsio_state = TLS_SERVER_IO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;
            if (xio_close(tls_io_instance->socket_io, on_underlying_io_close_complete, tls_io_instance) != 0)
            {
                LogError("xio_close failed");
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tls_server_io_schannel_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    if (internal_send((TLS_IO_INSTANCE*)tls_io, buffer, size, on_send_complete, callback_context) != 0)
    {
        LogError("send failed");
        result = MU_FAILURE;
    }
    else
    {
        result = 0;
    }

    return result;
}

void tls_server_io_schannel_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        xio_dowork(tls_io_instance->socket_io);
    }
}

int tls_server_io_schannel_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

    if (tls_io == NULL || optionName == NULL)
    {
        LogError("invalid argument detected: CONCRETE_IO_HANDLE tls_io = %p, const char* optionName = %p", tls_io, optionName);
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        if (strcmp("x509certificate", optionName) == 0)
        {
            if (tls_io_instance->x509certificate != NULL)
            {
                LogError("x509certificate has already been specified");
                result = MU_FAILURE;
            }
            else
            {
                tls_io_instance->x509certificate = (char *)tls_server_io_schannel_CloneOption("x509certificate", value);
                if (tls_io_instance->x509certificate == NULL)
                {
                    LogError("tls_server_io_schannel_CloneOption failed");
                    result = MU_FAILURE;
                }
                else
                {
                    if (tls_io_instance->x509privatekey != NULL)
                    {
                        tls_io_instance->x509_schannel_handle = x509_schannel_create(tls_io_instance->x509certificate, tls_io_instance->x509privatekey);
                        if (tls_io_instance->x509_schannel_handle == NULL)
                        {
                            LogError("x509_schannel_create failed");
                            result = MU_FAILURE;
                        }
                        else
                        {
                            /*all is fine, the x509 shall be used later*/
                            result = 0;
                        }
                    }
                    else
                    {
                        result = 0; /*all is fine, maybe x509 privatekey will come and then x509 is set*/
                    }
                }
            }
        }
        else if (strcmp("x509privatekey", optionName) == 0)
        {
            if (tls_io_instance->x509privatekey != NULL)
            {
                LogError("x509privatekey has already been specified");
                result = MU_FAILURE;
            }
            else
            {
                tls_io_instance->x509privatekey = (char *)tls_server_io_schannel_CloneOption("x509privatekey", value);
                if (tls_io_instance->x509privatekey == NULL)
                {
                    LogError("tls_server_io_schannel_CloneOption failed");
                    result = MU_FAILURE;
                }
                else
                {
                    if (tls_io_instance->x509certificate != NULL)
                    {
                        tls_io_instance->x509_schannel_handle = x509_schannel_create(tls_io_instance->x509certificate, tls_io_instance->x509privatekey);
                        if (tls_io_instance->x509_schannel_handle == NULL)
                        {
                            LogError("x509_schannel_create failed");
                            result = MU_FAILURE;
                        }
                        else
                        {
                            /*all is fine, the x509 shall be used later*/
                            result = 0;
                        }
                    }
                    else
                    {
                        result = 0; /*all is fine, maybe x509 cert will come and then x509 is set*/
                    }
                }
            }
        }
        else if (tls_io_instance->socket_io == NULL)
        {
            LogError("tls_io_instance->socket_io is not set");
            result = MU_FAILURE;
        }
        else
        {
            result = xio_setoption(tls_io_instance->socket_io, optionName, value);
        }
    }

    return result;
}

static const IO_INTERFACE_DESCRIPTION tls_server_io_schannel_interface_description =
{
    tls_server_io_schannel_retrieveoptions,
    tls_server_io_schannel_create,
    tls_server_io_schannel_destroy,
    tls_server_io_schannel_open,
    tls_server_io_schannel_close,
    tls_server_io_schannel_send,
    tls_server_io_schannel_dowork,
    tls_server_io_schannel_setoption
};

const IO_INTERFACE_DESCRIPTION* tls_server_io_get_interface_description(void)
{
    return &tls_server_io_schannel_interface_description;
}
