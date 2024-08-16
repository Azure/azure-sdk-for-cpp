// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use azure_core_amqp::messaging::AmqpMessageHeader;
use std::mem;

pub struct RustMessageHeader {
    pub(crate) inner: AmqpMessageHeader,
}

#[no_mangle]
extern "C" fn header_create() -> *mut RustMessageHeader {
    Box::into_raw(Box::new(RustMessageHeader {
        inner: Default::default(),
    }))
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

#[no_mangle]
extern "C" fn header_set_durable(header: *mut RustMessageHeader, durable: bool) -> u32 {
    let header = unsafe { &mut *header };
    header.inner.set_durable(durable);
    0
}

#[no_mangle]
extern "C" fn header_set_priority(header: *mut RustMessageHeader, priority: u8) -> u32 {
    let header = unsafe { &mut *header };
    header.inner.set_priority(priority);
    0
}

#[no_mangle]
extern "C" fn header_set_ttl(header: *mut RustMessageHeader, time_to_live: u64) -> u32 {
    let header = unsafe { &mut *header };
    header
        .inner
        .set_time_to_live(std::time::Duration::from_millis(time_to_live));
    0
}

#[no_mangle]
extern "C" fn header_set_first_acquirer(
    header: *mut RustMessageHeader,
    first_acquirer: bool,
) -> u32 {
    let header = unsafe { &mut *header };
    header.inner.set_first_acquirer(first_acquirer);
    0
}

#[no_mangle]
extern "C" fn header_set_delivery_count(
    header: *mut RustMessageHeader,
    delivery_count: u32,
) -> u32 {
    let header = unsafe { &mut *header };
    header.inner.set_delivery_count(delivery_count);
    0
}
