// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionoptionsbuilder amqpconnectionoptions amqpsession amqpsessionoptions amqpsessionoptions

use crate::{
    amqp::connection::RustAmqpConnection,
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str,
    model::value::RustAmqpValue,
};
use azure_core_amqp::{
    session::{AmqpSession, AmqpSessionApis, AmqpSessionOptions},
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};
use std::mem;
use tracing::error;

pub struct RustAmqpSession {
    inner: AmqpSession,
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
    mem::drop(Box::from_raw(session));
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
    let session = &mut *session;
    let connection = &mut *connection;
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
        let session_options = &*session_options;
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
    let session = &*session;
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
    mem::drop(Box::from_raw(session_options));
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_create() -> *mut RustAmqpSessionOptions {
    Box::into_raw(Box::new(RustAmqpSessionOptions {
        inner: AmqpSessionOptions::default(),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_outgoing_window(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    outgoing_window: u32,
) -> i32 {
    if session_options.is_null() {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let session_options = &mut *session_options;
    session_options.inner.outgoing_window = Some(outgoing_window);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_incoming_window(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    incoming_window: u32,
) -> i32 {
    if session_options.is_null() {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let session_options = &mut *session_options;
    session_options.inner.incoming_window = Some(incoming_window);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_next_outgoing_id(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    next_outgoing_id: u32,
) -> i32 {
    if session_options.is_null() {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let session_options = &mut *session_options;
    session_options.inner.next_outgoing_id = Some(next_outgoing_id);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_handle_max(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    handle_max: u32,
) -> i32 {
    if session_options.is_null() {
        let call_context = call_context_from_ptr_mut(call_context);
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let session_options = &mut *session_options;
    session_options.inner.handle_max = Some(handle_max);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_offered_capabilities(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    offered_capabilities: *mut RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    if session_options.is_null() {
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }

    let offered_capabilities = &*offered_capabilities;
    if let AmqpValue::List(offered_capabilities) = &offered_capabilities.inner {
        let offered_capabilities: Vec<AmqpSymbol> = offered_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        let session_options = &mut *session_options;
        session_options.inner.offered_capabilities = Some(offered_capabilities);
        0
    } else {
        call_context.set_error(error_from_str("Offered  Capabilities must be an AMQP list"));
        -1
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_desired_capabilities(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    desired_capabilities: *mut RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    if session_options.is_null() {
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let desired_capabilities = &*desired_capabilities;
    if let AmqpValue::List(desired_capabilities) = &desired_capabilities.inner {
        let desired_capabilities: Vec<AmqpSymbol> = desired_capabilities
            .iter()
            .map(|v| AmqpSymbol::from(v.clone()))
            .collect();
        let session_options = &mut *session_options;
        session_options.inner.desired_capabilities = Some(desired_capabilities);
        0
    } else {
        call_context.set_error(error_from_str("Desired Capabilities must be an AMQP list"));
        -1
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_properties(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    properties: *mut RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    if session_options.is_null() {
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let properties = &*properties;
    if let AmqpValue::Map(properties) = &properties.inner {
        let properties_map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
            .iter()
            .map(|(k, v)| (AmqpSymbol::from(k), v))
            .collect();
        let session_options = &mut *session_options;
        session_options.inner.properties = Some(properties_map);
        0
    } else {
        call_context.set_error(error_from_str("Properties must be an AMQP map"));
        -1
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpsessionoptions_set_buffer_size(
    call_context: *mut RustCallContext,
    session_options: *mut RustAmqpSessionOptions,
    buffer_size: usize,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    if session_options.is_null() {
        call_context.set_error(error_from_str("Session options builder is null"));
        return -1;
    }
    let session_options = &mut *session_options;
    session_options.inner.buffer_size = Some(buffer_size);
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::call_context::call_context_delete;
    use crate::runtime_context::RuntimeContext;
    use azure_core_amqp::{
        connection::{AmqpConnection, AmqpConnectionApis},
        value::AmqpList,
    };

    #[test]
    fn test_amqpsession_create() {
        unsafe {
            let session = { amqpsession_create() };
            assert_ne!(session, std::ptr::null_mut());

            amqpsession_destroy(session);
        }
    }

    #[test]
    fn test_amqpsessionoptions_create() {
        unsafe {
            let session_options = { amqpsessionoptions_create() };
            assert_ne!(session_options, std::ptr::null_mut());

            amqpsessionoptions_destroy(session_options);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_outgoing_window() {
        unsafe {
            let session_options = { amqpsessionoptions_create() };
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            amqpsessionoptions_set_outgoing_window(call_context, session_options, 10);

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_incoming_window() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let session_options = { amqpsessionoptions_create() };

            amqpsessionoptions_set_incoming_window(call_context, session_options, 10);

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_next_outgoing_id() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let session_options = { amqpsessionoptions_create() };

            amqpsessionoptions_set_next_outgoing_id(call_context, session_options, 10);

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_handle_max() {
        unsafe {
            let session_options = { amqpsessionoptions_create() };
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            amqpsessionoptions_set_handle_max(call_context, session_options, 10);

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_offered_capabilities() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let session_options = { amqpsessionoptions_create() };
            let offered_capabilities = RustAmqpValue {
                inner: AmqpValue::List(AmqpList::from(vec![AmqpValue::Symbol(AmqpSymbol::from(
                    "test",
                ))])),
            };

            amqpsessionoptions_set_offered_capabilities(
                call_context,
                session_options,
                &offered_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
            );

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_desired_capabilities() {
        unsafe {
            let session_options = { amqpsessionoptions_create() };
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let desired_capabilities = RustAmqpValue {
                inner: AmqpValue::List(vec![AmqpValue::Symbol(AmqpSymbol::from("test"))].into()),
            };

            amqpsessionoptions_set_desired_capabilities(
                call_context,
                session_options,
                &desired_capabilities as *const RustAmqpValue as *mut RustAmqpValue,
            );

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_properties() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let session_options = { amqpsessionoptions_create() };
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

            amqpsessionoptions_set_properties(
                call_context,
                session_options,
                &properties as *const RustAmqpValue as *mut RustAmqpValue,
            );

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsessionoptions_set_buffer_size() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let session_options = { amqpsessionoptions_create() };
            amqpsessionoptions_set_buffer_size(call_context, session_options, 1024);

            amqpsessionoptions_destroy(session_options);
            call_context_delete(call_context);
        }
    }

    #[test]
    fn test_amqpsession_begin() {
        unsafe {
            let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
            let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
            let session = amqpsession_create();
            let connection = AmqpConnection::new();

            call_context_from_ptr_mut(call_context)
                .runtime_context()
                .runtime()
                .block_on(connection.open(
                    "testConnection".to_string(),
                    url::Url::parse("amqp://localhost:25672").unwrap(),
                    None,
                ))
                .unwrap();

            let connection = Box::into_raw(Box::new(RustAmqpConnection::new(connection)));
            let result = amqpsession_begin(call_context, session, connection, std::ptr::null_mut());
            assert_eq!(result, 0);
            drop(Box::from_raw(connection));
            amqpsession_destroy(session);
            drop(Box::from_raw(runtime_context));
        }
    }

    #[test]
    fn test_amqpsession_begin_with_options() {
        unsafe {
            let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
            let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
            let session = amqpsession_create();
            let connection = AmqpConnection::new();

            call_context_from_ptr_mut(call_context)
                .runtime_context()
                .runtime()
                .block_on(connection.open(
                    "testConnection".to_string(),
                    url::Url::parse("amqp://localhost:25672").unwrap(),
                    None,
                ))
                .unwrap();

            let connection = Box::into_raw(Box::new(RustAmqpConnection::new(connection)));

            let session_options = amqpsessionoptions_create();
            let result = amqpsession_begin(call_context, session, connection, session_options);
            assert_eq!(result, 0);
            amqpsession_destroy(session);
            drop(Box::from_raw(connection));
            drop(Box::from_raw(runtime_context));
            amqpsessionoptions_destroy(session_options);
        }
    }
}
