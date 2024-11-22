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
use std::{
    ffi::c_char,
    mem, ptr,
    sync::{Arc, OnceLock},
    thread::sleep,
};
use tokio::spawn;
use tracing::{debug, error, trace};

use super::{message_sender::RustReceiverSettleMode, session::RustAmqpSession};

pub struct RustAmqpMessageReceiverOptions {
    inner: AmqpReceiverOptions,
}

/// AMQP Message Receiver.
///
/// This is a wrapper around the AMQP receiver.
///
/// It has three field.
/// inner - the underlying AmqpReceiver.
/// message_channel - the channel that will be used to receive messages.
/// message_task - the task that will be used to receive messages.
///
/// The inner field is an Arc<OnceLock<AmqpReceiver>>. The Arc is there because we will be passing the receiver to the message task and we need to be able to clone it.
/// Within the Arc is an OnceLock<AmqpReceiver>. This is because the act of detaching the message receiver consumes self and we need to be able to take it out of the Arc.
pub struct RustAmqpMessageReceiver {
    inner: Arc<AmqpReceiver>,
    message_channel: OnceLock<std::sync::mpsc::Receiver<Result<AmqpMessage>>>,
    message_task: OnceLock<tokio::task::JoinHandle<()>>,
}

impl RustAmqpMessageReceiver {
    fn new() -> Self {
        Self {
            inner: Arc::new(AmqpReceiver::new()),
            message_channel: OnceLock::new(),
            message_task: OnceLock::new(),
        }
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_create() -> *mut RustAmqpMessageReceiver {
    Box::into_raw(Box::new(RustAmqpMessageReceiver::new()))
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
        return -1;
    }

    if receiver.message_channel.get().is_some() {
        call_context.set_error(error_from_str("Receiver already has a channel."));
        return -1;
    }
    if receiver.message_task.get().is_some() {
        call_context.set_error(error_from_str("Receiver already has a task."));
        return -1;
    }
    trace!("Starting to receive messages");

    let (tx, rx) = std::sync::mpsc::channel();

    let res = receiver.message_channel.set(rx);
    // This should never happen because we checked to make sure that the message_channel was None earlier, but we need to handle it anyway.
    match res {
        Ok(_) => {}
        Err(_) => {
            error!("Failed to set message channel");
            call_context.set_error(error_from_str(
                "Failed to initialize message receiver channel.",
            ));
            return -1;
        }
    }
    let message_receiver = receiver.inner.clone(); // This is the receiver that will be used to receive messages

    // The call to .set should never fail because we checked to make sure that the message_task was None earlier, but we need to handle it anyway.
    match receiver
        .message_task
        .set(call_context.runtime_context().runtime().block_on(async {
            spawn(async move {
                loop {
                    trace!("Polling for message");
                    let message = message_receiver.receive().await;
                    if let Err(err) = tx.send(message) {
                        error!("Failed to send message to channel: {:?}", err);
                        break;
                    }
                }
            })
        })) {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to start receiving messages: {:?}", err);
            call_context.set_error(error_from_str("Unable set message task."));
            -1
        }
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
    //let receiver = receiver.deref_mut();
    if receiver.message_task.get().is_none() {
        call_context_from_ptr_mut(call_context)
            .set_error(error_from_str("Receiver does not have a task."));
        return -1;
    }
    trace!("Aborting message task");
    receiver.message_task.get().unwrap().abort();
    trace!("Aborted message task");
    sleep(std::time::Duration::from_millis(500));
    trace!("Dropping receiver");
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to detach receiver");
    let receiver_to_detach = Arc::into_inner(receiver.inner);
    if receiver_to_detach.is_none() {
        trace!("Could not extract receiver from arc, non zero reference count.");
        call_context.set_error(error_from_str("Receiver has a non zero reference count!"));
        return -1;
    }
    trace!("Detaching receiver");
    let receiver_to_detach = receiver_to_detach.unwrap();
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(receiver_to_detach.detach());
    trace!("Detached receiver");
    if let Err(err) = result {
        error!("Failed to detach receiver: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

// #[no_mangle]
// unsafe extern "C" fn amqpmessagereceiver_receive_message(
//     call_context: *mut RustCallContext,
//     receiver: *mut RustAmqpMessageReceiver,
// ) -> *mut RustAmqpMessage {
//     if receiver.is_null() {
//         call_context_from_ptr_mut(call_context).set_error(error_from_str("Null message receiver."));
//         return std::ptr::null_mut();
//     }

//     let receiver = &mut *receiver;
//     let call_context = call_context_from_ptr_mut(call_context);
//     trace!("Starting to receive message");

//     let result = call_context
//         .runtime_context()
//         .runtime()
//         .block_on(receiver.inner.receive());
//     trace!("Received message");
//     match result {
//         Ok(message) => Box::into_raw(Box::new(RustAmqpMessage::from(message))),
//         Err(err) => {
//             error!("Failed to receive message: {:?}", err);
//             call_context.set_error(Box::new(err));
//             std::ptr::null_mut()
//         }
//     }
// }

// struct RustMessageReceiverChannel {
//     inner: std::sync::mpsc::Receiver<Result<AmqpMessage>>,
//     task: tokio::task::JoinHandle<()>,
// }

// impl RustMessageReceiverChannel {
//     fn new(
//         receiver: std::sync::mpsc::Receiver<Result<AmqpMessage>>,
//         task: tokio::task::JoinHandle<()>,
//     ) -> Self {
//         Self {
//             inner: receiver,
//             task,
//         }
//     }
// }

// /// Destroy a RustMessageReceiverFuture
// #[no_mangle]
// unsafe extern "C" fn amqpmessagereceiverchannel_destroy(
//     call_context: *mut RustCallContext,
//     channel: *mut RustMessageReceiverChannel,
// ) {
//     if !channel.is_null() {
//         let channel = Box::from_raw(channel);
//         trace!("Destroying MessageReceiverChannel");
//         //      let call_context = call_context_from_ptr_mut(call_context);
//         channel.task.abort();
//         trace!("Aborted task.");
//         //        let res = call_context
//         //            .runtime_context()
//         //            .runtime()
//         //            .block_on(async { channel.task.await });
//         //    trace!("Task joined: {:?}.", res);
//     }
// }

// #[no_mangle]
// unsafe extern "C" fn amqpmessagereceiver_receive_message_async(
//     call_context: *mut RustCallContext,
//     receiver: *mut RustAmqpMessageReceiver,
// ) -> i32 {
//     let call_context = call_context_from_ptr_mut(call_context);
//     if receiver.is_null() {
//         call_context.set_error(error_from_str("Null message receiver."));
//         return -1;
//     }

//     let receiver = &mut *receiver;

//     0
// }

#[no_mangle]
unsafe extern "C" fn amqpmessagereceiver_receive_message_async_poll(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
) -> *mut RustAmqpMessage {
    let call_context = call_context_from_ptr_mut(call_context);
    if receiver.is_null() {
        call_context.set_error(error_from_str("Null receiver."));
        return ptr::null_mut();
    }
    let receiver = &mut *receiver;

    if receiver.message_channel.get().is_none() {
        call_context.set_error(error_from_str("Receiver does not have a channel."));
        return ptr::null_mut();
    }

    debug!("Polling for message");
    let output = receiver.message_channel.get().unwrap().try_recv();
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
unsafe extern "C" fn amqpmessagereceiver_receive_message_wait(
    call_context: *mut RustCallContext,
    receiver: *mut RustAmqpMessageReceiver,
) -> *mut RustAmqpMessage {
    let call_context = call_context_from_ptr_mut(call_context);
    if receiver.is_null() {
        call_context.set_error(error_from_str("Null Receiver Channel."));
        return ptr::null_mut();
    }

    let receiver = &mut *receiver;

    if receiver.message_channel.get().is_none() {
        call_context.set_error(error_from_str("Receiver does not have a channel."));
        return ptr::null_mut();
    }

    let output = receiver.message_channel.get().unwrap().recv();
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
