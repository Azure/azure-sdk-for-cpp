// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionoptionsbuilder amqpconnectionoptions

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_string,
    model::value::RustAmqpValue,
};
use core::panic;
use std::{
    ffi::{c_char, CStr},
    mem,
    ptr::{self},
};
use time::Duration;
use tracing::error;
use url::Url;

use azure_core_amqp::{
    connection::{AmqpConnection, AmqpConnectionApis, AmqpConnectionOptions},
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};

pub struct RustAmqpConnection {
    inner: AmqpConnection,
}

impl RustAmqpConnection {
    pub fn new(connection: AmqpConnection) -> RustAmqpConnection {
        Self { inner: connection }
    }
    pub(crate) fn get_connection(&self) -> &AmqpConnection {
        &self.inner
    }
}

pub struct RustAmqpConnectionOptions {
    inner: AmqpConnectionOptions,
}

#[no_mangle]
pub extern "C" fn amqpconnection_create() -> *mut RustAmqpConnection {
    Box::into_raw(Box::new(RustAmqpConnection {
        inner: AmqpConnection::new(),
    }))
}

/// # Safety
/// This function is unsafe because it dereferences the `connection` pointer.
/// The caller must guarantee that the pointer is valid.
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnection_destroy(connection: *mut RustAmqpConnection) {
    mem::drop(Box::from_raw(connection));
}

/// # Safety
/// This function is unsafe because it dereferences the `connection` pointer.
/// The caller must guarantee that the pointer is valid.
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnection_open(
    ctx: *mut RustCallContext,
    connection: *const RustAmqpConnection,
    url: *const c_char,
    container_id: *const c_char,
    options: *const RustAmqpConnectionOptions,
) -> i32 {
    let call_context = call_context_from_ptr_mut(ctx);
    let connection = &*connection;
    let url = CStr::from_ptr(url);
    let url = url.to_str();
    let container_id = CStr::from_ptr(container_id);
    let default_options: RustAmqpConnectionOptions = RustAmqpConnectionOptions {
        inner: Default::default(),
    };
    let options = if !options.is_null() {
        &*options
    } else {
        &default_options
    };

    if url.is_err() {
        error!("Failed to convert URL to string: {:?}", url.err());
        call_context.set_error(url.err().unwrap().into());
        return 1;
    }
    let url = url.unwrap();

    let url = Url::parse(url);
    if url.is_err() {
        let err = url.err().unwrap();
        error!("Failed to parse URL: {:?}", &err);
        call_context.set_error(err.into());
        return 1;
    }
    let url = url.unwrap();

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(connection.inner.open(
            container_id.to_str().unwrap().to_string(),
            url,
            Some(options.inner.clone()),
        ));
    match result {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to open connection: {:?}", err);
            call_context.set_error(err.into());
            1
        }
    }
}

