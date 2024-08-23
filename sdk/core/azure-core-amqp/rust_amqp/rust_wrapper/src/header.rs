// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use azure_core_amqp::{
    messaging::{builders::AmqpMessageHeaderBuilder, AmqpMessageHeader},
    value::{AmqpComposite, AmqpDescriptor, AmqpValue},
};
use std::mem;

use crate::value::RustAmqpValue;

pub struct RustMessageHeader {
    inner: AmqpMessageHeader,
}

impl RustMessageHeader {
    pub(crate) fn new(header: AmqpMessageHeader) -> Self {
        Self { inner: header }
    }

    pub(crate) fn get(&self) -> &AmqpMessageHeader {
        &self.inner
    }
}

#[no_mangle]
extern "C" fn header_destroy(header: *mut RustMessageHeader) {
    unsafe {
        mem::drop(Box::from_raw(header));
    }
}

#[no_mangle]
extern "C" fn header_get_durable(header: *const RustMessageHeader, durable: &mut bool) -> u32 {
    let header = unsafe { &*header };
    if let Some(d) = header.inner.durable() {
        *durable = *d;
        0
    } else {
        *durable = false;
        1
    }
}

#[no_mangle]
extern "C" fn header_get_priority(header: *const RustMessageHeader, priority: &mut u8) -> u32 {
    let header = unsafe { &*header };
    if let Some(p) = header.inner.priority() {
        *priority = *p;
        0
    } else {
        *priority = 4;
        1
    }
}

#[no_mangle]
extern "C" fn header_get_ttl(header: *const RustMessageHeader, time_to_live: &mut u64) -> u32 {
    let header = unsafe { &*header };
    if let Some(ttl) = header.inner.time_to_live() {
        *time_to_live = ttl.as_millis() as u64;
        0
    } else {
        *time_to_live = 0;
        1
    }
}

#[no_mangle]
extern "C" fn header_get_first_acquirer(
    header: *const RustMessageHeader,
    first_acquirer: &mut bool,
) -> u32 {
    let header = unsafe { &*header };
    if let Some(fa) = header.inner.first_acquirer() {
        *first_acquirer = *fa;
        0
    } else {
        *first_acquirer = false;
        1
    }
}

#[no_mangle]
extern "C" fn header_get_delivery_count(
    header: *const RustMessageHeader,
    delivery_count: &mut u32,
) -> u32 {
    let header = unsafe { &*header };
    if let Some(dc) = header.inner.delivery_count() {
        *delivery_count = *dc;
        0
    } else {
        *delivery_count = 0;
        1
    }
}

pub struct RustMessageHeaderBuilder {
    inner: AmqpMessageHeaderBuilder,
}

#[no_mangle]
extern "C" fn header_builder_create() -> *mut RustMessageHeaderBuilder {
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: AmqpMessageHeader::builder(),
    }))
}

#[no_mangle]
extern "C" fn header_builder_destroy(builder: *mut RustMessageHeaderBuilder) {
    unsafe {
        mem::drop(Box::from_raw(builder));
    }
}

#[no_mangle]
extern "C" fn header_set_durable(builder: *mut RustMessageHeaderBuilder, durable: bool) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_durable(durable);
    0
}

#[no_mangle]
extern "C" fn header_set_priority(builder: *mut RustMessageHeaderBuilder, priority: u8) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_priority(priority);
    0
}

#[no_mangle]
extern "C" fn header_set_ttl(builder: *mut RustMessageHeaderBuilder, time_to_live: u64) -> u32 {
    let builder = unsafe { &mut *builder };
    builder
        .inner
        .with_time_to_live(std::time::Duration::from_millis(time_to_live));
    0
}

#[no_mangle]
extern "C" fn header_set_first_acquirer(
    builder: *mut RustMessageHeaderBuilder,
    first_acquirer: bool,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_first_acquirer(first_acquirer);
    0
}

#[no_mangle]
extern "C" fn header_set_delivery_count(
    builder: *mut RustMessageHeaderBuilder,
    delivery_count: u32,
) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_delivery_count(delivery_count);
    0
}

#[no_mangle]
extern "C" fn header_build(
    builder: *mut RustMessageHeaderBuilder,
    header: *mut *mut RustMessageHeader,
) -> u32 {
    let builder = unsafe { &mut *builder };
    unsafe {
        *header = Box::into_raw(Box::new(RustMessageHeader {
            inner: builder.inner.build(),
        }))
    };
    0
}

#[no_mangle]
extern "C" fn amqpvalue_get_header(
    value: *const RustAmqpValue,
    header: &mut *mut RustMessageHeader,
) -> u32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x70) => {
                let h = value.value();
                match h {
                    AmqpValue::Composite(c) => {
                        *header = Box::into_raw(Box::new(RustMessageHeader {
                            inner: AmqpMessageHeader::from(c),
                        }));
                        0
                    }
                    AmqpValue::List(c) => {
                        *header = Box::into_raw(Box::new(RustMessageHeader {
                            inner: AmqpMessageHeader::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *header = std::ptr::null_mut();
                        1
                    }
                }
            }
            _ => {
                *header = std::ptr::null_mut();
                1
            }
        },
        _ => 1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_header(header: *const RustMessageHeader) -> *mut RustAmqpValue {
    let header = unsafe { &*header };
    let value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x70),
        header.inner.clone(),
    )));
    Box::into_raw(Box::new(RustAmqpValue { inner: value }))
}
