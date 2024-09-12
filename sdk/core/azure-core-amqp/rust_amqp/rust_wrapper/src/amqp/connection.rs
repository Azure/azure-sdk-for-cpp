// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionoptionsbuilder amqpconnectionoptions

use core::panic;
use std::{
    ffi::{c_char, CStr},
    mem,
    ptr::{self, null},
};
use time::Duration;

use crate::{
    model::value::RustAmqpValue,
    runtime_context::{runtime_context_from_ptr_mut, RuntimeContext},
};

use tracing::error;
use url::Url;

use azure_core_amqp::{
    connection::{
        builders::AmqpConnectionOptionsBuilder, AmqpConnection, AmqpConnectionApis,
        AmqpConnectionOptions,
    },
    value::{AmqpOrderedMap, AmqpSymbol, AmqpValue},
};

pub struct RustAmqpConnection {
    inner: AmqpConnection,
}

pub struct RustAmqpConnectionOptionsBuilder {
    inner: AmqpConnectionOptionsBuilder,
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

#[no_mangle]
pub extern "C" fn amqpconnection_destroy(connection: *mut RustAmqpConnection) {
    unsafe {
        mem::drop(Box::from_raw(connection));
    }
}

#[no_mangle]
pub extern "C" fn amqpconnection_open(
    ctx: *mut RuntimeContext,
    connection: *const RustAmqpConnection,
    url: *const c_char,
    container_id: *const c_char,
    options: *const RustAmqpConnectionOptions,
) -> u32 {
    let runtime_context = runtime_context_from_ptr_mut(ctx);
    let connection = unsafe { &*connection };
    let url = unsafe { CStr::from_ptr(url) };
    let url = url.to_str();
    let container_id = unsafe { CStr::from_ptr(container_id) };
    let default_options: RustAmqpConnectionOptions = RustAmqpConnectionOptions {
        inner: Default::default(),
    };
    let options = if options != null() {
        unsafe { &*options }
    } else {
        &default_options
    };

    if url.is_err() {
        error!("Failed to convert URL to string: {:?}", url.err());
        runtime_context.set_error(url.err().unwrap().into());
        return 1;
    }
    let url = url.unwrap();

    let url = Url::parse(url);
    if url.is_err() {
        let err = url.err().unwrap();
        error!("Failed to parse URL: {:?}", &err);
        runtime_context.set_error(err.into());
        return 1;
    }
    let url = url.unwrap();

    let result = runtime_context.runtime().block_on(connection.inner.open(
        container_id.to_str().unwrap(),
        url,
        Some(options.inner.clone()),
    ));
    match result {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to open connection: {:?}", err);
            runtime_context.set_error(err.into());
            1
        }
    }
}

#[no_mangle]
pub extern "C" fn amqpconnection_close(
    ctx: *mut RuntimeContext,
    connection: *const RustAmqpConnection,
) -> u32 {
    let connection = unsafe { &*connection };
    let runtime_context = runtime_context_from_ptr_mut(ctx);
    let result = runtime_context.runtime().block_on(connection.inner.close());
    match result {
        Ok(_) => 0,
        Err(err) => {
            error!("Failed to close connection: {:?}", err);
            runtime_context.set_error(err.into());
            1
        }
    }
}

