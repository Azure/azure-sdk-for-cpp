# session requirements
 
## Overview

session is module that implements the session layer in the AMQP ISO.

## Exposed API

```C
typedef struct SESSION_INSTANCE_TAG* SESSION_HANDLE;
typedef struct LINK_ENDPOINT_INSTANCE_TAG* LINK_ENDPOINT_HANDLE;

#define SESSION_STATE_VALUES \
    SESSION_STATE_UNMAPPED, \
    SESSION_STATE_BEGIN_SENT, \
    SESSION_STATE_BEGIN_RCVD, \
    SESSION_STATE_MAPPED, \
    SESSION_STATE_END_SENT, \
    SESSION_STATE_END_RCVD, \
    SESSION_STATE_DISCARDING, \
    SESSION_STATE_ERROR

DEFINE_ENUM(SESSION_STATE, SESSION_STATE_VALUES)

#define SESSION_SEND_TRANSFER_RESULT_VALUES \
    SESSION_SEND_TRANSFER_OK, \
    SESSION_SEND_TRANSFER_ERROR, \
    SESSION_SEND_TRANSFER_BUSY

DEFINE_ENUM(SESSION_SEND_TRANSFER_RESULT, SESSION_SEND_TRANSFER_RESULT_VALUES)

    typedef void(*LINK_ENDPOINT_FRAME_RECEIVED_CALLBACK)(void* context, AMQP_VALUE performative, uint32_t frame_payload_size, const unsigned char* payload_bytes);
    typedef void(*ON_SESSION_STATE_CHANGED)(void* context, SESSION_STATE new_session_state, SESSION_STATE previous_session_state);
    typedef void(*ON_SESSION_FLOW_ON)(void* context);
    typedef bool(*ON_LINK_ATTACHED)(void* context, LINK_ENDPOINT_HANDLE new_link_endpoint, const char* name, role role, AMQP_VALUE source, AMQP_VALUE target, fields properties);

    MOCKABLE_FUNCTION(, SESSION_HANDLE, session_create, CONNECTION_HANDLE, connection, ON_LINK_ATTACHED, on_link_attached, void*, callback_context);
    MOCKABLE_FUNCTION(, SESSION_HANDLE, session_create_from_endpoint, CONNECTION_HANDLE, connection, ENDPOINT_HANDLE, connection_endpoint, ON_LINK_ATTACHED, on_link_attached, void*, callback_context);
    MOCKABLE_FUNCTION(, int, session_set_incoming_window, SESSION_HANDLE, session, uint32_t, incoming_window);
    MOCKABLE_FUNCTION(, int, session_get_incoming_window, SESSION_HANDLE, session, uint32_t*, incoming_window);
    MOCKABLE_FUNCTION(, int, session_set_outgoing_window, SESSION_HANDLE, session, uint32_t, outgoing_window);
    MOCKABLE_FUNCTION(, int, session_get_outgoing_window, SESSION_HANDLE, session, uint32_t*, outgoing_window);
    MOCKABLE_FUNCTION(, int, session_set_handle_max, SESSION_HANDLE, session, handle, handle_max);
    MOCKABLE_FUNCTION(, int, session_get_handle_max, SESSION_HANDLE, session, handle*, handle_max);
    MOCKABLE_FUNCTION(, void, session_destroy, SESSION_HANDLE, session);
    MOCKABLE_FUNCTION(, int, session_begin, SESSION_HANDLE, session);
    MOCKABLE_FUNCTION(, int, session_end, SESSION_HANDLE, session, const char*, condition_value, const char*, description);
    MOCKABLE_FUNCTION(, LINK_ENDPOINT_HANDLE, session_create_link_endpoint, SESSION_HANDLE, session, const char*, name);
    MOCKABLE_FUNCTION(, void, session_destroy_link_endpoint, LINK_ENDPOINT_HANDLE, link_endpoint);
    MOCKABLE_FUNCTION(, int, session_start_link_endpoint, LINK_ENDPOINT_HANDLE, link_endpoint, ON_ENDPOINT_FRAME_RECEIVED, frame_received_callback, ON_SESSION_STATE_CHANGED, on_session_state_changed, ON_SESSION_FLOW_ON, on_session_flow_on, void*, context);
    MOCKABLE_FUNCTION(, int, session_send_flow, LINK_ENDPOINT_HANDLE, link_endpoint, FLOW_HANDLE, flow);
    MOCKABLE_FUNCTION(, int, session_send_attach, LINK_ENDPOINT_HANDLE, link_endpoint, ATTACH_HANDLE, attach);
    MOCKABLE_FUNCTION(, int, session_send_disposition, LINK_ENDPOINT_HANDLE, link_endpoint, DISPOSITION_HANDLE, disposition);
    MOCKABLE_FUNCTION(, int, session_send_detach, LINK_ENDPOINT_HANDLE, link_endpoint, DETACH_HANDLE, detach);
    MOCKABLE_FUNCTION(, SESSION_SEND_TRANSFER_RESULT, session_send_transfer, LINK_ENDPOINT_HANDLE, link_endpoint, TRANSFER_HANDLE, transfer, PAYLOAD*, payloads, size_t, payload_count, delivery_number*, delivery_id, ON_SEND_COMPLETE, on_send_complete, void*, callback_context);
```

