// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionoptionsbuilder amqpconnectionoptions amqpsession amqpsessionoptions amqpsessionoptionsbuilder

use std::mem;

use crate::{
    amqp::connection::RustAmqpConnection,
    model::value::RustAmqpValue,
    runtime_context::{runtime_context_from_ptr_mut, RuntimeContext},
};

use azure_core_amqp::{
    session::{
        builders::AmqpSessionOptionsBuilder, AmqpSession, AmqpSessionApis, AmqpSessionOptions,
    },
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};

pub struct RustAmqpSession {
    inner: AmqpSession,
}

pub struct RustAmqpSessionOptionsBuilder {
    inner: AmqpSessionOptionsBuilder,
}

pub struct RustAmqpSessionOptions {
    inner: AmqpSessionOptions,
}

#[no_mangle]
pub extern "C" fn amqpsession_create() -> *mut RustAmqpSession {
    Box::into_raw(Box::new(RustAmqpSession {
        inner: AmqpSession::new(),
    }))
}

#[no_mangle]
pub extern "C" fn amqpsession_destroy(session: *mut RustAmqpSession) {
    unsafe {
        mem::drop(Box::from_raw(session));
    }
}

#[no_mangle]
pub extern "C" fn amqpsession_begin(
    session: *mut RustAmqpSession,
    connection: *mut RustAmqpConnection,
    session_options: *mut RustAmqpSessionOptions,
    runtime_context: *mut RuntimeContext,
) -> *mut std::ffi::c_void {
    let session = unsafe { &mut *session };
    let connection = unsafe { &mut *connection };
    let runtime_context = runtime_context_from_ptr_mut(runtime_context);

    if session_options.is_null() {
        let result = runtime_context
            .runtime()
            .block_on(session.inner.begin(connection.get_connection(), None));
        match result {
            Ok(()) => return std::ptr::null_mut(),
            Err(err) => return Box::into_raw(Box::new(err)) as *mut std::ffi::c_void,
        }
    } else {
        let session_options = unsafe { &*session_options };
        let result = runtime_context.runtime().block_on(session.inner.begin(
            connection.get_connection(),
            Some(session_options.inner.clone()),
        ));
        match result {
            Ok(()) => return std::ptr::null_mut(),
            Err(err) => return Box::into_raw(Box::new(err)) as *mut std::ffi::c_void,
        }
    }
}

