// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words repr amqp amqpsession amqpmessagesender amqpmessagesenderoptions

use azure_core_amqp::{
    sender::{AmqpSendOptions, AmqpSender, AmqpSenderApis, AmqpSenderOptions},
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
    ReceiverSettleMode, SenderSettleMode,
};
use std::{ffi::c_char, mem};
use tracing::{error, trace};

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str, error_from_string,
    model::{
        message::RustAmqpMessage, source::RustAmqpSource, target::RustAmqpTarget,
        value::RustAmqpValue,
    },
};

use super::session::RustAmqpSession;

#[repr(C)]
pub enum RustSenderSettleMode {
    Unsettled,
    Settled,
    Mixed,
}

#[repr(C)]
pub enum RustReceiverSettleMode {
    First,
    Second,
}

impl From<ReceiverSettleMode> for RustReceiverSettleMode {
    fn from(mode: ReceiverSettleMode) -> Self {
        match mode {
            ReceiverSettleMode::First => RustReceiverSettleMode::First,
            ReceiverSettleMode::Second => RustReceiverSettleMode::Second,
        }
    }
}

impl From<RustReceiverSettleMode> for ReceiverSettleMode {
    fn from(mode: RustReceiverSettleMode) -> Self {
        match mode {
            RustReceiverSettleMode::First => ReceiverSettleMode::First,
            RustReceiverSettleMode::Second => ReceiverSettleMode::Second,
        }
    }
}

pub struct RustAmqpMessageSender {
    inner: AmqpSender,
}