/// # Safety
/// This function is unsafe because it dereferences the `connection` pointer.
/// The caller must guarantee that the pointer is valid.
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnection_close(
    ctx: *mut RustCallContext,
    connection: *const RustAmqpConnection,
) -> i32 {
    let connection = &*connection;
    let runtime_context = call_context_from_ptr_mut(ctx);
    let result = runtime_context
        .runtime_context()
        .runtime()
        .block_on(connection.inner.close());
    match result {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to close connection: {:?}", err);
            runtime_context.set_error(err.into());
            1
        }
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnection_close_with_error(
    ctx: *mut RustCallContext,
    connection: *const RustAmqpConnection,
    condition: *const c_char,
    description: *const c_char,
    info: *const RustAmqpValue,
) -> i32 {
    let connection = &*connection;
    let call_context = call_context_from_ptr_mut(ctx);
    let condition = CStr::from_ptr(condition);
    let description = CStr::from_ptr(description);
    let info = &*info;
    if let AmqpValue::Map(info) = &info.inner {
        let info: AmqpOrderedMap<AmqpSymbol, AmqpValue> = info
            .clone()
            .into_iter()
            .map(|f| {
                (
                    match f.0 {
                        AmqpValue::Symbol(s) => s,
                        _ => panic!("Expected symbol"),
                    },
                    f.1,
                )
            })
            .collect();
        let result =
            call_context
                .runtime_context()
                .runtime()
                .block_on(connection.inner.close_with_error(
                    condition.to_str().unwrap().into(),
                    Some(description.to_str().unwrap().into()),
                    Some(info.clone()),
                ));
        match result {
            Ok(_) => 0,
            Err(err) => {
                error!("Failed to close connection with error: {:?}", err);
                call_context.set_error(err.into());
                1
            }
        }
    } else {
        1
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_get_idle_timeout(
    options: *const RustAmqpConnectionOptions,
) -> u32 {
    let options = &*options;
    if let Some(timeout) = options.inner.idle_timeout {
        return timeout.whole_milliseconds() as u32;
    }
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_get_max_frame_size(
    options: *const RustAmqpConnectionOptions,
) -> u32 {
    let options = &*options;
    if let Some(max_frame_size) = options.inner.max_frame_size {
        return max_frame_size;
    }
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_get_channel_max(
    options: *const RustAmqpConnectionOptions,
) -> u16 {
    let options = &*options;
    if let Some(channel_max) = options.inner.channel_max {
        return channel_max;
    }
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_get_properties(
    options: *const RustAmqpConnectionOptions,
) -> *mut RustAmqpValue {
    let options = &*options;
    let properties = &options.inner.properties;
    if let Some(properties) = properties {
        let properties: AmqpOrderedMap<AmqpValue, AmqpValue> = properties
            .iter()
            .map(|f| (AmqpValue::Symbol(f.0.clone()), f.1.clone()))
            .collect();
        Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Map(properties),
        }))
    } else {
        ptr::null_mut()
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_create() -> *mut RustAmqpConnectionOptions {
    Box::into_raw(Box::new(RustAmqpConnectionOptions {
        inner: AmqpConnectionOptions::default(),
    }))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_destroy(options: *mut RustAmqpConnectionOptions) {
    mem::drop(Box::from_raw(options));
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_idle_timeout(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    idle_timeout: u32,
) -> i32 {
    let options = &mut *options;
    options.inner.idle_timeout = Some(Duration::milliseconds(idle_timeout as i64));
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_max_frame_size(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    max_frame_size: u32,
) -> i32 {
    let options = &mut *options;
    options.inner.max_frame_size = Some(max_frame_size);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_channel_max(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    channel_max: u16,
) -> i32 {
    let options = &mut *options;
    options.inner.channel_max = Some(channel_max);
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_outgoing_locales(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    locales: *const *const c_char,
    count: usize,
) -> i32 {
    let options = &mut *options;
    let locales = std::slice::from_raw_parts(locales, count);
    let locales = locales
        .iter()
        .map(|locale| CStr::from_ptr(*locale).to_str().unwrap())
        .collect::<Vec<&str>>();
    options.inner.outgoing_locales = Some(locales.into_iter().map(String::from).collect());
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_incoming_locales(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    locales: *const *const c_char,
    count: usize,
) -> i32 {
    let options = &mut *options;
    let locales = std::slice::from_raw_parts(locales, count);
    let locales = locales
        .iter()
        .map(|locale| CStr::from_ptr(*locale).to_str().unwrap())
        .collect::<Vec<&str>>();
    options.inner.incoming_locales = Some(locales.into_iter().map(String::from).collect());
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_offered_capabilities(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    capabilities: *const *const c_char,
    count: usize,
) -> i32 {
    let options = &mut *options;
    let capabilities = std::slice::from_raw_parts(capabilities, count);
    let capabilities = capabilities
        .iter()
        .map(|capability| CStr::from_ptr(*capability).to_str().unwrap())
        .collect::<Vec<&str>>();
    options.inner.offered_capabilities = Some(
        capabilities
            .into_iter()
            .map(|capability| AmqpSymbol::from(capability))
            .collect(),
    );
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_desired_capabilities(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    capabilities: *const *const c_char,
    count: usize,
) -> i32 {
    let options = &mut *options;
    let capabilities = std::slice::from_raw_parts(capabilities, count);
    let capabilities = capabilities
        .iter()
        .map(|capability| CStr::from_ptr(*capability).to_str().unwrap())
        .collect::<Vec<&str>>();
    options.inner.desired_capabilities = Some(
        capabilities
            .into_iter()
            .map(|capability| AmqpSymbol::from(capability))
            .collect(),
    );
    0
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_properties(
    call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    properties: *const RustAmqpValue,
) -> i32 {
    let options = &mut *options;
    let properties = &*properties;
    match &properties.inner {
        AmqpValue::Map(properties) => {
            let properties: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
                .clone()
                .into_iter()
                .map(|f| {
                    (
                        match f.0 {
                            AmqpValue::Symbol(s) => s,
                            _ => panic!("Expected symbol for value, found: {:?}", f.0),
                        },
                        f.1,
                    )
                })
                .collect();
            options.inner.properties = Some(properties);
            0
        }
        _ => {
            call_context_from_ptr_mut(call_context).set_error(error_from_string(format!(
                "Expected map for value, found: {:?}",
                properties.inner
            )));
            0
        }
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpconnectionoptions_set_buffer_size(
    _call_context: *mut RustCallContext,
    options: *mut RustAmqpConnectionOptions,
    buffer_size: usize,
) -> i32 {
    let options = &mut *options;
    options.inner.buffer_size = Some(buffer_size);
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::runtime_context::RuntimeContext;
    use std::ffi::CString;
    use std::ptr;

    fn create_runtime_context() -> *mut RuntimeContext {
        // Mock implementation for creating a runtime context
        Box::into_raw(Box::new(RuntimeContext::new().unwrap()))
    }
    fn create_call_context(runtime_context: *mut RuntimeContext) -> *mut RustCallContext {
        // Mock implementation for creating a runtime context
        Box::into_raw(Box::new(RustCallContext::new(runtime_context)))
    }

    #[test]
    fn test_amqpconnection_create_and_destroy() {
        let connection = amqpconnection_create();
        assert!(!connection.is_null());
        unsafe {
            amqpconnection_destroy(connection);
        }
    }

    #[test]
    fn test_amqpconnection_open_and_close() {
        unsafe {
            let ctx = create_runtime_context();
            let ctx = create_call_context(ctx);
            let connection = amqpconnection_create();
            let url = CString::new("amqp://localhost:25672").unwrap();
            let container_id = CString::new("test_container").unwrap();
            let options = ptr::null();

            let open_result = amqpconnection_open(
                ctx,
                connection,
                url.as_ptr(),
                container_id.as_ptr(),
                options,
            );
            assert_eq!(open_result, 0);

            let close_result = amqpconnection_close(ctx, connection);
            assert_eq!(close_result, 0);

            amqpconnection_destroy(connection);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_create_and_destroy() {
        unsafe {
            let options = amqpconnectionoptions_create();
            assert!(!options.is_null());
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_idle_timeout() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));

            let options = amqpconnectionoptions_create();
            let result = amqpconnectionoptions_set_idle_timeout(call_context, options, 30);
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptions_set_max_frame_size() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = amqpconnectionoptions_create();
            let result = { amqpconnectionoptions_set_max_frame_size(call_context, options, 65536) };
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptions_set_channel_max() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = { amqpconnectionoptions_create() };
            let result = { amqpconnectionoptions_set_channel_max(call_context, options, 256) };
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_outgoing_locales() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = { amqpconnectionoptions_create() };
            let locales = [
                CString::new("en-US").unwrap(),
                CString::new("fr-FR").unwrap(),
            ];
            let locale_ptrs: Vec<*const c_char> =
                locales.iter().map(|locale| locale.as_ptr()).collect();
            let result = {
                amqpconnectionoptions_set_outgoing_locales(
                    call_context,
                    options,
                    locale_ptrs.as_ptr(),
                    locale_ptrs.len(),
                )
            };
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptions_set_incoming_locales() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = { amqpconnectionoptions_create() };
            let locales = [
                CString::new("en-US").unwrap(),
                CString::new("fr-FR").unwrap(),
            ];
            let locale_ptrs: Vec<*const c_char> =
                locales.iter().map(|locale| locale.as_ptr()).collect();
            let result = {
                amqpconnectionoptions_set_incoming_locales(
                    call_context,
                    options,
                    locale_ptrs.as_ptr(),
                    locale_ptrs.len(),
                )
            };
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_offered_capabilities() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = { amqpconnectionoptions_create() };
            let capabilities = [
                CString::new("capability1").unwrap(),
                CString::new("capability2").unwrap(),
            ];
            let capability_ptrs: Vec<*const c_char> =
                capabilities.iter().map(|cap| cap.as_ptr()).collect();
            let result = {
                amqpconnectionoptions_set_offered_capabilities(
                    call_context,
                    options,
                    capability_ptrs.as_ptr(),
                    capability_ptrs.len(),
                )
            };
            assert_eq!(result, 0);

            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_desired_capabilities() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = { amqpconnectionoptions_create() };
            let capabilities = [
                CString::new("capability1").unwrap(),
                CString::new("capability2").unwrap(),
            ];
            let capability_ptrs: Vec<*const c_char> =
                capabilities.iter().map(|cap| cap.as_ptr()).collect();
            let result = amqpconnectionoptions_set_desired_capabilities(
                call_context,
                options,
                capability_ptrs.as_ptr(),
                capability_ptrs.len(),
            );
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_properties() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = amqpconnectionoptions_create();
            let mut map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = AmqpOrderedMap::new();
            map.insert(AmqpSymbol::from("key1"), AmqpValue::from("value1"));
            map.insert(AmqpSymbol::from("key2"), AmqpValue::from("value2"));

            let rust_value = RustAmqpValue {
                inner: AmqpValue::Map(map.into_iter().map(|f| (f.0.into(), f.1)).collect()),
            };

            let result = amqpconnectionoptions_set_properties(
                call_context,
                options,
                &rust_value as *const RustAmqpValue,
            );
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_buffer_size() {
        unsafe {
            let call_context = Box::into_raw(Box::new(RustCallContext::new(Box::into_raw(
                Box::new(RuntimeContext::new().unwrap()),
            ))));
            let options = amqpconnectionoptions_create();
            let result = amqpconnectionoptions_set_buffer_size(call_context, options, 1024);
            assert_eq!(result, 0);
            amqpconnectionoptions_destroy(options);
        }
    }
}
