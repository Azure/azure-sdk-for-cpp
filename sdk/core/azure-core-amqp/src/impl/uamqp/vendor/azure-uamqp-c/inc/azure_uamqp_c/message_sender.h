// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#include <stdbool.h>
#include "azure_uamqp_c/link.h"
#include "azure_uamqp_c/message.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_uamqp_c/async_operation.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "umock_c/umock_c_prod.h"
#include "azure_macro_utils/macro_utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MESSAGE_SEND_RESULT_VALUES \
    MESSAGE_SEND_OK, \
    MESSAGE_SEND_ERROR, \
    MESSAGE_SEND_TIMEOUT, \
    MESSAGE_SEND_CANCELLED

MU_DEFINE_ENUM(MESSAGE_SEND_RESULT, MESSAGE_SEND_RESULT_VALUES)

#define MESSAGE_SENDER_STATE_VALUES \
    MESSAGE_SENDER_STATE_IDLE, \
    MESSAGE_SENDER_STATE_OPENING, \
    MESSAGE_SENDER_STATE_OPEN, \
    MESSAGE_SENDER_STATE_CLOSING, \
    MESSAGE_SENDER_STATE_ERROR

MU_DEFINE_ENUM(MESSAGE_SENDER_STATE, MESSAGE_SENDER_STATE_VALUES)

    typedef struct MESSAGE_SENDER_INSTANCE_TAG* MESSAGE_SENDER_HANDLE;
    typedef void(*ON_MESSAGE_SEND_COMPLETE)(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state);
    typedef void(*ON_MESSAGE_SENDER_STATE_CHANGED)(void* context, MESSAGE_SENDER_STATE new_state, MESSAGE_SENDER_STATE previous_state);

    MOCKABLE_FUNCTION(, MESSAGE_SENDER_HANDLE, messagesender_create, LINK_HANDLE, link, ON_MESSAGE_SENDER_STATE_CHANGED, on_message_sender_state_changed, void*, context);
    MOCKABLE_FUNCTION(, void, messagesender_destroy, MESSAGE_SENDER_HANDLE, message_sender);
    MOCKABLE_FUNCTION(, int, messagesender_open, MESSAGE_SENDER_HANDLE, message_sender);
    MOCKABLE_FUNCTION(, int, messagesender_close, MESSAGE_SENDER_HANDLE, message_sender);
    /**
     * @brief   Sends a message.
     * @param   message_sender           The message sender.
     * @param   message                  The message to send. The caller keeps ownership.
     * @param   on_message_send_complete Called once when the send completes.
     * @param   callback_context         Context passed to @p on_message_send_complete.
     * @param   timeout                  Send timeout in milliseconds; 0 for none.
     * @return  A handle that can cancel the send, valid until @p on_message_send_complete is called;
     *          NULL on failure, in which case @p on_message_send_complete is not called.
     * @remarks @p on_message_send_complete can be called before this function returns (e.g. the send
     *          is settled or fails while being sent). The returned handle is then already destroyed
     *          and must not be used; callers must detect that case in the callback.
     */
    MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, messagesender_send_async, MESSAGE_SENDER_HANDLE, message_sender, MESSAGE_HANDLE, message, ON_MESSAGE_SEND_COMPLETE, on_message_send_complete, void*, callback_context, tickcounter_ms_t, timeout);
    MOCKABLE_FUNCTION(, void, messagesender_set_trace, MESSAGE_SENDER_HANDLE, message_sender, bool, traceOn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MESSAGE_SENDER_H */
