// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words repr amqp amqpsession amqpmessagereceiver amqpmessagereceiveroptions amqpmessagereceiverchannel spmc

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str, error_from_string,
    model::{
        message::RustAmqpMessage, source::RustAmqpSource, target::RustAmqpTarget,
        value::RustAmqpValue,
    },
};
use azure_core::Result;
use azure_core_amqp::{
    messaging::AmqpMessage,
    receiver::{AmqpReceiver, AmqpReceiverApis, AmqpReceiverOptions, ReceiverCreditMode},
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};
use std::{ffi::c_char, mem, ptr};
use tokio::spawn;
use tracing::{debug, error, trace};

use super::{message_sender::RustReceiverSettleMode, session::RustAmqpSession};

pub struct RustAmqpMessageReceiverOptions {
    inner: AmqpReceiverOptions,
}

pub struct RustAmqpMessageReceiver {
    inner: AmqpReceiver,
}

impl RustAmqpMessageReceiver {}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_create() -> *mut RustAmqpMessageReceiver {
    Box::into_raw(Box::new(RustAmqpMessageReceiver {
        inner: AmqpReceiver::new(),
    }))
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_destroy(receiver: *mut RustAmqpMessageReceiver) {
    if !receiver.is_null() {
        trace!("Destroying MessageReceiver");
        mem::drop(Box::from_raw(receiver));
    }
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_attach(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
    session: *mut RustAmqpSession,
    source: *mut RustAmqpSource,
    options: *mut RustAmqpMessageReceiverOptions,
) -> i32 {
    if receiver.is_null() || session.is_null() || source.is_null() || options.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let session = { &*session };
    let source = { &*source };

    let receiver = &mut *receiver;
    let options = &*options;
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to attach receiver");
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(receiver.inner.attach(
            session.get_session(),
            source.get().clone(),
            Some(options.inner.clone()),
        ));
    trace!("Attached receiver");
    if let Err(err) = result {
        error!("Failed to attach receiver: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_detach_and_release(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
) -> i32 {
    if receiver.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let receiver = Box::from_raw(receiver);
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to detach receiver");
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(receiver.inner.detach());
    trace!("Detached receiver");
    if let Err(err) = result {
        error!("Failed to detach receiver: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_receive_message(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
) -> *mut RustAmqpMessage {
    if receiver.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Null message receiver."));
        return std::ptr::null_mut();
    }

    let receiver = &mut *receiver;
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to receive message");

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(receiver.inner.receive());
    trace!("Received message");
    match result {
        Ok(message) => Box::into_raw(Box::new(RustAmqpMessage::from(message))),
        Err(err) => {
            error!("Failed to receive message: {:?}", err);
            call_context.set_error(Box::new(err));
            std::ptr::null_mut()
        }
    }
}

struct RustMessageReceiverChannel {
    inner: std::sync::mpsc::Receiver<Result<AmqpMessage>>,
    task: tokio::task::JoinHandle<()>,
}

impl RustMessageReceiverChannel {
    fn new(
        receiver: std::sync::mpsc::Receiver<Result<AmqpMessage>>,
        task: tokio::task::JoinHandle<()>,
    ) -> Self {
        Self {
            inner: receiver,
            task,
        }
    }
}

/// Destroy a RustMessageReceiverFuture
#[no_mangle]
unsafe extern "C" fn amqpmessagereceiverchannel_destroy(
    call_context: *mut RustCallContext,
    channel: *mut RustMessageReceiverChannel,
) {
    if !channel.is_null() {
        let channel = Box::from_raw(channel);
        trace!("Destroying MessageReceiverChannel");
        //      let call_context = call_context_from_ptr_mut(call_context);
        channel.task.abort();
        trace!("Aborted task.");
        //        let res = call_context
        //            .runtime_context()
        //            .runtime()
        //            .block_on(async { channel.task.await });
        //    trace!("Task joined: {:?}.", res);
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_receive_message_async(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
) -> *mut RustMessageReceiverChannel {
    let call_context = call_context_from_ptr_mut(call_context);
    if receiver.is_null() {
        call_context.set_error(error_from_str("Null message receiver."));
        return ptr::null_mut();
    }

    let receiver = &mut *receiver;
    //    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to receive message");

    let (tx, rx) = std::sync::mpsc::channel();

    let task = call_context.runtime_context().runtime().block_on(async {
        spawn(async move {
            loop {
                trace!("Polling for message");
                let message = receiver.inner.receive().await;
                if tx.send(message).is_err() {
                    error!("Failed to send message to channel");
                    break;
                }
            }
        })
    });

    let receiver_channel = RustMessageReceiverChannel::new(rx, task);
    return Box::into_raw(Box::new(receiver_channel));
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_receive_message_async_poll(
    call_context: *mut RustCallContext,
    channel: *mut RustMessageReceiverChannel,
) -> *mut RustAmqpMessage {
    debug!("Polling for message");
    let call_context = call_context_from_ptr_mut(call_context);
    if channel.is_null() {
        call_context.set_error(error_from_str("Null receiver channel."));
        return ptr::null_mut();
    }
    let channel = &mut *channel;

    let output = channel.inner.try_recv();
    trace!("Poll returned {:?}", output);

    match output {
        Err(err) => {
            trace!("Failed to receive message: {:?}", err);
            call_context.set_error(Box::new(err));
            ptr::null_mut()
        }
        Ok(Err(err)) => {
            error!("Error receiving: {:?}", err);
            call_context.set_error(Box::new(err));
            ptr::null_mut()
        }
        Ok(Ok(message)) => {
            trace!("Received message: {:?}", message);
            Box::into_raw(Box::new(RustAmqpMessage::from(message)))
        }
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_receive_message_async_wait(
    call_context: *mut RustCallContext,
    channel: *mut RustMessageReceiverChannel,
) -> *mut RustAmqpMessage {
    let call_context = call_context_from_ptr_mut(call_context);
    if channel.is_null() {
        call_context.set_error(error_from_str("Null Receiver Channel."));
        return ptr::null_mut();
    }

    let channel = &mut *channel;
    let output = channel.inner.recv();
    match output {
        Err(err) => {
            trace!("Failed to receive message: {:?}", err);
            call_context.set_error(Box::new(err));
            ptr::null_mut()
        }
        Ok(Err(err)) => {
            error!("Failed to receive message: {:?}", err);
            call_context.set_error(Box::new(err));
            ptr::null_mut()
        }
        Ok(Ok(message)) => {
            trace!("Received message: {:?}", message);
            Box::into_raw(Box::new(RustAmqpMessage::from(message)))
        }
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_create() -> *mut RustAmqpMessageReceiverOptions {
    Box::into_raw(Box::new(RustAmqpMessageReceiverOptions {
        inner: AmqpReceiverOptions::default(),
    }))
}

/// # Safety
#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_destroy(
    options: *mut RustAmqpMessageReceiverOptions,
) {
    if !options.is_null() {
        trace!("Destroying MessageReceiverOptions");
        mem::drop(Box::from_raw(options));
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_receiver_settle_mode(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    settle_mode: RustReceiverSettleMode,
) -> i32 {
    if options.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    options.inner.receiver_settle_mode =
        Some(azure_core_amqp::ReceiverSettleMode::from(settle_mode));
    0
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_target(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    target: *mut RustAmqpTarget,
) -> i32 {
    if options.is_null() || target.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    let target = &*target;
    options.inner.target = Some(target.get().clone());
    0
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_name(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    name: *const c_char,
) -> i32 {
    if options.is_null() || name.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    let name = std::ffi::CStr::from_ptr(name).to_str().unwrap();
    options.inner.name = Some(name.to_string());
    0
}

#[repr(C)]
pub struct RustReceiverCreditMode {
    pub manual: bool,
    pub credit_mode: u32,
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_credit_mode(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    credit_mode: RustReceiverCreditMode,
) -> i32 {
    if options.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    if credit_mode.manual {
        options.inner.credit_mode = Some(ReceiverCreditMode::Manual);
    } else {
        options.inner.credit_mode = Some(ReceiverCreditMode::Auto(credit_mode.credit_mode));
    }
    0
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_auto_accept(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    auto_accept: bool,
) -> i32 {
    if options.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    options.inner.auto_accept = auto_accept;
    0
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiveroptions_set_properties(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpMessageReceiverOptions,
    properties: *mut RustAmqpValue,
) -> i32 {
    if options.is_null() || properties.is_null() {
        call_context_from_ptr_mut(call_context).set_error(error_from_str("Invalid input"));
        error!("Invalid input");
        return -1;
    }

    let options = &mut *options;
    let properties = &*properties;
    match properties.inner {
        AmqpValue::Map(ref map) => {
            let properties_map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = map
                .iter()
                .map(|(k, v)| (AmqpSymbol::from(k.clone()), v.clone()))
                .collect();
            let options = &mut *options;
            options.inner.properties = Some(properties_map);
            0
        }
        _ => {
            let call_context = call_context_from_ptr_mut(call_context);
            call_context.set_error(error_from_string(format!(
                "Invalid properties: {:?}",
                properties.inner
            )));
            -1
        }
    }
}
