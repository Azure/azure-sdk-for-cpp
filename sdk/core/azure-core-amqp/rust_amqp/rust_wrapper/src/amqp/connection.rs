// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpconnection amqpconnectionbuilder

use std::{
    ffi::{c_char, CStr},
    mem,
    ptr::null,
};
use time::Duration;

use crate::runtime_context::{runtime_context_from_ptr_mut, RuntimeContext};

use tracing::error;
use url::Url;

use azure_core_amqp::{
    connection::{
        builders::AmqpConnectionOptionsBuilder, AmqpConnection, AmqpConnectionApis,
        AmqpConnectionOptions,
    },
    value::AmqpSymbol,
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
pub extern "C" fn amqpconnectionbuilder_create() -> *mut RustAmqpConnectionOptionsBuilder {
    let builder = AmqpConnectionOptions::builder();
    Box::into_raw(Box::new(RustAmqpConnectionOptionsBuilder {
        inner: builder,
    }))
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_destroy(builder: *mut RustAmqpConnectionOptionsBuilder) {
    unsafe {
        mem::drop(Box::from_raw(builder));
    }
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_set_idle_timeout(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    idle_timeout: u32,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder
        .inner
        .with_idle_timeout(Duration::seconds(idle_timeout as i64));
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_set_max_frame_size(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    max_frame_size: u32,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_max_frame_size(max_frame_size);
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_set_channel_max(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    channel_max: u16,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_channel_max(channel_max);
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_set_outgoing_locales(
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
pub extern "C" fn amqpconnectionbuilder_set_incoming_locales(
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
pub extern "C" fn amqpconnectionbuilder_set_offered_capabilities(
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
pub extern "C" fn amqpconnectionbuilder_set_desired_capabilities(
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
pub extern "C" fn amqpconnectionbuilder_set_properties(
    builder: *mut RustAmqpConnectionOptionsBuilder,
    keys: *const *const c_char,
    values: *const *const c_char,
    count: usize,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let keys = unsafe { std::slice::from_raw_parts(keys, count) };
    let values = unsafe { std::slice::from_raw_parts(values, count) };
    let properties = keys
        .iter()
        .zip(values.iter())
        .map(|(key, value)| {
            (unsafe { CStr::from_ptr(*key).to_str().unwrap() }, unsafe {
                CStr::from_ptr(*value).to_str().unwrap()
            })
        })
        .collect::<Vec<(&str, &str)>>();

    // Apply the code block changes
    builder.inner.with_properties(properties);

    // Return the result
    0
}

#[no_mangle]
pub extern "C" fn amqpconnectionbuilder_set_buffer_size(
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
    fn test_amqpconnectionbuilder_create_and_destroy() {
        let builder = amqpconnectionbuilder_create();
        assert!(!builder.is_null());
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_idle_timeout() {
        let builder = amqpconnectionbuilder_create();
        let result = amqpconnectionbuilder_set_idle_timeout(builder, 30);
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_max_frame_size() {
        let builder = amqpconnectionbuilder_create();
        let result = amqpconnectionbuilder_set_max_frame_size(builder, 65536);
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_channel_max() {
        let builder = amqpconnectionbuilder_create();
        let result = amqpconnectionbuilder_set_channel_max(builder, 256);
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_outgoing_locales() {
        let builder = amqpconnectionbuilder_create();
        let locales = vec![
            CString::new("en-US").unwrap(),
            CString::new("fr-FR").unwrap(),
        ];
        let locale_ptrs: Vec<*const c_char> =
            locales.iter().map(|locale| locale.as_ptr()).collect();
        let result = amqpconnectionbuilder_set_outgoing_locales(
            builder,
            locale_ptrs.as_ptr(),
            locale_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_incoming_locales() {
        let builder = amqpconnectionbuilder_create();
        let locales = vec![
            CString::new("en-US").unwrap(),
            CString::new("fr-FR").unwrap(),
        ];
        let locale_ptrs: Vec<*const c_char> =
            locales.iter().map(|locale| locale.as_ptr()).collect();
        let result = amqpconnectionbuilder_set_incoming_locales(
            builder,
            locale_ptrs.as_ptr(),
            locale_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_offered_capabilities() {
        let builder = amqpconnectionbuilder_create();
        let capabilities = vec![
            CString::new("capability1").unwrap(),
            CString::new("capability2").unwrap(),
        ];
        let capability_ptrs: Vec<*const c_char> =
            capabilities.iter().map(|cap| cap.as_ptr()).collect();
        let result = amqpconnectionbuilder_set_offered_capabilities(
            builder,
            capability_ptrs.as_ptr(),
            capability_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_desired_capabilities() {
        let builder = amqpconnectionbuilder_create();
        let capabilities = vec![
            CString::new("capability1").unwrap(),
            CString::new("capability2").unwrap(),
        ];
        let capability_ptrs: Vec<*const c_char> =
            capabilities.iter().map(|cap| cap.as_ptr()).collect();
        let result = amqpconnectionbuilder_set_desired_capabilities(
            builder,
            capability_ptrs.as_ptr(),
            capability_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_properties() {
        let builder = amqpconnectionbuilder_create();
        let keys = vec![CString::new("key1").unwrap(), CString::new("key2").unwrap()];
        let values = vec![
            CString::new("value1").unwrap(),
            CString::new("value2").unwrap(),
        ];
        let key_ptrs: Vec<*const c_char> = keys.iter().map(|key| key.as_ptr()).collect();
        let value_ptrs: Vec<*const c_char> = values.iter().map(|value| value.as_ptr()).collect();
        let result = amqpconnectionbuilder_set_properties(
            builder,
            key_ptrs.as_ptr(),
            value_ptrs.as_ptr(),
            key_ptrs.len(),
        );
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }

    #[test]
    fn test_amqpconnectionbuilder_set_buffer_size() {
        let builder = amqpconnectionbuilder_create();
        let result = amqpconnectionbuilder_set_buffer_size(builder, 1024);
        assert_eq!(result, 0);
        amqpconnectionbuilder_destroy(builder);
    }
}