#[no_mangle]
pub extern "C" fn amqpsessionoptions_destroy(session_options: *mut RustAmqpSessionOptions) {
    unsafe {
        mem::drop(Box::from_raw(session_options));
    }
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_create() -> *mut RustAmqpSessionOptionsBuilder {
    Box::into_raw(Box::new(RustAmqpSessionOptionsBuilder {
        inner: AmqpSessionOptions::builder(),
    }))
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_destroy(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
) {
    unsafe {
        mem::drop(Box::from_raw(session_options_builder));
    }
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_outgoing_window(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    outgoing_window: u32,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    session_options_builder
        .inner
        .with_outgoing_window(outgoing_window);
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_incoming_window(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    incoming_window: u32,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    session_options_builder
        .inner
        .with_incoming_window(incoming_window);
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_next_outgoing_id(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    next_outgoing_id: u32,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    session_options_builder
        .inner
        .with_next_outgoing_id(next_outgoing_id);
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_handle_max(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    handle_max: u32,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    session_options_builder.inner.with_handle_max(handle_max);
}
#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_offered_capabilities(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    offered_capabilities: *mut RustAmqpValue,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    let offered_capabilities = unsafe { &*offered_capabilities };
    if let AmqpValue::List(offered_capabilities) = &offered_capabilities.inner {
        let offered_capabilities: Vec<AmqpSymbol> = offered_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        session_options_builder
            .inner
            .with_offered_capabilities(offered_capabilities);
    }
}
#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_desired_capabilities(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    desired_capabilities: *mut RustAmqpValue,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    let desired_capabilities = unsafe { &*desired_capabilities };
    if let AmqpValue::List(desired_capabilities) = &desired_capabilities.inner {
        let desired_capabilities: Vec<AmqpSymbol> = desired_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        session_options_builder
            .inner
            .with_desired_capabilities(desired_capabilities);
    }
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_properties(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    properties: *mut RustAmqpValue,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    let properties = unsafe { &*properties };
    if let AmqpValue::Map(properties) = &properties.inner {
        let properties_map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
            .iter()
            .map(|(k, v)| (AmqpSymbol::from(k), AmqpValue::from(v)))
            .collect();
        session_options_builder
            .inner
            .with_properties(properties_map);
    }
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_set_buffer_size(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
    buffer_size: usize,
) {
    let session_options_builder = unsafe { &mut *session_options_builder };
    session_options_builder.inner.with_buffer_size(buffer_size);
}

#[no_mangle]
pub extern "C" fn amqpsessionoptionsbuilder_build(
    session_options_builder: *mut RustAmqpSessionOptionsBuilder,
) -> *mut RustAmqpSessionOptions {
    let session_options_builder = unsafe { &mut *session_options_builder };
    Box::into_raw(Box::new(RustAmqpSessionOptions {
        inner: session_options_builder.inner.build(),
    }))
}

#[cfg(test)]
mod tests {
    use azure_core_amqp::value::AmqpList;

    use super::*;

    #[test]
    fn test_amqpsession_create() {
        let session = amqpsession_create();
        assert_ne!(session, std::ptr::null_mut());
        amqpsession_destroy(session);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_create() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        assert_ne!(session_options_builder, std::ptr::null_mut());
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_outgoing_window() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        amqpsessionoptionsbuilder_set_outgoing_window(session_options_builder, 10);
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_incoming_window() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        amqpsessionoptionsbuilder_set_incoming_window(session_options_builder, 10);
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_next_outgoing_id() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        amqpsessionoptionsbuilder_set_next_outgoing_id(session_options_builder, 10);
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_handle_max() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        amqpsessionoptionsbuilder_set_handle_max(session_options_builder, 10);
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_offered_capabilities() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        let offered_capabilities = RustAmqpValue {
            inner: AmqpValue::List(AmqpList::from(vec![AmqpValue::Symbol(AmqpSymbol::from(
                "test",
            ))])),
        };
        amqpsessionoptionsbuilder_set_offered_capabilities(
            session_options_builder,
            &offered_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
        );
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_desired_capabilities() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        let desired_capabilities = RustAmqpValue {
            inner: AmqpValue::List(vec![AmqpValue::Symbol(AmqpSymbol::from("test"))].into()),
        };
        amqpsessionoptionsbuilder_set_desired_capabilities(
            session_options_builder,
            &desired_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
        );
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_properties() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
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
        amqpsessionoptionsbuilder_set_properties(
            session_options_builder,
            &properties as *const RustAmqpValue as *mut RustAmqpValue,
        );
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_set_buffer_size() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        amqpsessionoptionsbuilder_set_buffer_size(session_options_builder, 1024);
        amqpsessionoptionsbuilder_destroy(session_options_builder);
    }

    #[test]
    fn test_amqpsessionoptionsbuilder_build() {
        let session_options_builder = amqpsessionoptionsbuilder_create();
        let session_options = amqpsessionoptionsbuilder_build(session_options_builder);
        assert_ne!(session_options, std::ptr::null_mut());
        amqpsessionoptions_destroy(session_options);
    }

    #[test]
    fn test_amqpsession_begin() {
        let session = amqpsession_create();
        let connection = Box::into_raw(Box::new(RustAmqpConnection::new()));
        let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        let result = amqpsession_begin(session, connection, std::ptr::null_mut(), runtime_context);
        assert_eq!(result, std::ptr::null_mut());
        unsafe {
            amqpsession_destroy(session);
            Box::from_raw(connection);
            Box::from_raw(runtime_context);
        }
    }

    #[test]
    fn test_amqpsession_begin_with_options() {
        let session = amqpsession_create();
        let connection = Box::into_raw(Box::new(RustAmqpConnection::new()));
        let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        let session_options_builder = amqpsessionoptionsbuilder_create();
        let session_options = amqpsessionoptionsbuilder_build(session_options_builder);
        let result = amqpsession_begin(session, connection, session_options, runtime_context);
        assert_eq!(result, std::ptr::null_mut());
        unsafe {
            amqpsession_destroy(session);
            Box::from_raw(connection);
            Box::from_raw(runtime_context);
            amqpsessionoptions_destroy(session_options);
        }
    }
}