###session_create

```C
extern SESSION_HANDLE session_create(CONNECTION_HANDLE connection);
```

**S_R_S_SESSION_01_030: [**session_create shall create a new session instance and return a non-NULL handle to it.**]** 
**S_R_S_SESSION_01_031: [**If connection is NULL, session_create shall fail and return NULL.**]** 
**S_R_S_SESSION_01_032: [**session_create shall create a new session endpoint by calling connection_create_endpoint.**]** 
**S_R_S_SESSION_01_033: [**If connection_create_endpoint fails, session_create shall fail and return NULL.**]**
**S_R_S_SESSION_01_042: [**If allocating memory for the session fails, session_create shall fail and return NULL.**]** 

###session_destroy

```C
extern void session_destroy(SESSION_HANDLE session);
```

**S_R_S_SESSION_01_034: [**session_destroy shall free all resources allocated by session_create.**]** 
**S_R_S_SESSION_01_035: [**The endpoint created in session_create shall be freed by calling connection_destroy_endpoint.**]** 
**S_R_S_SESSION_01_036: [**If session is NULL, session_destroy shall do nothing.**]** 

###session_create_link_endpoint

```C
extern LINK_ENDPOINT_HANDLE session_create_link_endpoint(SESSION_HANDLE session, const char* name, LINK_ENDPOINT_FRAME_RECEIVED_CALLBACK frame_received_callback, void* context);
```

**S_R_S_SESSION_01_043: [**session_create_link_endpoint shall create a link endpoint associated with a given session and return a non-NULL handle to it.**]** 
**S_R_S_SESSION_01_044: [**If session, name or frame_received_callback is NULL, session_create_link_endpoint shall fail and return NULL.**]** 
**S_R_S_SESSION_01_045: [**If allocating memory for the link endpoint fails, session_create_link_endpoint shall fail and return NULL.**]** 
**S_R_S_SESSION_01_046: [**An unused handle shall be assigned to the link endpoint.**]** 
**S_R_S_SESSION_01_047: [**The lowest available handle shall be used.**]** 
**S_R_S_SESSION_01_048: [**If no more handles are available, session_create_link_endpoint shall fail and return NULL.**]** 

###session_destroy_link_endpoint

```C
extern void session_destroy_link_endpoint(LINK_ENDPOINT_HANDLE link_endpoint);
```

**S_R_S_SESSION_01_049: [**session_destroy_link_endpoint shall detach the associated endpoint, but not free the resources of the endpoint.**]** 
**S_R_S_SESSION_01_050: [**If link_endpoint is NULL, session_destroy_link_endpoint shall do nothing.**]** 

###session_send_transfer

```C
extern int session_send_transfer(LINK_ENDPOINT_HANDLE link_endpoint, TRANSFER_HANDLE transfer, PAYLOAD* payloads, size_t payload_count, delivery_number* delivery_id);
```

