// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::model::value::RustAmqpValue;
use azure_core_amqp::value::{AmqpDescribed, AmqpValue};

#[no_mangle]
extern "C" fn amqpvalue_create_delivery_annotations(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let annotations = match &value.inner {
        AmqpValue::Map(map) => map,
        _ => return std::ptr::null_mut(),
    };

    let delivery_annotations = AmqpDescribed::new(0x71, AmqpValue::Map(annotations.clone()));
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(delivery_annotations)),
    }))
}

#[no_mangle]
extern "C" fn amqpvalue_create_message_annotations(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let annotations = match &value.inner {
        AmqpValue::Map(map) => map,
        _ => return std::ptr::null_mut(),
    };

    let message_annotations = AmqpDescribed::new(0x72, AmqpValue::Map(annotations.clone()));
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(message_annotations)),
    }))
}

#[no_mangle]
extern "C" fn amqpvalue_create_footer(value: *const RustAmqpValue) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let annotations = match &value.inner {
        AmqpValue::Map(map) => map,
        _ => return std::ptr::null_mut(),
    };

    let footer = AmqpDescribed::new(0x78, AmqpValue::Map(annotations.clone()));
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(footer)),
    }))
}

#[no_mangle]
extern "C" fn amqpvalue_create_application_properties(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let properties = match &value.inner {
        AmqpValue::Map(map) => map,
        _ => return std::ptr::null_mut(),
    };

    let application_properties = AmqpDescribed::new(0x74, AmqpValue::Map(properties.clone()));
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(application_properties)),
    }))
}
