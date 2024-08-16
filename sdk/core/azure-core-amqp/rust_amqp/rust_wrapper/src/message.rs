// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::{header::RustMessageHeader, properties::RustMessageProperties, value::RustAmqpValue};
use azure_core_amqp::{
    messaging::{AmqpAnnotationKey, AmqpMessage},
    value::{AmqpOrderedMap, AmqpValue},
};
use std::mem;

pub struct RustAmqpMessage {
    inner: AmqpMessage,
}

#[no_mangle]
extern "C" fn message_create() -> *mut RustAmqpMessage {
    Box::into_raw(Box::new(RustAmqpMessage {
        inner: Default::default(),
    }))
}

#[no_mangle]
extern "C" fn message_destroy(message: *mut RustAmqpMessage) {
    unsafe {
        mem::drop(Box::from_raw(message));
    }
}

#[no_mangle]
extern "C" fn message_get_header(
    message: *const RustAmqpMessage,
    message_header: *mut *mut RustMessageHeader,
) -> u32 {
    let message = unsafe { &*message };
    if message.inner.header().is_none() {
        unsafe { *message_header = std::ptr::null_mut() };
        return 0;
    }
    let header = message.inner.header().unwrap();
    unsafe {
        *message_header = Box::into_raw(Box::new(RustMessageHeader {
            inner: header.clone(),
        }))
    }
    //    let header = message.inner.header();
    //    match header {
    //        Some(h) => unsafe {
    //            *message_header = Box::into_raw(Box::new(RustMessageHeader { inner: h.clone() }))
    //        },
    //        None => unsafe { *message_header = std::ptr::null_mut() },
    //    }
    //    let header = header.map(|h| Box::new(RustMessageHeader { inner: h.clone() }));
    //match header {
    //    Some(h) => unsafe { *message_header = Box::into_raw(h) },
    //    None => unsafe { *message_header = std::ptr::null_mut() },
    //}
    0
}

#[no_mangle]
extern "C" fn message_get_properties(
    message: *const RustAmqpMessage,
    message_properties: *mut *mut RustMessageProperties,
) -> u32 {
    let message = unsafe { &*message };
    let properties = message.inner.properties();
    let properties = properties.map(|p| Box::new(RustMessageProperties { inner: p.clone() }));
    match properties {
        Some(p) => unsafe { *message_properties = Box::into_raw(p) },
        None => unsafe { *message_properties = std::ptr::null_mut() },
    }
    0
}

#[no_mangle]
extern "C" fn message_get_delivery_annotations(
    message: *const RustAmqpMessage,
    delivery_annotations: *mut *mut RustAmqpValue,
) -> u32 {
    let message = unsafe { &*message };
    let amqp_da = message.inner.delivery_annotations();
    match amqp_da {
        Some(da) => {
            let map: AmqpOrderedMap<AmqpValue, AmqpValue> =
                da.0.clone()
                    .into_iter()
                    .map(|f| match f.0 {
                        AmqpAnnotationKey::Symbol(s) => (AmqpValue::from(s), f.1),
                        AmqpAnnotationKey::Ulong(u) => (AmqpValue::from(u), f.1),
                    })
                    .collect();
            unsafe {
                *delivery_annotations = Box::into_raw(Box::new(RustAmqpValue {
                    inner: AmqpValue::Map(map),
                }))
            };
            0
        }
        None => {
            unsafe { *delivery_annotations = std::ptr::null_mut() };
            1
        }
    }

    // let amqp_da = amqp_da.map(|da| {
    //     da.0.into_iter()
    //         .map(|f| {
    //             (match f.0 {
    //                 AmqpAnnotationKey::Symbol(s) => (AmqpValue::from(s), f.1),
    //                 AmqpAnnotationKey::Ulong(u) => (AmqpValue::from(u), f.1),
    //             },)
    //         })
    //         .collect()
    // });
    // match amqp_da {
    //     Some(da) => {
    //         unsafe {
    //             *delivery_annotations = Box::into_raw(Box::new(RustAmqpValue {
    //                 inner: AmqpValue::Map(da),
    //             }))
    //         }
    //         0
    //     }
    //     None => {
    //         unsafe { *delivery_annotations = std::ptr::null_mut() }
    //         1
    //     }
    // }
}

#[no_mangle]
extern "C" fn message_set_header(
    message: *mut RustAmqpMessage,
    message_header: *const RustMessageHeader,
) -> u32 {
    let message = unsafe { &mut *message };
    let header = unsafe { &*message_header };
    message.inner.set_header(header.inner.clone());
    0
}

#[no_mangle]
extern "C" fn message_set_properties(
    message: *mut RustAmqpMessage,
    message_properties: *const RustMessageProperties,
) -> u32 {
    let message = unsafe { &mut *message };
    let properties = unsafe { &*message_properties };
    message.inner.set_properties(properties.inner.clone());
    0
}
