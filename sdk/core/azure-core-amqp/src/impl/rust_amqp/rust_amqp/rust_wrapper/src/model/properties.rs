// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str, error_from_string,
    model::value::RustAmqpValue,
};
use azure_core_amqp::{
    messaging::{AmqpMessageId, AmqpMessageProperties},
    value::{AmqpComposite, AmqpDescriptor, AmqpSymbol, AmqpTimestamp, AmqpValue},
};
use std::{
    time::UNIX_EPOCH,
    {
        ffi::{c_char, CStr},
        mem,
    },
};

pub struct RustMessageProperties {
    pub(crate) inner: AmqpMessageProperties,
}
#[no_mangle]
extern "C" fn properties_create() -> *mut RustMessageProperties {
    Box::into_raw(Box::new(RustMessageProperties {
        inner: AmqpMessageProperties::default(),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_destroy(properties: *mut RustMessageProperties) {
    mem::drop(Box::from_raw(properties));
}

fn value_from_message_id(message_id: AmqpMessageId) -> AmqpValue {
    match message_id {
        AmqpMessageId::Binary(id) => AmqpValue::Binary(id),
        AmqpMessageId::String(id) => AmqpValue::String(id),
        AmqpMessageId::Uuid(id) => AmqpValue::Uuid(id),
        AmqpMessageId::Ulong(id) => AmqpValue::ULong(id),
    }
}

#[no_mangle]
unsafe extern "C" fn properties_get_message_id(
    properties: *const RustMessageProperties,
    message_id: &mut *mut RustAmqpValue,
) -> i32 {
    let properties = &*properties;
    if let Some(id) = &properties.inner.message_id {
        *message_id = Box::into_raw(Box::new(RustAmqpValue {
            inner: value_from_message_id(id.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_correlation_id(
    properties: *const RustMessageProperties,
    correlation_id: &mut *mut RustAmqpValue,
) -> i32 {
    let properties = &*properties;
    if let Some(id) = &properties.inner.correlation_id {
        *correlation_id = Box::into_raw(Box::new(RustAmqpValue {
            inner: value_from_message_id(id.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_user_id(
    properties: *const RustMessageProperties,
    value: &mut *const u8,
    size: &mut u32,
) -> i32 {
    let properties = &*properties;
    if let Some(id) = &properties.inner.user_id {
        *value = id.as_ptr();
        *size = id.len() as u32;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_to(
    properties: *const RustMessageProperties,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let properties = &*properties;
    if let Some(to) = &properties.inner.to {
        *value = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::String(to.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_subject(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = &*properties;
    if let Some(subject) = &properties.inner.subject {
        *string_value = std::ffi::CString::new(subject.clone()).unwrap().into_raw();
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_reply_to(
    properties: *const RustMessageProperties,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let properties = &*properties;
    if let Some(reply_to) = &properties.inner.reply_to {
        *value = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::String(reply_to.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_content_type(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = { &*properties };
    if let Some(content_type) = &properties.inner.content_type {
        *string_value = std::ffi::CString::new(content_type.clone().0)
            .unwrap()
            .into_raw();
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_content_encoding(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = { &*properties };
    if let Some(content_encoding) = &properties.inner.content_encoding {
        *string_value = std::ffi::CString::new(content_encoding.clone().0)
            .unwrap()
            .into_raw();
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_absolute_expiry_time(
    properties: *const RustMessageProperties,
    expiry_time: &mut u64,
) -> i32 {
    let properties = { &*properties };
    if let Some(expiry) = &properties.inner.absolute_expiry_time {
        *expiry_time = (*expiry).0.map_or(u64::MAX, |t| {
            t.duration_since(UNIX_EPOCH).unwrap().as_millis() as u64
        });
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_creation_time(
    properties: *const RustMessageProperties,
    creation_time: &mut u64,
) -> i32 {
    let properties = { &*properties };
    if let Some(creation) = &properties.inner.creation_time {
        *creation_time = (*creation).0.map_or(u64::MAX, |t| {
            t.duration_since(UNIX_EPOCH).unwrap().as_millis() as u64
        });
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_group_id(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = { &*properties };
    if let Some(group_id) = &properties.inner.group_id {
        *string_value = std::ffi::CString::new(group_id.clone()).unwrap().into_raw();
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_group_sequence(
    properties: *const RustMessageProperties,
    group_sequence: &mut u32,
) -> i32 {
    let properties = { &*properties };
    if let Some(sequence) = &properties.inner.group_sequence {
        *group_sequence = *sequence;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_get_reply_to_group_id(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = { &*properties };
    if let Some(reply_to_group_id) = &properties.inner.reply_to_group_id {
        *string_value = std::ffi::CString::new(reply_to_group_id.clone())
            .unwrap()
            .into_raw();
    } else {
        return -1;
    }
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_message_id(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    message_id: *const RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let message_id = &*message_id;
    options.inner.message_id = Some(match &message_id.inner {
        AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
        AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
        AmqpValue::Uuid(id) => AmqpMessageId::Uuid(*id),
        AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
        _ => {
            call_context.set_error(error_from_string(format!(
                "Could not convert AmqpValue to MessageId, found: {:?}",
                message_id.inner
            )));
            return -1;
        }
    });
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_correlation_id(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    correlation_id: *const RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let correlation_id = &*correlation_id;
    options.inner.correlation_id = Some(match &correlation_id.inner {
        AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
        AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
        AmqpValue::Uuid(id) => AmqpMessageId::Uuid(*id),
        AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
        _ => {
            call_context.set_error(error_from_string(format!(
                "Could not convert AmqpValue to MessageId, found: {:?}",
                correlation_id.inner
            )));
            return -1;
        }
    });
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_user_id(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    value: *const u8,
    size: u32,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let value = std::slice::from_raw_parts(value, size as usize);
    options.inner.user_id = Some(value.to_vec());
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_to(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    value: *const c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let value = { CStr::from_ptr(value) };
    options.inner.to = Some(value.to_str().unwrap().to_string());
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_subject(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let subject = {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    options.inner.subject = Some(subject);
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_reply_to(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    value: *const RustAmqpValue,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let value = { &*value };
    if let AmqpValue::String(reply_to) = &value.inner {
        options.inner.reply_to = Some(reply_to.clone());
        0
    } else {
        call_context.set_error(error_from_str("ReplyTo value must be a string."));
        -1
    }
}

#[no_mangle]
unsafe extern "C" fn properties_set_content_type(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let content_type = {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    options.inner.content_type = Some(AmqpSymbol::from(content_type));
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_content_encoding(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let content_encoding = {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    options.inner.content_encoding = Some(AmqpSymbol::from(content_encoding));
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_absolute_expiry_time(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    expiry_time: u64,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.absolute_expiry_time = Some(AmqpTimestamp(Some(
        UNIX_EPOCH + std::time::Duration::from_millis(expiry_time),
    )));
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_creation_time(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    creation_time: u64,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.creation_time = Some(AmqpTimestamp(Some(
        UNIX_EPOCH + std::time::Duration::from_millis(creation_time),
    )));
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_group_id(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let group_id = {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    options.inner.group_id = Some(group_id);
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_group_sequence(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    group_sequence: u32,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.group_sequence = Some(group_sequence);
    0
}

#[no_mangle]
unsafe extern "C" fn properties_set_reply_to_group_id(
    call_context: *mut RustCallContext,
    options: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    let reply_to_group_id = {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    options.inner.reply_to_group_id = Some(reply_to_group_id);
    0
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_get_properties(
    call_context: *mut RustCallContext,
    value: *const RustAmqpValue,
    header: &mut *mut RustMessageProperties,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let value = { &*value };
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x73) => {
                let h = value.value();
                match h {
                    AmqpValue::List(c) => {
                        *header = Box::into_raw(Box::new(RustMessageProperties {
                            inner: AmqpMessageProperties::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *header = std::ptr::null_mut();
                        call_context.set_error(error_from_string(format!(
                            "Properties value must be an AMQP list. Found: {:?}",
                            h
                        )));
                        -1
                    }
                }
            }
            _ => {
                call_context.set_error(error_from_string(format!(
                    "Unexpected AMQP descriptor code: {:?}",
                    value
                )));
                *header = std::ptr::null_mut();
                1
            }
        },
        _ => {
            call_context.set_error(error_from_string(format!(
                "Properties must be a described type. found: {:?}",
                value.inner
            )));
            1
        }
    }
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_create_properties(
    properties: *const RustMessageProperties,
) -> *mut RustAmqpValue {
    let properties = { &*properties };
    let value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x73),
        properties.inner.clone(),
    )));
    Box::into_raw(Box::new(RustAmqpValue { inner: value }))
}

#[cfg(test)]
mod tests {
    use azure_core_amqp::value::AmqpSymbol;

    use super::*;
    use std::ffi::CString;
    use std::ptr;

    fn create_test_properties() -> *mut RustMessageProperties {
        let properties = AmqpMessageProperties {
            message_id: Some(AmqpMessageId::String("test_message_id".to_string())),
            correlation_id: Some(AmqpMessageId::String("test_correlation_id".to_string())),
            user_id: Some(vec![1, 2, 3, 4]),
            to: Some("test_to".to_string()),
            subject: Some("test_subject".to_string()),
            reply_to: Some("test_reply_to".to_string()),
            content_type: Some(AmqpSymbol::from("test_content_type")),
            content_encoding: Some(AmqpSymbol::from("test_content_encoding")),
            absolute_expiry_time: Some(AmqpTimestamp(Some(
                UNIX_EPOCH + std::time::Duration::from_secs(1000),
            ))),
            creation_time: Some(AmqpTimestamp(Some(
                UNIX_EPOCH + std::time::Duration::from_secs(500),
            ))),
            group_id: Some("test_group_id".to_string()),
            group_sequence: Some(42),
            reply_to_group_id: Some("test_reply_to_group_id".to_string()),
        };
        Box::into_raw(Box::new(RustMessageProperties { inner: properties }))
    }

    #[test]
    fn test_properties_get_message_id() {
        unsafe {
            let properties = create_test_properties();
            let mut message_id: *mut RustAmqpValue = ptr::null_mut();
            let result = properties_get_message_id(properties, &mut message_id);
            assert_eq!(result, 0);

            assert_eq!(
                (*message_id).inner,
                AmqpValue::String("test_message_id".to_string())
            );
            drop(Box::from_raw(message_id));

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_correlation_id() {
        unsafe {
            let properties = create_test_properties();
            let mut correlation_id: *mut RustAmqpValue = ptr::null_mut();
            let result = properties_get_correlation_id(properties, &mut correlation_id);
            assert_eq!(result, 0);

            assert_eq!(
                (*correlation_id).inner,
                AmqpValue::String("test_correlation_id".to_string())
            );
            drop(Box::from_raw(correlation_id));

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_user_id() {
        unsafe {
            let properties = create_test_properties();
            let mut value: *const u8 = ptr::null();
            let mut size: u32 = 0;
            let result = properties_get_user_id(properties, &mut value, &mut size);
            assert_eq!(result, 0);

            assert_eq!(
                std::slice::from_raw_parts(value, size as usize),
                &[1, 2, 3, 4]
            );

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_to() {
        unsafe {
            let properties = create_test_properties();
            let mut value: *mut RustAmqpValue = ptr::null_mut();
            let result = properties_get_to(properties, &mut value);
            assert_eq!(result, 0);

            assert_eq!((*value).inner, AmqpValue::String("test_to".to_string()));
            drop(Box::from_raw(value));

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_subject() {
        unsafe {
            let properties = create_test_properties();
            let mut string_value: *const std::os::raw::c_char = ptr::null();
            let result = properties_get_subject(properties, &mut string_value);
            assert_eq!(result, 0);

            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_subject"
            );

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_reply_to() {
        unsafe {
            let properties = create_test_properties();
            let mut value: *mut RustAmqpValue = ptr::null_mut();
            let result = properties_get_reply_to(properties, &mut value);
            assert_eq!(result, 0);

            assert_eq!(
                (*value).inner,
                AmqpValue::String("test_reply_to".to_string())
            );
            drop(Box::from_raw(value));

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_content_type() {
        unsafe {
            let properties = create_test_properties();
            let mut string_value: *const std::os::raw::c_char = ptr::null();
            let result = properties_get_content_type(properties, &mut string_value);
            assert_eq!(result, 0);

            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_content_type"
            );

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_content_encoding() {
        unsafe {
            let properties = create_test_properties();
            let mut string_value: *const std::os::raw::c_char = ptr::null();
            let result = properties_get_content_encoding(properties, &mut string_value);
            assert_eq!(result, 0);

            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_content_encoding"
            );

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_absolute_expiry_time() {
        unsafe {
            let properties = create_test_properties();
            let mut expiry_time: u64 = 0;
            let result = properties_get_absolute_expiry_time(properties, &mut expiry_time);
            assert_eq!(result, 0);
            assert_eq!(expiry_time, 1000 * 1000);

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_creation_time() {
        unsafe {
            let properties = create_test_properties();
            let mut creation_time: u64 = 0;
            let result = properties_get_creation_time(properties, &mut creation_time);
            assert_eq!(result, 0);
            assert_eq!(creation_time, 500 * 1000);

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_group_id() {
        unsafe {
            let properties = create_test_properties();
            let mut string_value: *const std::os::raw::c_char = ptr::null();
            let result = properties_get_group_id(properties, &mut string_value);
            assert_eq!(result, 0);

            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_group_id"
            );

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_group_sequence() {
        unsafe {
            let properties = create_test_properties();
            let mut group_sequence: u32 = 0;
            let result = properties_get_group_sequence(properties, &mut group_sequence);
            assert_eq!(result, 0);
            assert_eq!(group_sequence, 42);

            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_reply_to_group_id() {
        unsafe {
            let properties = create_test_properties();
            let mut string_value: *const std::os::raw::c_char = ptr::null();
            let result = properties_get_reply_to_group_id(properties, &mut string_value);
            assert_eq!(result, 0);

            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_reply_to_group_id"
            );

            drop(Box::from_raw(properties));
        }
    }
}