**S_R_S_SESSION_01_051: [**session_send_transfer shall send a transfer frame with the performative indicated in the transfer argument.**]** 
**S_R_S_SESSION_01_053: [**On success, session_send_transfer shall return 0.**]** 
**S_R_S_SESSION_01_054: [**If link_endpoint or transfer is NULL, session_send_transfer shall fail and return a non-zero value.**]** 
**S_R_S_SESSION_01_055: [**The encoding of the frame shall be done by calling connection_encode_frame and passing as arguments: the connection handle associated with the session, the transfer performative and the payload chunks passed to session_send_transfer.**]** 
**S_R_S_SESSION_01_056: [**If connection_encode_frame fails then session_send_transfer shall fail and return a non-zero value.**]** 
**S_R_S_SESSION_01_057: [**The delivery ids shall be assigned starting at 0.**]** 
**S_R_S_SESSION_01_058: [**When any other error occurs, session_send_transfer shall fail and return a non-zero value.**]** 
**S_R_S_SESSION_01_059: [**When session_send_transfer is called while the session is not in the MAPPED state, session_send_transfer shall fail and return a non-zero value.**]** 

###connection_state_changed_callback

The following shall be done when the connection_state_changed_callback is triggered:

-	**S_R_S_SESSION_01_060: [**If the previous connection state is not OPENED and the new connection state is OPENED, the BEGIN frame shall be sent out and the state shall be switched to BEGIN_SENT.**]**
-	**S_R_S_SESSION_01_061: [**If the previous connection state is OPENED and the new connection state is not OPENED anymore, the state shall be switched to DISCARDING.**]** 
-	**S_R_S_SESSION_09_001: [**If the new connection state is ERROR, the state shall be switched to ERROR.**]** 

##ISO section

Sessions

A session is a bidirectional sequential conversation between two containers that provides a grouping for related links. Sessions serve as the context for link communication. Any number of links of any directionality can be attached to a given session. However, a link MUST NOT be attached to more than one session at a time.

...

Figure 2.25: Instance Diagram of Session/Link attachment

Messages transferred on a link are sequentially identified within the session. A session can be viewed as multiplexing link traffic, much like a connection multiplexes session traffic. However, unlike the sessions on a connection, links on a session are not entirely independent since they share a common delivery sequence scoped to the session.
This common sequence allows endpoints to efficiently refer to sets of deliveries regardless of the originating link. This is of particular benefit when a single application is receiving messages along a large number of different links. In this case the session provides aggregation of otherwise independent links into a single stream that can be efficiently acknowledged by the receiving application.

2.5.1 Establishing A Session

**S_R_S_SESSION_01_001: [**Sessions are established by creating a session endpoint, assigning it to an unused channel number, and sending a begin announcing the association of the session endpoint with the outgoing channel.**]** **S_R_S_SESSION_01_002: [**Upon receiving the begin the partner will check the remote-channel field and find it empty.**]** This indicates that the begin is referring to remotely initiated session. **S_R_S_SESSION_01_003: [**The partner will therefore allocate an unused outgoing channel for the remotely initiated session and indicate this by sending its own begin setting the remote-channel field to the incoming channel of the remotely initiated session.**]** 
**S_R_S_SESSION_01_004: [**To make it easier to monitor AMQP sessions, it is RECOMMENDED that implementations always assign the lowest available unused channel number.**]** 
**S_R_S_SESSION_01_005: [**The remote-channel field of a begin frame MUST be empty for a locally initiated session**]**,**S_R_S_SESSION_01_006: [**and MUST be set when announcing the endpoint created as a result of a remotely initiated session.**]** 

...

Figure 2.26: Session Begin Sequence

2.5.2 Ending A Session

**S_R_S_SESSION_01_007: [**Sessions end automatically when the connection is closed or interrupted.**]** **S_R_S_SESSION_01_008: [**Sessions are explicitly ended when either endpoint chooses to end the session.**]** **S_R_S_SESSION_01_009: [**When a session is explicitly ended, an end frame is sent to announce the disassociation of the endpoint from its outgoing channel, and to carry error information when relevant.**]** 

...

Figure 2.27: Session End Sequence

2.5.3 Simultaneous End

Due to the potentially asynchronous nature of sessions, it is possible that both peers simultaneously decide to end a session. If this happens, it will appear to each peer as though their partner's spontaneously initiated end frame is actually an answer to the peers initial end frame.

...

Figure 2.28: Simultaneous Session End Sequence

2.5.4 Session Errors

