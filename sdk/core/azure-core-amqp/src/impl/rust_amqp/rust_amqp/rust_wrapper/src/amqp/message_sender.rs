// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words repr amqp amqpsession amqpmessagesender amqpmessagesenderoptions

use azure_core_amqp::{
    sender::{
        builders::AmqpSenderOptionsBuilder, AmqpSendOptions, AmqpSender, AmqpSenderApis, AmqpSenderOptions
    }, value::{AmqpOrderedMap, AmqpSymbol, AmqpValue}, ReceiverSettleMode, SenderSettleMode
};
use std::{ffi::c_char, mem};
use tracing::{error, trace};

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    model::{message::RustAmqpMessage, source::RustAmqpSource, target::RustAmqpTarget, value::RustAmqpValue},
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

pub struct RustAmqpMessageSender {
    inner: AmqpSender,
}

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

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_attach(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
    session: *mut RustAmqpSession,
    name: *const c_char,
    target: *mut RustAmqpTarget,
    options: *mut RustAmqpSenderOptions,
) -> i32{
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

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(sender.inner.attach(
            session.get_session(),
            name,
            target.get().clone(),
            Some(options.inner.clone()),
        ));
    if let Err(err) = result {
        error!("Failed to attach sender: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    }
    else {
        0
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesender_send(
    call_context: *mut RustCallContext,
    sender: *mut RustAmqpMessageSender,
    message: *mut RustAmqpMessage,
    options: *mut RustAmqpSendOptions,
) {
    if sender.is_null() || message.is_null() || options.is_null() {
        error!("Invalid input");
        return;
    }

    let sender = &mut *sender;
    let message = &*message;
    let options = &*options;
    let call_context = call_context_from_ptr_mut(call_context);

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(sender.inner.send(message.get().clone(), Some(options.inner.clone())));
    if let Err(err) = result {
        error!("Failed to send message: {:?}", err);
        call_context.set_error(Box::new(err));
    }
}

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
        Ok(size) => {*max_size = size.unwrap_or(0); 0},
        Err(err) => {
            *max_size = 0;
            error!("Failed to get max message size: {:?}", err);
            call_context.set_error(Box::new(err));
            -1
        }
    }
}

pub struct RustAmqpSenderOptionsBuilder {
    inner: AmqpSenderOptionsBuilder, //
}

pub struct RustAmqpSenderOptions {
    inner: AmqpSenderOptions,
}

pub struct RustAmqpSendOptions {
    inner: AmqpSendOptions,
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_create(
) -> *mut RustAmqpSenderOptionsBuilder {
    Box::into_raw(Box::new(RustAmqpSenderOptionsBuilder {
        inner: AmqpSenderOptions::builder(),
    }))
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_destroy(
    options: *mut RustAmqpSenderOptionsBuilder,
) {
    if !options.is_null() {
        drop(Box::from_raw(options));
    } else {
        error!("Options builder cannot be null.");
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_destroy(options: *mut RustAmqpSenderOptions) {
    if !options.is_null() {
        drop(Box::from_raw(options));
    } else {
        error!("Options cannot be null.");
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_build(
    builder: *const RustAmqpSenderOptionsBuilder,
) -> *mut RustAmqpSenderOptions {
    if builder.is_null() {
        error!("Invalid input");
        return std::ptr::null_mut();
    }
    let builder = &*builder;
    Box::into_raw(Box::new(RustAmqpSenderOptions {
        inner: builder.inner.clone().build(),
    }))
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_sender_settle_mode(
    builder: *mut RustAmqpSenderOptionsBuilder,
    settle_mode: RustSenderSettleMode,
) {
    let sender_settle_mode = match RustSenderSettleMode::from(settle_mode) {
        RustSenderSettleMode::Unsettled => SenderSettleMode::Unsettled,
        RustSenderSettleMode::Settled => SenderSettleMode::Settled,
        RustSenderSettleMode::Mixed => SenderSettleMode::Mixed,
    };
    if !builder.is_null() {
        let builder = &mut *builder;
        builder.inner.with_sender_settle_mode(sender_settle_mode);
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_receiver_settle_mode(
    builder: *mut RustAmqpSenderOptionsBuilder,
    settle_mode: RustReceiverSettleMode
) {
    let receiver_settle_mode = match RustReceiverSettleMode::from(settle_mode) {
        RustReceiverSettleMode::First => ReceiverSettleMode::First,
        RustReceiverSettleMode::Second => ReceiverSettleMode::Second,
    };

    if !builder.is_null() {
        let builder = &mut *builder;
        builder.inner.with_receiver_settle_mode(receiver_settle_mode);
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_source(
    builder: *mut RustAmqpSenderOptionsBuilder,
    source: *mut RustAmqpSource,
) {
    if !builder.is_null() && !source.is_null() {
        let builder = &mut *builder;
        let source = &*source;
        builder.inner.with_source(source.get().clone());
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_offered_capabilities(
    builder: *mut RustAmqpSenderOptionsBuilder,
    offered_capabilities: *mut RustAmqpValue) {
    if !builder.is_null() && !offered_capabilities.is_null() {
        let builder = &mut *builder;
        let offered_capabilities = &*offered_capabilities;
        match offered_capabilities.inner {
            AmqpValue::Array(ref array) => {
                let capabilities = array.iter().map(|c| match c {
                     AmqpValue::Symbol(symbol)  => AmqpSymbol::from(symbol.clone()),
                    _ => panic!("Invalid offered capability: {:?}", c),
                }).collect();
                builder.inner.with_offered_capabilities(capabilities);
            }
            _ => error!("Invalid offered capabilities: {:?}", offered_capabilities),
        }        
    }
}
#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_desired_capabilities(
    builder: *mut RustAmqpSenderOptionsBuilder,
    desired_capabilities: *mut RustAmqpValue) {
    if !builder.is_null() && !desired_capabilities.is_null() {
        let builder = &mut *builder;
        let desired_capabilities = &*desired_capabilities;
        match desired_capabilities.inner {
            AmqpValue::Array(ref array) => {
                let capabilities = array.iter().map(|c| match c {
                     AmqpValue::Symbol(symbol)  => AmqpSymbol::from(symbol.clone()),
                    _ => panic!("Invalid desired capability: {:?}", c),
                }).collect();
                builder.inner.with_offered_capabilities(capabilities);
            }
            _ => error!("Invalid desired capabilities: {:?}", desired_capabilities.inner),
        }        
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_properties(
    builder: *mut RustAmqpSenderOptionsBuilder,
    properties: *mut RustAmqpValue) {
    if !builder.is_null() && !properties.is_null() {
        let builder = &mut *builder;
        let properties = &*properties;
        match properties.inner {
            AmqpValue::Map(ref map) => {
                let properties_map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = map.iter().map(|(k, v)| {
                    (AmqpSymbol::from(k.clone()), v.clone())
                }).collect();
                builder.inner.with_properties(properties_map);
            }
            _ => error!("Invalid properties: {:?}", properties),
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_initial_delivery_count(
    builder: *mut RustAmqpSenderOptionsBuilder,
    initial_delivery_count: u32,
) {
    if !builder.is_null() {
        let builder = &mut *builder;
        builder.inner.with_initial_delivery_count(initial_delivery_count);
    }
}

#[no_mangle]
pub unsafe extern "C" fn amqpmessagesenderoptions_builder_set_max_message_size(
    builder: *mut RustAmqpSenderOptionsBuilder,
    max_message_size: u64,
) {
    if !builder.is_null() {
        let builder = &mut *builder;
        builder.inner.with_max_message_size(max_message_size);
    }
}