#[no_mangle]
pub extern "C" fn amqpconnection_close_with_error(
    ctx: *mut RuntimeContext,
    connection: *const RustAmqpConnection,
    condition: *const c_char,
    description: *const c_char,
    info: *const RustAmqpValue,
) -> u32 {
    let connection = unsafe { &*connection };
    let runtime_context = runtime_context_from_ptr_mut(ctx);
    let condition = unsafe { CStr::from_ptr(condition) };
    let description = unsafe { CStr::from_ptr(description) };
    let info = unsafe { &*info };
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
        let result = runtime_context
            .runtime()
            .block_on(connection.inner.close_with_error(
                condition.to_str().unwrap(),
                Some(description.to_str().unwrap().into()),
                Some(info.clone()),
            ));
        match result {
            Ok(_) => 0,
            Err(err) => {
                error!("Failed to close connection with error: {:?}", err);
                runtime_context.set_error(err.into());
                1
            }
        }
    } else {
        1
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptions_get_idle_timeout(
    options: *const RustAmqpConnectionOptions,
) -> u32 {
    let options = unsafe { &*options };
    if let Some(timeout) = options.inner.idle_timeout() {
        return timeout.whole_milliseconds() as u32;
    }
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptions_get_max_frame_size(
    options: *const RustAmqpConnectionOptions,
) -> u32 {
    let options = unsafe { &*options };
    if let Some(max_frame_size) = options.inner.max_frame_size() {
        return max_frame_size;
    }
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptions_get_channel_max(
    options: *const RustAmqpConnectionOptions,
) -> u16 {
    let options = unsafe { &*options };
    if let Some(channel_max) = options.inner.channel_max() {
        return channel_max;
    }
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptions_get_properties(
    options: *const RustAmqpConnectionOptions,
) -> *mut RustAmqpValue {
    let options = unsafe { &*options };
    let properties = options.inner.properties();
    if let Some(properties) = properties {
        let properties: AmqpOrderedMap<AmqpValue, AmqpValue> = properties
            .into_iter()
            .map(|f| (AmqpValue::Symbol(f.0.clone()), f.1.clone()))
            .collect();
        Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Map(properties),
        }))
    } else {
        ptr::null_mut()
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_create() -> *mut RustAmqpConnectionOptionsBuilder {
    let builder = AmqpConnectionOptions::builder();
    Box::into_raw(Box::new(RustAmqpConnectionOptionsBuilder {
        inner: builder,
    }))
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_destroy(
    builder: *mut RustAmqpConnectionOptionsBuilder,
) {
    unsafe {
        mem::drop(Box::from_raw(builder));
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_build(
    builder: *mut RustAmqpConnectionOptionsBuilder,
) -> *mut RustAmqpConnectionOptions {
    let builder = unsafe { &mut *builder };
    let options = builder.inner.build();
    Box::into_raw(Box::new(RustAmqpConnectionOptions { inner: options }))
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptions_destroy(options: *mut RustAmqpConnectionOptions) {
    unsafe {
        mem::drop(Box::from_raw(options));
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_idle_timeout(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    idle_timeout: u32,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder
        .inner
        .with_idle_timeout(Duration::milliseconds(idle_timeout as i64));
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_max_frame_size(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    max_frame_size: u32,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_max_frame_size(max_frame_size);
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_channel_max(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    channel_max: u16,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_channel_max(channel_max);
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_outgoing_locales(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    locales: *const *const c_char,
    count: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let locales = unsafe { std::slice::from_raw_parts(locales, count) };
    let locales = locales
        .iter()
        .map(|locale| unsafe { CStr::from_ptr(*locale).to_str().unwrap() })
        .collect::<Vec<&str>>();
    builder
        .inner
        .with_outgoing_locales(locales.into_iter().map(String::from).collect());
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_incoming_locales(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    locales: *const *const c_char,
    count: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let locales = unsafe { std::slice::from_raw_parts(locales, count) };
    let locales = locales
        .iter()
        .map(|locale| unsafe { CStr::from_ptr(*locale).to_str().unwrap() })
        .collect::<Vec<&str>>();
    builder
        .inner
        .with_incoming_locales(locales.into_iter().map(String::from).collect());
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_offered_capabilities(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    capabilities: *const *const c_char,
    count: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let capabilities = unsafe { std::slice::from_raw_parts(capabilities, count) };
    let capabilities = capabilities
        .iter()
        .map(|capability| unsafe { CStr::from_ptr(*capability).to_str().unwrap() })
        .collect::<Vec<&str>>();
    builder
        .inner
        .with_offered_capabilities(capabilities.into_iter().map(AmqpSymbol::from).collect());
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_desired_capabilities(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    capabilities: *const *const c_char,
    count: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let capabilities = unsafe { std::slice::from_raw_parts(capabilities, count) };
    let capabilities = capabilities
        .iter()
        .map(|capability| unsafe { CStr::from_ptr(*capability).to_str().unwrap() })
        .collect::<Vec<&str>>();
    builder
        .inner
        .with_desired_capabilities(capabilities.into_iter().map(AmqpSymbol::from).collect());
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_properties(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    properties: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let properties = unsafe { &*properties };
    match &properties.inner {
        AmqpValue::Map(properties) => {
            let properties: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
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
            builder.inner.with_properties(properties);
            0
        }
        _ => 1,
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionoptionsbuilder_set_buffer_size(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    buffer_size: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_buffer_size(buffer_size);
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;
    use std::ptr;

    fn create_runtime_context() -> *mut RuntimeContext {
        // Mock implementation for creating a runtime context
        Box::into_raw(Box::new(RuntimeContext::new().unwrap()))
    }

    #[test]
    fn test_amqpconnection_create_and_destroy() {
        let connection = amqpconnection_create();
        assert!(!connection.is_null());
        amqpconnection_destroy(connection);
    }

    #[test]
    fn test_amqpconnection_open_and_close() {
        let ctx = create_runtime_context();
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

    #[test]
    fn test_amqpconnectionoptionsbuilder_create_and_destroy() {
        let builder = amqpconnectionoptionsbuilder_create();
        assert!(!builder.is_null());
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_idle_timeout() {
        let builder = amqpconnectionoptionsbuilder_create();
        let result = amqpconnectionoptionsbuilder_set_idle_timeout(builder, 30);
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_max_frame_size() {
        let builder = amqpconnectionoptionsbuilder_create();
        let result = amqpconnectionoptionsbuilder_set_max_frame_size(builder, 65536);
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_channel_max() {
        let builder = amqpconnectionoptionsbuilder_create();
        let result = amqpconnectionoptionsbuilder_set_channel_max(builder, 256);
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_outgoing_locales() {
        let builder = amqpconnectionoptionsbuilder_create();
        let locales = vec![
            CString::new("en-US").unwrap(),
            CString::new("fr-FR").unwrap(),
        ];
        let locale_ptrs: Vec<*const c_char> =
            locales.iter().map(|locale| locale.as_ptr()).collect();
        let result = amqpconnectionoptionsbuilder_set_outgoing_locales(
            builder,
            locale_ptrs.as_ptr(),
            locale_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_incoming_locales() {
        let builder = amqpconnectionoptionsbuilder_create();
        let locales = vec![
            CString::new("en-US").unwrap(),
            CString::new("fr-FR").unwrap(),
        ];
        let locale_ptrs: Vec<*const c_char> =
            locales.iter().map(|locale| locale.as_ptr()).collect();
        let result = amqpconnectionoptionsbuilder_set_incoming_locales(
            builder,
            locale_ptrs.as_ptr(),
            locale_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_offered_capabilities() {
        let builder = amqpconnectionoptionsbuilder_create();
        let capabilities = vec![
            CString::new("capability1").unwrap(),
            CString::new("capability2").unwrap(),
        ];
        let capability_ptrs: Vec<*const c_char> =
            capabilities.iter().map(|cap| cap.as_ptr()).collect();
        let result = amqpconnectionoptionsbuilder_set_offered_capabilities(
            builder,
            capability_ptrs.as_ptr(),
            capability_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_desired_capabilities() {
        let builder = amqpconnectionoptionsbuilder_create();
        let capabilities = vec![
            CString::new("capability1").unwrap(),
            CString::new("capability2").unwrap(),
        ];
        let capability_ptrs: Vec<*const c_char> =
            capabilities.iter().map(|cap| cap.as_ptr()).collect();
        let result = amqpconnectionoptionsbuilder_set_desired_capabilities(
            builder,
            capability_ptrs.as_ptr(),
            capability_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_properties() {
        let builder = amqpconnectionoptionsbuilder_create();
        let mut map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = AmqpOrderedMap::new();
        map.insert(AmqpSymbol::from("key1"), AmqpValue::from("value1"));
        map.insert(AmqpSymbol::from("key2"), AmqpValue::from("value2"));

        let rust_value = RustAmqpValue {
            inner: AmqpValue::Map(map.into_iter().map(|f| (f.0.into(), f.1)).collect()),
        };

        let result = amqpconnectionoptionsbuilder_set_properties(
            builder,
            &rust_value as *const RustAmqpValue,
        );
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionoptionsbuilder_set_buffer_size() {
        let builder = amqpconnectionoptionsbuilder_create();
        let result = amqpconnectionoptionsbuilder_set_buffer_size(builder, 1024);
        assert_eq!(result, 0);
        amqpconnectionoptionsbuilder_destroy(builder);
    }
}
