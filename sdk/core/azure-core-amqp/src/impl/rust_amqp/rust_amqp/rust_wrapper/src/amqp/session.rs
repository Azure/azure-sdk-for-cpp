// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionoptionsbuilder amqpconnectionoptions amqpsession amqpsessionoptions amqpsessionoptionsbuilder

use crate::{
    amqp::connection::RustAmqpConnection,
    call_context::{call_context_from_ptr_mut, RustCallContext},
    model::value::RustAmqpValue,
};
use azure_core_amqp::{
    session::{
        builders::AmqpSessionOptionsBuilder, AmqpSession, AmqpSessionApis, AmqpSessionOptions,
    },
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};
use std::{mem, ptr};
use tracing::error;

pub struct RustAmqpSession {
    inner: AmqpSession,
}

pub struct RustAmqpSessionOptionsBuilder {
    inner: AmqpSessionOptionsBuilder,
}

pub struct RustAmqpSessionOptions {
    inner: AmqpSessionOptions,
}

impl RustAmqpSession {
    pub(crate) fn get_session(&self) -> &AmqpSession {
        &self.inner
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsession_create() -> *mut RustAmqpSession {
    Box::into_raw(Box::new(RustAmqpSession {
        inner: AmqpSession::new(),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsession_destroy(session: *mut RustAmqpSession) {
    unsafe {
        mem::drop(Box::from_raw(session));
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsession_begin(
    call_context: *mut RustCallContext,
    session: *mut RustAmqpSession,
    connection: *mut RustAmqpConnection,
    session_options: *mut RustAmqpSessionOptions,
) -> i32 {
    let session = unsafe { &mut *session };
    let connection = unsafe { &mut *connection };
    let call_context = call_context_from_ptr_mut(call_context);

    if session_options.is_null() {
        let result = call_context
            .runtime_context()
            .runtime()
            .block_on(session.inner.begin(connection.get_connection(), None));
        match result {
            Ok(()) => 0,
            Err(err) => {
                error!("Failed to open connection with session options: {:?}", err);
                call_context.set_error(err.into());
                1
            }
        }
    } else {
        let session_options = unsafe { &*session_options };
        let result = call_context
            .runtime_context()
            .runtime()
            .block_on(session.inner.begin(
                connection.get_connection(),
                Some(session_options.inner.clone()),
            ));
        match result {
            Ok(()) => 0,
            Err(err) => {
                error!("Failed to open connection with session options: {:?}", err);
                call_context.set_error(err.into());
                1
            }
        }
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsession_end(
    call_context: *mut RustCallContext,
    session: *mut RustAmqpSession,
) -> i32 {
    let session = unsafe { &*session };
    let call_context = call_context_from_ptr_mut(call_context);
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(session.inner.end());
    match result {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to end session: {:?}", err);
            call_context.set_error(err.into());
            1
        }
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_destroy(session_options: *mut RustAmqpSessionOptions) {
    unsafe {
        mem::drop(Box::from_raw(session_options));
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_create() -> *mut RustAmqpSessionOptionsBuilder {
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: AmqpSessionOptions::builder(),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_destroy(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
) {
    unsafe {
        mem::drop(Box::from_raw(session_options_builder));
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_outgoing_window(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    outgoing_window: u32,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: session_options_builder
            .inner
            .with_outgoing_window(outgoing_window),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_incoming_window(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    incoming_window: u32,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: session_options_builder
            .inner
            .with_incoming_window(incoming_window),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_next_outgoing_id(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    next_outgoing_id: u32,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: session_options_builder
            .inner
            .with_next_outgoing_id(next_outgoing_id),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_handle_max(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    handle_max: u32,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: session_options_builder.inner.with_handle_max(handle_max),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_offered_capabilities(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    offered_capabilities: *mut RustAmqpValue,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    let offered_capabilities = unsafe { &*offered_capabilities };
    if let AmqpValue::List(offered_capabilities) = &offered_capabilities.inner {
        let offered_capabilities: Vec<AmqpSymbol> = offered_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
            inner: session_options_builder
                .inner
                .with_offered_capabilities(offered_capabilities),
        }))
    } else {
        ptr::null_mut()
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_desired_capabilities(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    desired_capabilities: *mut RustAmqpValue,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    let desired_capabilities = unsafe { &*desired_capabilities };
    if let AmqpValue::List(desired_capabilities) = &desired_capabilities.inner {
        let desired_capabilities: Vec<AmqpSymbol> = desired_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
            inner: session_options_builder
                .inner
                .with_desired_capabilities(desired_capabilities),
        }))
    } else {
        ptr::null_mut()
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_properties(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    properties: *mut RustAmqpValue,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    let properties = unsafe { &*properties };
    if let AmqpValue::Map(properties) = &properties.inner {
        let properties_map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
            .iter()
            .map(|(k, v)| (AmqpSymbol::from(k), v))
            .collect();
        Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
            inner: session_options_builder
                .inner
                .with_properties(properties_map),
        }))
    } else {
        ptr::null_mut()
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_set_buffer_size(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    buffer_size: usize,
) -> *mut RustAmqpSessionOptionsBuilder {
    let session_options_builder = Box::from_raw(session_options_builder);
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: session_options_builder.inner.with_buffer_size(buffer_size),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptionsbuilder_build(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
) -> *mut RustAmqpSessionOptions {
    let session_options_builder = unsafe { &mut *session_options_builder };
    Box::into_raw(Box::new(RustAmqpSessionOptions {
        inner: session_options_builder.inner.build(),
    }))
}

#[cfg(test)]
mod tests {
    use crate::runtime_context::RuntimeContext;
    use azure_core_amqp::{
        connection::{AmqpConnection, AmqpConnectionApis},
        value::AmqpList,
    };

    use super::*;

    #[test]
    fn test_amqpsession_create() {
        let session = unsafe { amqpsession_create() };
        assert_ne!(session, std::ptr::null_mut());
        unsafe { amqpsession_destroy(session) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_create() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        assert_ne!(session_options_builder, std::ptr::null_mut());
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_outgoing_window() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        unsafe { amqpsessionoptionsbuilder_set_outgoing_window(session_options_builder, 10) };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_incoming_window() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        unsafe { amqpsessionoptionsbuilder_set_incoming_window(session_options_builder, 10) };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_next_outgoing_id() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        unsafe { amqpsessionoptionsbuilder_set_next_outgoing_id(session_options_builder, 10) };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_handle_max() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        unsafe { amqpsessionoptionsbuilder_set_handle_max(session_options_builder, 10) };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_offered_capabilities() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        let offered_capabilities = RustAmqpValue {
            inner: AmqpValue::List(AmqpList::from(vec![AmqpValue::Symbol(AmqpSymbol::from(
                "test",
            ))])),
        };
        unsafe {
            amqpsessionoptionsbuilder_set_offered_capabilities(
                session_options_builder,
                &offered_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
            )
        };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_desired_capabilities() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        let desired_capabilities = RustAmqpValue {
            inner: AmqpValue::List(vec![AmqpValue::Symbol(AmqpSymbol::from("test"))].into()),
        };
        unsafe {
            amqpsessionoptionsbuilder_set_desired_capabilities(
                session_options_builder,
                &desired_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
            )
        };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_properties() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        let properties = RustAmqpValue {
            inner: AmqpValue::Map(
                vec![(
                    AmqpValue::Symbol("key".into()),
                    AmqpValue::Symbol("value".into()),
                )]
                .into_iter()
                .collect(),
            ),
        };
        unsafe {
            amqpsessionoptionsbuilder_set_properties(
                session_options_builder,
                &properties as *const RustAmqpValue as *mut RustAmqpValue,
            )
        };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_buffer_size() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        unsafe { amqpsessionoptionsbuilder_set_buffer_size(session_options_builder, 1024) };
        unsafe { amqpsessionoptionsbuilder_destroy(session_options_builder) };
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_build() {
        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        let session_options = unsafe { amqpsessionoptionsbuilder_build(session_options_builder) };
        assert_ne!(session_options, std::ptr::null_mut());
        unsafe { amqpsessionoptions_destroy(session_options) };
    }

    #[test]
    fn test_amqpsession_begin() {
        let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
        let session = unsafe { amqpsession_create() };
        let connection = AmqpConnection::new();

        call_context_from_ptr_mut(call_context)
            .runtime_context()
            .runtime()
            .block_on(connection.open(
                "testConnection",
                url::Url::parse("amqp://localhost:25672").unwrap(),
                None,
            ))
            .unwrap();

        let connection = Box::into_raw(Box::new(RustAmqpConnection::new(connection)));
        let result =
            unsafe { amqpsession_begin(call_context, session, connection, std::ptr::null_mut()) };
        assert_eq!(result, 0);
        unsafe {
            drop(Box::from_raw(connection));
            amqpsession_destroy(session);
            drop(Box::from_raw(runtime_context));
        }
    }

    #[test]
    fn test_amqpsession_begin_with_options() {
        let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
        let session = unsafe { amqpsession_create() };
        let connection = AmqpConnection::new();

        call_context_from_ptr_mut(call_context)
            .runtime_context()
            .runtime()
            .block_on(connection.open(
                "testConnection",
                url::Url::parse("amqp://localhost:25672").unwrap(),
                None,
            ))
            .unwrap();

        let connection = Box::into_raw(Box::new(RustAmqpConnection::new(connection)));

        let session_options_builder = unsafe { amqpsessionoptionsbuilder_create() };
        let session_options = unsafe { amqpsessionoptionsbuilder_build(session_options_builder) };
        let result =
            unsafe { amqpsession_begin(call_context, session, connection, session_options) };
        assert_eq!(result, 0);
        unsafe { amqpsession_destroy(session) };
        unsafe {
            drop(Box::from_raw(connection));
            drop(Box::from_raw(runtime_context));
        }
        unsafe { amqpsessionoptions_destroy(session_options) };
    }
}
