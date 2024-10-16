// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use azure_core_amqp::{
    messaging::{builders::AmqpMessageHeaderBuilder, AmqpMessageHeader},
    value::{AmqpComposite, AmqpDescriptor, AmqpValue},
};
use std::mem;

use super::value::RustAmqpValue;

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
extern "C" fn header_get_durable(header: *const RustMessageHeader, durable: &mut bool) -> i32 {
    let header = unsafe { &*header };
    *durable = header.inner.durable();
    0
}

#[no_mangle]
extern "C" fn header_get_priority(header: *const RustMessageHeader, priority: &mut u8) -> i32 {
    let header = unsafe { &*header };
    *priority = header.inner.priority();
    0
}

#[no_mangle]
extern "C" fn header_get_ttl(header: *const RustMessageHeader, time_to_live: &mut u64) -> i32 {
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
) -> i32 {
    let header = unsafe { &*header };

    *first_acquirer = header.inner.first_acquirer();
    0
}

#[no_mangle]
extern "C" fn header_get_delivery_count(
    header: *const RustMessageHeader,
    delivery_count: &mut u32,
) -> i32 {
    let header = unsafe { &*header };
    *delivery_count = header.inner.delivery_count();
    0
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
unsafe extern "C" fn header_set_durable(
    builder: *mut RustMessageHeaderBuilder,
    durable: bool,
) -> *mut RustMessageHeaderBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: builder.inner.with_durable(durable),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_set_priority(
    builder: *mut RustMessageHeaderBuilder,
    priority: u8,
) -> *mut RustMessageHeaderBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: builder.inner.with_priority(priority),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_set_ttl(
    builder: *mut RustMessageHeaderBuilder,
    time_to_live: u64,
) -> *mut RustMessageHeaderBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: builder
            .inner
            .with_time_to_live(Some(std::time::Duration::from_millis(time_to_live))),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_set_first_acquirer(
    builder: *mut RustMessageHeaderBuilder,
    first_acquirer: bool,
) -> *mut RustMessageHeaderBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: builder.inner.with_first_acquirer(first_acquirer),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_set_delivery_count(
    builder: *mut RustMessageHeaderBuilder,
    delivery_count: u32,
) -> *mut RustMessageHeaderBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustMessageHeaderBuilder {
        inner: builder.inner.with_delivery_count(delivery_count),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_build(
    builder: *mut RustMessageHeaderBuilder,
    header: *mut *mut RustMessageHeader,
) -> i32 {
    let builder = Box::from_raw(builder);
    *header = Box::into_raw(Box::new(RustMessageHeader {
        inner: builder.inner.build(),
    }));
    0
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_get_header(
    value: *const RustAmqpValue,
    header: &mut *mut RustMessageHeader,
) -> i32 {
    let value = &*value;
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x70) => {
                let h = value.value();
                match h {
                    AmqpValue::List(c) => {
                        *header = Box::into_raw(Box::new(RustMessageHeader {
                            inner: AmqpMessageHeader::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *header = std::ptr::null_mut();
                        println!("Invalid header type: {:?}", h);
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

#[cfg(test)]
mod tests {

    use azure_core_amqp::Deserializable;

    use crate::model::value::{amqpvalue_encode, amqpvalue_get_encoded_size};

    use super::*;

    #[test]
    fn test_amqpvalue_create_header() {
        let header = RustMessageHeader {
            inner: AmqpMessageHeader::builder().with_priority(3).build(),
        };
        let value = amqpvalue_create_header(&header);
        //        let value = unsafe { Box::from_raw(value) };

        let mut size: usize = 0;
        assert_eq!(
            unsafe { amqpvalue_get_encoded_size(value.clone(), &mut size as *mut usize) },
            0
        );
        let mut buffer: Vec<u8> = vec![0; size];
        assert_eq!(
            unsafe { amqpvalue_encode(value, buffer.as_mut_ptr(), size) },
            0
        );

        let deserialized = <AmqpValue as Deserializable<AmqpValue>>::decode(buffer.as_slice());
        assert!(deserialized.is_ok());
        let deserialized = deserialized.unwrap();
        println!("Deserialized: {:?}", deserialized);

        // let mut header2 = Box::new(RustMessageHeader {
        //     inner: AmqpMessageHeader::builder().with_priority(3).build(),
        // });
        // assert_eq!(
        //     amqpvalue_get_header(
        //         deserialized.as_ref(),
        //         &mut Box::into_raw(header2) as &mut *mut RustMessageHeader
        //     ),
        //     0
        // );
    }
}
