// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use azure_core_amqp::{
    messaging::{AmqpMessageId, AmqpMessageProperties},
    value::{AmqpTimestamp, AmqpValue},
};
use std::mem;
use std::time::UNIX_EPOCH;

use crate::value::RustAmqpValue;

pub struct RustMessageProperties {
    pub(crate) inner: AmqpMessageProperties,
}

#[no_mangle]
extern "C" fn properties_create() -> *mut RustMessageProperties {
    Box::into_raw(Box::new(RustMessageProperties {
        inner: Default::default(),
    }))
}

#[no_mangle]
extern "C" fn properties_destroy(properties: *mut RustMessageProperties) {
    unsafe {
        mem::drop(Box::from_raw(properties));
    }
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
extern "C" fn properties_get_message_id(
    properties: *const RustMessageProperties,
    message_id: &mut *mut RustAmqpValue,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(id) = properties.inner.message_id() {
        *message_id = Box::into_raw(Box::new(RustAmqpValue {
            inner: value_from_message_id(id.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_correlation_id(
    properties: *const RustMessageProperties,
    correlation_id: &mut *mut RustAmqpValue,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(id) = properties.inner.correlation_id() {
        *correlation_id = Box::into_raw(Box::new(RustAmqpValue {
            inner: value_from_message_id(id.clone()),
        }));
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_user_id(
    properties: *const RustMessageProperties,
    value: &mut *const u8,
    size: &mut u32,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(id) = properties.inner.user_id() {
        *value = id.as_ptr();
        *size = id.len() as u32;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_to(
    properties: *const RustMessageProperties,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(to) = properties.inner.to() {
        unsafe {
            *value = Box::into_raw(Box::new(RustAmqpValue {
                inner: AmqpValue::String(to.clone()),
            }))
        };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_subject(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(subject) = properties.inner.subject() {
        unsafe { *string_value = std::ffi::CString::new(subject.clone()).unwrap().into_raw() };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_reply_to(
    properties: *const RustMessageProperties,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(reply_to) = properties.inner.reply_to() {
        unsafe {
            *value = Box::into_raw(Box::new(RustAmqpValue {
                inner: AmqpValue::String(reply_to.clone()),
            }))
        };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_content_type(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(content_type) = properties.inner.content_type() {
        unsafe {
            *string_value = std::ffi::CString::new(content_type.clone().0)
                .unwrap()
                .into_raw()
        };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_content_encoding(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(content_encoding) = properties.inner.content_encoding() {
        unsafe {
            *string_value = std::ffi::CString::new(content_encoding.clone().0)
                .unwrap()
                .into_raw()
        };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_absolute_expiry_time(
    properties: *const RustMessageProperties,
    expiry_time: &mut u64,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(expiry) = properties.inner.absolute_expiry_time() {
        *expiry_time = expiry.0.duration_since(UNIX_EPOCH).unwrap().as_millis() as u64;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_creation_time(
    properties: *const RustMessageProperties,
    creation_time: &mut u64,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(creation) = properties.inner.creation_time() {
        *creation_time = creation.0.duration_since(UNIX_EPOCH).unwrap().as_millis() as u64;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_group_id(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(group_id) = properties.inner.group_id() {
        unsafe { *string_value = std::ffi::CString::new(group_id.clone()).unwrap().into_raw() };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_group_sequence(
    properties: *const RustMessageProperties,
    group_sequence: &mut u32,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(sequence) = properties.inner.group_sequence() {
        *group_sequence = *sequence;
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_get_reply_to_group_id(
    properties: *const RustMessageProperties,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &*properties };
    if let Some(reply_to_group_id) = properties.inner.reply_to_group_id() {
        unsafe {
            *string_value = std::ffi::CString::new(reply_to_group_id.clone())
                .unwrap()
                .into_raw()
        };
    } else {
        return -1;
    }
    0
}

#[no_mangle]
extern "C" fn properties_set_message_id(
    properties: *mut RustMessageProperties,
    message_id: *const RustAmqpValue,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let message_id = unsafe { &*message_id };
    properties.inner.set_message_id(match &message_id.inner {
        AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
        AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
        AmqpValue::Uuid(id) => AmqpMessageId::Uuid(id.clone()),
        AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
        _ => return -1,
    });
    0
}

#[no_mangle]
extern "C" fn properties_set_correlation_id(
    properties: *mut RustMessageProperties,
    correlation_id: *const RustAmqpValue,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let correlation_id = unsafe { &*correlation_id };
    properties
        .inner
        .set_correlation_id(match &correlation_id.inner {
            AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
            AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
            AmqpValue::Uuid(id) => AmqpMessageId::Uuid(id.clone()),
            AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
            _ => return -1,
        });
    0
}

#[no_mangle]
extern "C" fn properties_set_user_id(
    properties: *mut RustMessageProperties,
    value: *const u8,
    size: u32,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let value = unsafe { std::slice::from_raw_parts(value, size as usize) };
    properties.inner.set_user_id(value.to_vec());
    0
}

#[no_mangle]
extern "C" fn properties_set_to(
    properties: *mut RustMessageProperties,
    value: *const RustAmqpValue,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let value = unsafe { &*value };
    if let AmqpValue::String(to) = &value.inner {
        properties.inner.set_to(to.clone());
        0
    } else {
        -1
    }
}

#[no_mangle]
extern "C" fn properties_set_subject(
    properties: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let subject = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    properties.inner.set_subject(subject);
    0
}

#[no_mangle]
extern "C" fn properties_set_reply_to(
    properties: *mut RustMessageProperties,
    value: *const RustAmqpValue,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let value = unsafe { &*value };
    if let AmqpValue::String(reply_to) = &value.inner {
        properties.inner.set_reply_to(reply_to.clone());
        0
    } else {
        -1
    }
}

#[no_mangle]
extern "C" fn properties_set_content_type(
    properties: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let content_type = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    properties.inner.set_content_type(content_type.into());
    0
}

#[no_mangle]
extern "C" fn properties_set_content_encoding(
    properties: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let content_encoding = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    properties
        .inner
        .set_content_encoding(content_encoding.into());
    0
}

#[no_mangle]
extern "C" fn properties_set_absolute_expiry_time(
    properties: *mut RustMessageProperties,
    expiry_time: u64,
) -> i32 {
    let properties = unsafe { &mut *properties };
    properties.inner.set_absolute_expiry_time(AmqpTimestamp(
        UNIX_EPOCH + std::time::Duration::from_millis(expiry_time),
    ));
    0
}

#[no_mangle]
extern "C" fn properties_set_creation_time(
    properties: *mut RustMessageProperties,
    creation_time: u64,
) -> i32 {
    let properties = unsafe { &mut *properties };
    properties.inner.set_creation_time(AmqpTimestamp(
        UNIX_EPOCH + std::time::Duration::from_millis(creation_time),
    ));
    0
}

#[no_mangle]
extern "C" fn properties_set_group_id(
    properties: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let group_id = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    properties.inner.set_group_id(group_id);
    0
}

#[no_mangle]
extern "C" fn properties_set_group_sequence(
    properties: *mut RustMessageProperties,
    group_sequence: u32,
) -> i32 {
    let properties = unsafe { &mut *properties };
    properties.inner.set_group_sequence(group_sequence);
    0
}

#[no_mangle]
extern "C" fn properties_set_reply_to_group_id(
    properties: *mut RustMessageProperties,
    string_value: *const std::os::raw::c_char,
) -> i32 {
    let properties = unsafe { &mut *properties };
    let reply_to_group_id = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    properties.inner.set_reply_to_group_id(reply_to_group_id);
    0
}