impl RustAmqpMessageSender {}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_create() -> *mut RustAmqpMessageSender {
    Box::into_raw(Box::new(RustAmqpMessageSender {
        inner: AmqpSender::new(),
    }))
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_destroy(sender: *mut RustAmqpMessageSender) {
    if !sender.is_null() {
        trace!("Destroying MessageSender");
        mem::drop(Box::from_raw(sender));
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_attach(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
    session: *mut RustAmqpSession,
    name: *const c_char,
    target: *mut RustAmqpTarget,
    options: *mut RustAmqpSenderOptions,
) -> i32 {
    if sender.is_null()
        || session.is_null()
        || name.is_null()
        || target.is_null()
        || options.is_null()
    {
        error!("Invalid input");
        return -1;
    }

    let session = { &*session };
    let name = std::ffi::CStr::from_ptr(name).to_str().unwrap();
    let target = { &*target };

    let sender = &mut *sender;
    let options = &*options;
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to attach sender");
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(sender.inner.attach(
            session.get_session(),
            name.into(),
            target.get().clone(),
            Some(options.inner.clone()),
        ));
    trace!("Attached sender");
    if let Err(err) = result {
        error!("Failed to attach sender: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_detach_and_release(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
) -> i32 {
    if sender.is_null() {
        error!("Invalid input");
        return -1;
    }
    let call_context = call_context_from_ptr_mut(call_context);

    trace!("Closing sender");
    let sender = Box::from_raw(sender);
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(sender.inner.detach());
    trace!("Closed sender");
    if let Err(err) = result {
        error!("Failed to detach sender: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_send(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
    message: *mut RustAmqpMessage,
    options: *mut RustAmqpSendOptions,
) -> i32 {
    if sender.is_null() || message.is_null() {
        error!("Invalid input");
        return -1;
    }

    let sender = &mut *sender;
    let message = &*message;
    let call_context = call_context_from_ptr_mut(call_context);
    let mut send_options: Option<AmqpSendOptions> = None;
    if !options.is_null() {
        let mut options_to_set = AmqpSendOptions::default();
        let options = &*options;
        if !options.message_format.is_null() {
            options_to_set.message_format = Some(*options.message_format);
        }
        if !options.settled.is_null() {
            options_to_set.settled = Some(*options.settled);
        }
        send_options = Some(options_to_set);
    }

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(sender.inner.send(message.get().clone(), send_options));
    if let Err(err) = result {
        error!("Failed to send message: {:?}", err);
        call_context.set_error(Box::new(err));
        return -1;
    }
    0
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_get_max_message_size(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
    max_size: *mut u64,
) -> i64 {
    if sender.is_null() {
        error!("Invalid input");
        return 0;
    }

    let sender = &mut *sender;
    let call_context = call_context_from_ptr_mut(call_context);

    let result = sender.inner.max_message_size();
    match result {
        Ok(size) => {
            *max_size = size.unwrap_or(0);
            0
        }
        Err(err) => {
            *max_size = 0;
            error!("Failed to get max message size: {:?}", err);
            call_context.set_error(Box::new(err));
            -1
        }
    }
}

pub struct RustAmqpSenderOptions {
    inner: AmqpSenderOptions,
}
#[repr(C)]
pub struct RustAmqpSendOptions {
    message_format: *const u32,
    settled: *const bool,
}

#[no_mangle]
unsafe extern "C" fn amqpmessagesenderoptions_create() -> *mut RustAmqpSenderOptions {
    Box::into_raw(Box::new(RustAmqpSenderOptions {
        inner: AmqpSenderOptions::default(),
    }))
}

#[no_mangle]
unsafe extern "C" fn amqpmessagesenderoptions_destroy(options: *mut RustAmqpSenderOptions) {
    if !options.is_null() {
        drop(Box::from_raw(options));
    } else {
        error!("Options cannot be null.");
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_sender_settle_mode(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    settle_mode: RustSenderSettleMode,
) -> i32 {
    let sender_settle_mode = match settle_mode {
        RustSenderSettleMode::Unsettled => SenderSettleMode::Unsettled,
        RustSenderSettleMode::Settled => SenderSettleMode::Settled,
        RustSenderSettleMode::Mixed => SenderSettleMode::Mixed,
    };
    if !options.is_null() {
        let options = &mut *options;
        options.inner.sender_settle_mode = Some(sender_settle_mode);
        0
    } else {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Options cannot be null."));
        -1
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_receiver_settle_mode(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    settle_mode: RustReceiverSettleMode,
) -> i32 {
    let receiver_settle_mode = match settle_mode {
        RustReceiverSettleMode::First => ReceiverSettleMode::First,
        RustReceiverSettleMode::Second => ReceiverSettleMode::Second,
    };

    if !options.is_null() {
        let options = &mut *options;
        options.inner.receiver_settle_mode = Some(receiver_settle_mode);
        0
    } else {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Options cannot be null."));
        -1
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_source(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    source: *mut RustAmqpSource,
) -> i32 {
    if !options.is_null() && !source.is_null() {
        let options = &mut *options;
        let source = &*source;
        options.inner.source = Some(source.get().clone());
        0
    } else {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Options and source cannot be null."));
        -1
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_offered_capabilities(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    offered_capabilities: *mut RustAmqpValue,
) -> i32 {
    if !options.is_null() && !offered_capabilities.is_null() {
        let options = &mut *options;
        let offered_capabilities = &*offered_capabilities;
        match offered_capabilities.inner {
            AmqpValue::Array(ref array) => {
                let capabilities = array
                    .iter()
                    .map(|c| match c {
                        AmqpValue::Symbol(symbol) => symbol.clone(),
                        _ => panic!("Invalid offered capability: {:?}", c),
                    })
                    .collect();
                let options = &mut *options;
                options.inner.offered_capabilities = Some(capabilities);
                0
            }
            _ => {
                let call_context = call_context_from_ptr_mut(call_context);
                call_context.set_error(error_from_string(format!(
                    "Invalid offered capabilities: {:?}",
                    offered_capabilities
                )));
                -1
            }
        }
    } else {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str(
            "Options and offered capabilities cannot be null.",
        ));
        -1
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_desired_capabilities(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    desired_capabilities: *mut RustAmqpValue,
) -> i32 {
    if !options.is_null() && !desired_capabilities.is_null() {
        let desired_capabilities = &*desired_capabilities;
        let call_context = call_context_from_ptr_mut(call_context);
        match desired_capabilities.inner {
            AmqpValue::Array(ref array) => {
                let capabilities = array
                    .iter()
                    .map(|c| match c {
                        AmqpValue::Symbol(symbol) => symbol.clone(),
                        _ => {
                            call_context.set_error(error_from_string(format!(
                                "Invalid desired capability: {:?}",
                                c
                            )));
                            panic!("Invalid desired capability: {:?}", c);
                        }
                    })
                    .collect();
                let options = &mut *options;
                options.inner.desired_capabilities = Some(capabilities);
                0
            }
            _ => {
                call_context.set_error(error_from_string(format!(
                    "Invalid desired capabilities: {:?}",
                    desired_capabilities.inner
                )));
                -1
            }
        }
    } else {
        error!("Options and desired capabilities cannot be null.");
        -1
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagesenderoptions_set_properties(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    properties: *mut RustAmqpValue,
) -> i32 {
    if !options.is_null() && !properties.is_null() {
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
    } else {
        error!("Options and properties cannot be null.");
        -1
    }
}

#[no_mangle]
unsafe extern "C" fn amqpmessagesenderoptions_set_initial_delivery_count(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    initial_delivery_count: u32,
) -> i32 {
    if !options.is_null() {
        let options = &mut *options;
        options.inner.initial_delivery_count = Some(initial_delivery_count);
        0
    } else {
        call_context_from_ptr_mut(call_context)
            .set_error(error_from_str("Options cannot be null."));
        -1
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_set_max_message_size(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpSenderOptions,
    max_message_size: u64,
) -> i32 {
    if !options.is_null() {
        let options = &mut *options;
        options.inner.max_message_size = Some(max_message_size);
        0
    } else {
        call_context_from_ptr_mut(call_context)
            .set_error(error_from_str("Options cannot be null."));
        -1
    }
}
