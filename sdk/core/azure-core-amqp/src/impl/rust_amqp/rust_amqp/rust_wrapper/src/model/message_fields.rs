// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::model::value::RustAmqpValue;
use azure_core_amqp::value::{AmqpDescribed, AmqpValue};

#[no_mangle]
unsafe extern "C" fn amqpvalue_create_delivery_annotations(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = { &*value };
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
unsafe extern "C" fn amqpvalue_create_message_annotations(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = { &*value };
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
unsafe extern "C" fn amqpvalue_create_footer(value: *const RustAmqpValue) -> *mut RustAmqpValue {
    let value = { &*value };
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
unsafe extern "C" fn amqpvalue_create_application_properties(
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = { &*value };
    let properties = match &value.inner {
        AmqpValue::Map(map) => map,
        _ => return std::ptr::null_mut(),
    };

    let application_properties = AmqpDescribed::new(0x74, AmqpValue::Map(properties.clone()));
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(application_properties)),
    }))
}
#[cfg(test)]
mod tests {
    use azure_core_amqp::value::AmqpDescriptor;

    use super::*;

    #[test]
    fn test_amqpvalue_create_delivery_annotations() {
        unsafe {
            let map = AmqpValue::Map(
                vec![("key".into(), AmqpValue::String("value".into()))]
                    .into_iter()
                    .collect(),
            );
            let rust_amqp_value = RustAmqpValue { inner: map };
            let result = { amqpvalue_create_delivery_annotations(&rust_amqp_value) };
            assert!(!result.is_null());
            let result_value = { &*result };
            match &result_value.inner {
                AmqpValue::Described(described) => {
                    assert_eq!(described.descriptor, AmqpDescriptor::Code(0x71));
                    match &described.value {
                        AmqpValue::Map(map) => {
                            assert_eq!(map.get("key").unwrap(), &AmqpValue::String("value".into()));
                        }
                        _ => panic!("Expected AmqpValue::Map"),
                    }
                }
                _ => panic!("Expected AmqpValue::Described"),
            }
        }
    }

    #[test]
    fn test_amqpvalue_create_message_annotations() {
        unsafe {
            let map = AmqpValue::Map(
                vec![("key".into(), AmqpValue::String("value".into()))]
                    .into_iter()
                    .collect(),
            );
            let rust_amqp_value = RustAmqpValue { inner: map };
            let result = { amqpvalue_create_message_annotations(&rust_amqp_value) };
            assert!(!result.is_null());
            let result_value = { &*result };
            match &result_value.inner {
                AmqpValue::Described(described) => {
                    assert_eq!(described.descriptor, AmqpDescriptor::Code(0x72));
                    match &described.value {
                        AmqpValue::Map(map) => {
                            assert_eq!(map.get("key").unwrap(), &AmqpValue::String("value".into()));
                        }
                        _ => panic!("Expected AmqpValue::Map"),
                    }
                }
                _ => panic!("Expected AmqpValue::Described"),
            }
        }
    }

    #[test]
    fn test_amqpvalue_create_footer() {
        unsafe {
            let map = AmqpValue::Map(
                vec![("key".into(), AmqpValue::String("value".into()))]
                    .into_iter()
                    .collect(),
            );
            let rust_amqp_value = RustAmqpValue { inner: map };
            let result = { amqpvalue_create_footer(&rust_amqp_value) };
            assert!(!result.is_null());
            let result_value = { &*result };
            match &result_value.inner {
                AmqpValue::Described(described) => {
                    assert_eq!(described.descriptor, AmqpDescriptor::Code(0x78));
                    match &described.value {
                        AmqpValue::Map(map) => {
                            assert_eq!(map.get("key").unwrap(), &AmqpValue::String("value".into()));
                        }
                        _ => panic!("Expected AmqpValue::Map"),
                    }
                }
                _ => panic!("Expected AmqpValue::Described"),
            }
        }
    }

    #[test]
    fn test_amqpvalue_create_application_properties() {
        unsafe {
            let map = AmqpValue::Map(
                vec![("key".into(), AmqpValue::String("value".into()))]
                    .into_iter()
                    .collect(),
            );
            let rust_amqp_value = RustAmqpValue { inner: map };
            let result = { amqpvalue_create_application_properties(&rust_amqp_value) };
            assert!(!result.is_null());
            let result_value = { &*result };
            match &result_value.inner {
                AmqpValue::Described(described) => {
                    assert_eq!(described.descriptor, AmqpDescriptor::Code(0x74));
                    match &described.value {
                        AmqpValue::Map(map) => {
                            assert_eq!(map.get("key").unwrap(), &AmqpValue::String("value".into()));
                        }
                        _ => panic!("Expected AmqpValue::Map"),
                    }
                }
                _ => panic!("Expected AmqpValue::Described"),
            }
        }
    }
}