**S_R_S_SESSION_01_010: [**When a session is unable to process input, it MUST indicate this by issuing an END with an appropriate error indicating the cause of the problem.**]** **S_R_S_SESSION_01_011: [**It MUST then proceed to discard all incoming frames from the remote endpoint until receiving the remote endpoint's corresponding end frame.**]** 

...

Figure 2.29: Session Error Sequence

2.5.5 Session States

UNMAPPED In the UNMAPPED state, the session endpoint is not mapped to any incoming or outgoing channels on the connection endpoint. In this state an endpoint cannot send or receive frames.

BEGIN SENT In the BEGIN SENT state, the session endpoint is assigned an outgoing channel number, but there is no entry in the incoming channel map. In this state the endpoint MAY send frames but cannot receive them.

BEGIN RCVD In the BEGIN RCVD state, the session endpoint has an entry in the incoming channel map, but has not yet been assigned an outgoing channel number. The endpoint MAY receive frames, but cannot send them.

MAPPED In the MAPPED state, the session endpoint has both an outgoing channel number and an entry in the incoming channel map. The endpoint MAY both send and receive frames.

END SENT In the END SENT state, the session endpoint has an entry in the incoming channel map, but is no longer assigned an outgoing channel number. The endpoint MAY receive frames, but cannot send them.

END RCVD In the END RCVD state, the session endpoint is assigned an outgoing channel number, but there is no entry in the incoming channel map. The endpoint MAY send frames, but cannot receive them.

DISCARDING The DISCARDING state is a variant of the END SENT state where the end is triggered by an error. In this case any incoming frames on the session MUST be silently discarded until the peer's end frame is received.

...

Figure 2.30: State Transitions

There is no obligation to retain a session endpoint after it transitions to the UNMAPPED state.

2.5.6 Session Flow Control

**S_R_S_SESSION_01_012: [**The session endpoint assigns each outgoing transfer frame an implicit transfer-id from a session scoped sequence.**]****S_R_S_SESSION_01_013: [**Each session endpoint maintains the following state to manage incoming and outgoing transfer frames**]** :
**S_R_S_SESSION_01_014: [**next-incoming-id The next-incoming-id identifies the expected transfer-id of the next incoming transfer frame.**]** 
**S_R_S_SESSION_01_015: [**incoming-window The incoming-window defines the maximum number of incoming transfer frames that the endpoint can currently receive.**]** This identifies a current maximum incoming transfer-id that can be computed by subtracting one from the sum of incoming-window and next-incomingid.
**S_R_S_SESSION_01_016: [**next-outgoing-id The next-outgoing-id is the transfer-id to assign to the next transfer frame.**]** **S_R_S_SESSION_01_017: [**The nextoutgoing-id MAY be initialized to an arbitrary value **]** and **S_R_S_SESSION_01_018: [**is incremented after each successive transfer according to RFC-1982 [RFC1982**]** serial number arithmetic.] 
**S_R_S_SESSION_01_019: [**outgoing-window The outgoing-window defines the maximum number of outgoing transfer frames that the endpoint can currently send.**]** This identifies a current maximum outgoing transfer-id that can be computed by subtracting one from the sum of outgoing-window and next-outgoing-id.
**S_R_S_SESSION_01_020: [**remote-incoming-window The remote-incoming-window reflects the maximum number of outgoing transfers that can be sent without exceeding the remote endpoint's incoming-window.**]****S_R_S_SESSION_01_021: [**This value MUST be decremented after every transfer frame is sent**]**,**S_R_S_SESSION_01_022: [**and recomputed when informed of the remote session endpoint state.**]** 
**S_R_S_SESSION_01_023: [**remote-outgoing-window The remote-outgoing-window reflects the maximum number of incoming transfers that MAY arrive without exceeding the remote endpoint's outgoing-window.**]****S_R_S_SESSION_01_024: [**This value MUST be decremented after every incoming transfer frame is received**]**, **S_R_S_SESSION_01_025: [**and recomputed when informed of the remote session endpoint state.**]** When this window shrinks, it is an indication of outstanding transfers. Settling outstanding transfers can cause the window to grow.
**S_R_S_SESSION_01_026: [**Once initialized, this state is updated by various events that occur in the lifespan of a session and its associated links:**]** 
**S_R_S_SESSION_01_027: [**sending a transfer Upon sending a transfer, the sending endpoint will increment its next-outgoing-id**]**, **S_R_S_SESSION_01_062: [**decrement its remote-incoming-window**]** ,**S_R_S_SESSION_01_063: [**and MAY (depending on policy) decrement its outgoing window**]** . 
**S_R_S_SESSION_01_028: [**receiving a transfer Upon receiving a transfer, the receiving endpoint will increment the next-incoming-id to match the implicit transfer-id of the incoming transfer plus one, as well as decrementing the remote-outgoing-window, and MAY (depending on policy) decrement its incoming-window.**]** 
**S_R_S_SESSION_01_029: [**receiving a flow When the endpoint receives a flow frame from its peer, it MUST update the next-incoming-id directly from the next-outgoing-id of the frame, and it MUST update the remote-outgoingwindow directly from the outgoing-window of the frame.**]** 

The remote-incoming-window is computed as follows:

next-incoming-idow + incoming-windowow - next-outgoing-idendpoint

If the next-incoming-id field of the flow frame is not set, then remote-incoming-window is computed as follows:

initial-outgoing-idendpoint + incoming-windowow - next-outgoing-idendpoint

Begin frame

2.7.2 Begin

Begin a session on a channel.

\<type name="begin" class="composite" source="list" provides="frame">

\<descriptor name="amqp:begin:list" code="0x00000000:0x00000011"/>

\<field name="remote-channel" type="ushort"/>

\<field name="next-outgoing-id" type="transfer-number" mandatory="true"/>

\<field name="incoming-window" type="uint" mandatory="true"/>

\<field name="outgoing-window" type="uint" mandatory="true"/>

\<field name="handle-max" type="handle" default="4294967295"/>

\<field name="offered-capabilities" type="symbol" multiple="true"/>

\<field name="desired-capabilities" type="symbol" multiple="true"/>

\<field name="properties" type="fields"/>

\</type>

Indicate that a session has begun on the channel.

Field Details

remote-channel the remote channel for this session

If a session is locally initiated, the remote-channel MUST NOT be set. When an endpoint responds
to a remotely initiated session, the remote-channel MUST be set to the channel on which the
remote session sent the begin.

next-outgoing-id the transfer-id of the first transfer id the sender will send

See subsection 2.5.6.

incoming-window the initial incoming-window of the sender

See subsection 2.5.6.

outgoing-window the initial outgoing-window of the sender

See subsection 2.5.6.

handle-max the maximum handle value that can be used on the session

The handle-max value is the highest handle value that can be used on the session. A peer MUST
NOT attempt to attach a link using a handle value outside the range that its partner can handle.
A peer that receives a handle outside the supported range MUST close the connection with the

framing-error error-code.

offered-capabilities the extension capabilities the sender supports

A registry of commonly defined session capabilities and their meanings is maintained
[AMQPSESSCAP].

desired-capabilities the extension capabilities the sender can use if the receiver supports them

The sender MUST NOT attempt to use any capability other than those it has declared in desiredcapabilities
field.

properties session properties

The properties map contains a set of fields intended to indicate information about the session and
its container.
A registry of commonly defined session properties and their meanings is maintained
[AMQPSESSPROP].

Transfer frame

2.7.5 Transfer

Transfer a message.

\<type name="transfer" class="composite" source="list" provides="frame">

\<descriptor name="amqp:transfer:list" code="0x00000000:0x00000014"/>

\<field name="handle" type="handle" mandatory="true"/>

\<field name="delivery-id" type="delivery-number"/>

\<field name="delivery-tag" type="delivery-tag"/>

\<field name="message-format" type="message-format"/>

\<field name="settled" type="boolean"/>

\<field name="more" type="boolean" default="false"/>

\<field name="rcv-settle-mode" type="receiver-settle-mode"/>

\<field name="state" type="*" requires="delivery-state"/>

\<field name="resume" type="boolean" default="false"/>

\<field name="aborted" type="boolean" default="false"/>

\<field name="batchable" type="boolean" default="false"/>

\</type>

The transfer frame is used to send messages across a link. Messages MAY be carried by a single transfer up to the maximum negotiated frame size for the connection. Larger messages MAY be split across several transfer frames.

Field Details

handle

Specifies the link on which the message is transferred.

delivery-id alias for delivery-tag

The delivery-id MUST be supplied on the first transfer of a multi-transfer delivery. On continuation
transfers the delivery-id MAY be omitted. It is an error if the delivery-id on a continuation transfer
differs from the delivery-id on the first transfer of a delivery.

delivery-tag

Uniquely identifies the delivery attempt for a given message on this link. This field MUST be
specified for the first transfer of a multi-transfer message and can only be omitted for continuation
transfers. It is an error if the delivery-tag on a continuation transfer differs from the delivery-tag on
the first transfer of a delivery.

message-format indicates the message format

This field MUST be specified for the first transfer of a multi-transfer message and can only be
omitted for continuation transfers. It is an error if the message-format on a continuation transfer
differs from the message-format on the first transfer of a delivery.

settled

If not set on the first (or only) transfer for a (multi-transfer) delivery, then the settled flag MUST be
interpreted as being false. For subsequent transfers in a multi-transfer delivery if the settled flag
is left unset then it MUST be interpreted as true if and only if the value of the settled flag on any
of the preceding transfers was true; if no preceding transfer was sent with settled being true then
the value when unset MUST be taken as false.
If the negotiated value for snd-settle-mode at attachment is settled, then this field MUST be true
on at least one transfer frame for a delivery (i.e., the delivery MUST be settled at the sender at
the point the delivery has been completely transferred).
If the negotiated value for snd-settle-mode at attachment is unsettled, then this field MUST be
false (or unset) on every transfer frame for a delivery (unless the delivery is aborted).

more indicates that the message has more content

Note that if both the more and aborted fields are set to true, the aborted flag takes precedence.
That is, a receiver SHOULD ignore the value of the more field if the transfer is marked as aborted.
A sender SHOULD NOT set the more flag to true if it also sets the aborted flag to true.

rcv-settle-mode

If first, this indicates that the receiver MUST settle the delivery once it has arrived without waiting
for the sender to settle first.
If second, this indicates that the receiver MUST NOT settle until sending its disposition to the
sender and receiving a settled disposition from the sender.
If not set, this value is defaulted to the value negotiated on link attach.
If the negotiated link value is first, then it is illegal to set this field to second.
If the message is being sent settled by the sender, the value of this field is ignored.
The (implicit or explicit) value of this field does not form part of the transfer state, and is not
retained if a link is suspended and subsequently resumed.

state the state of the delivery at the sender

When set this informs the receiver of the state of the delivery at the sender. This is particularly
useful when transfers of unsettled deliveries are resumed after resuming a link. Setting the state
on the transfer can be thought of as being equivalent to sending a disposition immediately before
the transfer performative, i.e., it is the state of the delivery (not the transfer) that existed at the
point the frame was sent.
Note that if the transfer performative (or an earlier disposition performative referring to the
delivery) indicates that the delivery has attained a terminal state, then no future transfer or
disposition sent by the sender can alter that terminal state.

resume indicates a resumed delivery

If true, the resume flag indicates that the transfer is being used to reassociate an unsettled delivery
from a dissociated link endpoint. See subsection 2.6.13 for more details.
The receiver MUST ignore resumed deliveries that are not in its local unsettled map. The sender
MUST NOT send resumed transfers for deliveries not in its local unsettled map.
If a resumed delivery spans more than one transfer performative, then the resume flag MUST be
set to true on the first transfer of the resumed delivery. For subsequent transfers for the same
delivery the resume flag MAY be set to true, or MAY be omitted.
In the case where the exchange of unsettled maps makes clear that all message data has been
successfully transferred to the receiver, and that only the final state (and potentially settlement) at
the sender needs to be conveyed, then a resumed delivery MAY carry no payload and instead act
solely as a vehicle for carrying the terminal state of the delivery at the sender.

aborted indicates that the message is aborted

Aborted messages SHOULD be discarded by the recipient (any payload within the frame carrying
the performative MUST be ignored). An aborted message is implicitly settled.

batchable batchable hint

If true, then the issuer is hinting that there is no need for the peer to urgently communicate
updated delivery state. This hint MAY be used to artificially increase the amount of batching
an implementation uses when communicating delivery states, and thereby save bandwidth.
If the message being delivered is too large to fit within a single frame, then the setting of batchable
to true on any of the transfer performatives for the delivery is equivalent to setting batchable to
true for all the transfer performatives for the delivery.
The batchable value does not form part of the transfer state, and is not retained if a link is suspended
and subsequently resumed.
