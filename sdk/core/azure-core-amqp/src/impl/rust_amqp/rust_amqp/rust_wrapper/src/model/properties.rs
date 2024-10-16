// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::model::value::RustAmqpValue;
use azure_core_amqp::{
    messaging::{builders::AmqpMessagePropertiesBuilder, AmqpMessageId, AmqpMessageProperties},
    value::{AmqpComposite, AmqpDescriptor, AmqpTimestamp, AmqpValue},
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

pub struct RustMessagePropertiesBuilder {
    pub(crate) inner: AmqpMessagePropertiesBuilder,
}

#[no_mangle]
extern "C" fn properties_builder_create() -> *mut RustMessagePropertiesBuilder {
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: AmqpMessageProperties::builder(),
    }))
}

#[no_mangle]
extern "C" fn properties_builder_destroy(properties: *mut RustMessagePropertiesBuilder) {
    unsafe {
        mem::drop(Box::from_raw(properties));
    }
}

#[no_mangle]
unsafe extern "C" fn properties_set_message_id(
    builder: *mut RustMessagePropertiesBuilder,
    message_id: *const RustAmqpValue,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let message_id = unsafe { &*message_id };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_message_id(match &message_id.inner {
            AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
            AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
            AmqpValue::Uuid(id) => AmqpMessageId::Uuid(*id),
            AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
            _ => return std::ptr::null_mut(),
        }),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_correlation_id(
    builder: *mut RustMessagePropertiesBuilder,
    correlation_id: *const RustAmqpValue,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let correlation_id = unsafe { &*correlation_id };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder
            .inner
            .with_correlation_id(match &correlation_id.inner {
                AmqpValue::Binary(id) => AmqpMessageId::Binary(id.clone()),
                AmqpValue::String(id) => AmqpMessageId::String(id.clone()),
                AmqpValue::Uuid(id) => AmqpMessageId::Uuid(*id),
                AmqpValue::ULong(id) => AmqpMessageId::Ulong(*id),
                _ => return std::ptr::null_mut(),
            }),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_user_id(
    builder: *mut RustMessagePropertiesBuilder,
    value: *const u8,
    size: u32,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let value = unsafe { std::slice::from_raw_parts(value, size as usize) };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_user_id(value.to_vec()),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_to(
    builder: *mut RustMessagePropertiesBuilder,
    value: *const c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let value = unsafe { CStr::from_ptr(value) };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_to(value.to_str().unwrap()),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_subject(
    builder: *mut RustMessagePropertiesBuilder,
    string_value: *const std::os::raw::c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let subject = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_subject(subject),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_reply_to(
    builder: *mut RustMessagePropertiesBuilder,
    value: *const RustAmqpValue,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let value = unsafe { &*value };
    if let AmqpValue::String(reply_to) = &value.inner {
        Box::into_raw(Box::new(RustMessagePropertiesBuilder {
            inner: builder.inner.with_reply_to(reply_to.clone()),
        }))
    } else {
        std::ptr::null_mut()
    }
}

#[no_mangle]
unsafe extern "C" fn properties_set_content_type(
    builder: *mut RustMessagePropertiesBuilder,
    string_value: *const std::os::raw::c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let content_type = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_content_type(content_type),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_content_encoding(
    builder: *mut RustMessagePropertiesBuilder,
    string_value: *const std::os::raw::c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let content_encoding = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_content_encoding(content_encoding),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_absolute_expiry_time(
    builder: *mut RustMessagePropertiesBuilder,
    expiry_time: u64,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_absolute_expiry_time(AmqpTimestamp(
            UNIX_EPOCH + std::time::Duration::from_millis(expiry_time),
        )),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_creation_time(
    builder: *mut RustMessagePropertiesBuilder,
    creation_time: u64,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_creation_time(AmqpTimestamp(
            UNIX_EPOCH + std::time::Duration::from_millis(creation_time),
        )),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_group_id(
    builder: *mut RustMessagePropertiesBuilder,
    string_value: *const std::os::raw::c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let group_id = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_group_id(group_id),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_group_sequence(
    builder: *mut RustMessagePropertiesBuilder,
    group_sequence: u32,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_group_sequence(group_sequence),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_set_reply_to_group_id(
    builder: *mut RustMessagePropertiesBuilder,
    string_value: *const std::os::raw::c_char,
) -> *mut RustMessagePropertiesBuilder {
    let builder = Box::from_raw(builder);
    let reply_to_group_id = unsafe {
        std::ffi::CStr::from_ptr(string_value)
            .to_string_lossy()
            .to_string()
    };
    Box::into_raw(Box::new(RustMessagePropertiesBuilder {
        inner: builder.inner.with_reply_to_group_id(reply_to_group_id),
    }))
}

#[no_mangle]
unsafe extern "C" fn properties_build(
    builder: *mut RustMessagePropertiesBuilder,
    header: *mut *mut RustMessageProperties,
) -> i32 {
    let builder = Box::from_raw(builder);
    *header = Box::into_raw(Box::new(RustMessageProperties {
        inner: builder.inner.build(),
    }));
    0
}

#[no_mangle]
extern "C" fn amqpvalue_get_properties(
    value: *const RustAmqpValue,
    header: &mut *mut RustMessageProperties,
) -> i32 {
    let value = unsafe { &*value };
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
                        println!("Unexpected properties value: {:?}", value);
                        1
                    }
                }
            }
            _ => {
                println!("Unexpected properties descriptor code: {:?}", value);
                *header = std::ptr::null_mut();
                1
            }
        },
        _ => {
            println!("Unexpected properties type: {:?}", value.inner);
            1
        }
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_properties(
    properties: *const RustMessageProperties,
) -> *mut RustAmqpValue {
    let properties = unsafe { &*properties };
    let value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x73),
        properties.inner.clone(),
    )));
    Box::into_raw(Box::new(RustAmqpValue { inner: value }))
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;
    use std::ptr;

    fn create_test_properties() -> *mut RustMessageProperties {
        let properties = AmqpMessageProperties::builder()
            .with_message_id(AmqpMessageId::String("test_message_id".to_string()))
            .with_correlation_id(AmqpMessageId::String("test_correlation_id".to_string()))
            .with_user_id(vec![1, 2, 3, 4])
            .with_to("test_to".to_string())
            .with_subject("test_subject".to_string())
            .with_reply_to("test_reply_to".to_string())
            .with_content_type("test_content_type".to_string())
            .with_content_encoding("test_content_encoding".to_string())
            .with_absolute_expiry_time(AmqpTimestamp(
                UNIX_EPOCH + std::time::Duration::from_secs(1000),
            ))
            .with_creation_time(AmqpTimestamp(
                UNIX_EPOCH + std::time::Duration::from_secs(500),
            ))
            .with_group_id("test_group_id".to_string())
            .with_group_sequence(42)
            .with_reply_to_group_id("test_reply_to_group_id".to_string())
            .build();
        Box::into_raw(Box::new(RustMessageProperties { inner: properties }))
    }

    #[test]
    fn test_properties_get_message_id() {
        let properties = create_test_properties();
        let mut message_id: *mut RustAmqpValue = ptr::null_mut();
        let result = properties_get_message_id(properties, &mut message_id);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                (*message_id).inner,
                AmqpValue::String("test_message_id".to_string())
            );
            drop(Box::from_raw(message_id));
        }
        unsafe {
            drop(Box::from_raw(properties));
        }
    }

    #[test]
    fn test_properties_get_correlation_id() {
        let properties = create_test_properties();
        let mut correlation_id: *mut RustAmqpValue = ptr::null_mut();
        let result = properties_get_correlation_id(properties, &mut correlation_id);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                (*correlation_id).inner,
                AmqpValue::String("test_correlation_id".to_string())
            );
            drop(Box::from_raw(correlation_id));
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_user_id() {
        let properties = create_test_properties();
        let mut value: *const u8 = ptr::null();
        let mut size: u32 = 0;
        let result = properties_get_user_id(properties, &mut value, &mut size);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                std::slice::from_raw_parts(value, size as usize),
                &[1, 2, 3, 4]
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_to() {
        let properties = create_test_properties();
        let mut value: *mut RustAmqpValue = ptr::null_mut();
        let result = properties_get_to(properties, &mut value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!((*value).inner, AmqpValue::String("test_to".to_string()));
            drop(Box::from_raw(value));
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_subject() {
        let properties = create_test_properties();
        let mut string_value: *const std::os::raw::c_char = ptr::null();
        let result = properties_get_subject(properties, &mut string_value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_subject"
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_reply_to() {
        let properties = create_test_properties();
        let mut value: *mut RustAmqpValue = ptr::null_mut();
        let result = properties_get_reply_to(properties, &mut value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                (*value).inner,
                AmqpValue::String("test_reply_to".to_string())
            );
            drop(Box::from_raw(value));
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_content_type() {
        let properties = create_test_properties();
        let mut string_value: *const std::os::raw::c_char = ptr::null();
        let result = properties_get_content_type(properties, &mut string_value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_content_type"
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_content_encoding() {
        let properties = create_test_properties();
        let mut string_value: *const std::os::raw::c_char = ptr::null();
        let result = properties_get_content_encoding(properties, &mut string_value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_content_encoding"
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_absolute_expiry_time() {
        let properties = create_test_properties();
        let mut expiry_time: u64 = 0;
        let result = properties_get_absolute_expiry_time(properties, &mut expiry_time);
        assert_eq!(result, 0);
        assert_eq!(expiry_time, 1000 * 1000);
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_creation_time() {
        let properties = create_test_properties();
        let mut creation_time: u64 = 0;
        let result = properties_get_creation_time(properties, &mut creation_time);
        assert_eq!(result, 0);
        assert_eq!(creation_time, 500 * 1000);
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_group_id() {
        let properties = create_test_properties();
        let mut string_value: *const std::os::raw::c_char = ptr::null();
        let result = properties_get_group_id(properties, &mut string_value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_group_id"
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_group_sequence() {
        let properties = create_test_properties();
        let mut group_sequence: u32 = 0;
        let result = properties_get_group_sequence(properties, &mut group_sequence);
        assert_eq!(result, 0);
        assert_eq!(group_sequence, 42);
        unsafe { drop(Box::from_raw(properties)) };
    }

    #[test]
    fn test_properties_get_reply_to_group_id() {
        let properties = create_test_properties();
        let mut string_value: *const std::os::raw::c_char = ptr::null();
        let result = properties_get_reply_to_group_id(properties, &mut string_value);
        assert_eq!(result, 0);
        unsafe {
            assert_eq!(
                CString::from_raw(string_value as *mut _).to_str().unwrap(),
                "test_reply_to_group_id"
            );
        }
        unsafe { drop(Box::from_raw(properties)) };
    }
}
