// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use std::ffi::c_char;

use azure_core_amqp::{
    messaging::{builders::AmqpTargetBuilder, AmqpTarget},
    value::{AmqpDescribed, AmqpDescriptor, AmqpList, AmqpOrderedMap, AmqpValue},
};

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str,
    model::{
        source::{RustExpiryPolicy, RustTerminusDurability},
        value::RustAmqpValue,
    },
};
use tracing::warn;

pub struct RustAmqpTarget {
    inner: AmqpTarget,
}

impl RustAmqpTarget {
    fn new(target: AmqpTarget) -> Self {
        Self { inner: target }
    }

    pub(crate) fn get(&self) -> &AmqpTarget {
        &self.inner
    }
}

#[no_mangle]
unsafe extern "C" fn target_destroy(source: *mut RustAmqpTarget) {
    std::mem::drop(Box::from_raw(source));
}

#[no_mangle]
unsafe extern "C" fn target_clone(target: *const RustAmqpTarget) -> *mut RustAmqpTarget {
    let target = &*target;
    Box::into_raw(Box::new(RustAmqpTarget::new(target.inner.clone())))
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_create_target(target: *const RustAmqpTarget) -> *mut RustAmqpValue {
    let target = &*target;
    Box::into_raw(Box::new(RustAmqpValue {
        inner: AmqpValue::Described(Box::new(AmqpDescribed::new(
            AmqpDescriptor::Code(0x29),
            AmqpValue::List(AmqpList::from(target.inner.clone())),
        ))),
    }))
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_get_target(
    value: *const RustAmqpValue,
    target: *mut *mut RustAmqpTarget,
) -> i32 {
    let value = &*value;
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x29) => {
                let h = value.value();
                match h {
                    AmqpValue::List(c) => {
                        *target = Box::into_raw(Box::new(RustAmqpTarget {
                            inner: AmqpTarget::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *target = std::ptr::null_mut();
                        println!("Unexpected target value: {:?}", value);
                        1
                    }
                }
            }
            _ => {
                println!("Unexpected target descriptor code: {:?}", value);
                *target = std::ptr::null_mut();
                1
            }
        },
        _ => {
            println!("Unexpected source type: {:?}", value.inner);
            1
        }
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_address(
    target: *const RustAmqpTarget,
    address: &mut *mut RustAmqpValue,
) -> i32 {
    let target = &*target;
    if let Some(s) = target.inner.address() {
        *address = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::String(s.clone()),
        }));
        0
    } else {
        println!("No address found: {:?}", target.inner);
        *address = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_durable(
    target: *const RustAmqpTarget,
    durable: &mut RustTerminusDurability,
) -> i32 {
    let target = &*target;
    if let Some(durability) = target.inner.durable() {
        match durability {
            azure_core_amqp::messaging::TerminusDurability::None => {
                *durable = RustTerminusDurability::None;
            }
            azure_core_amqp::messaging::TerminusDurability::Configuration => {
                *durable = RustTerminusDurability::Configuration;
            }
            azure_core_amqp::messaging::TerminusDurability::UnsettledState => {
                *durable = RustTerminusDurability::UnsettledState;
            }
        }
        0
    } else {
        // If durable isn't set, we still return a value, but it's None
        *durable = RustTerminusDurability::None;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_expiry_policy(
    target: *const RustAmqpTarget,
    expiry_policy: &mut RustExpiryPolicy,
) -> i32 {
    let target = &*target;
    if let Some(policy) = target.inner.expiry_policy() {
        match policy {
            azure_core_amqp::messaging::TerminusExpiryPolicy::LinkDetach => {
                *expiry_policy = RustExpiryPolicy::LinkDetach;
            }
            azure_core_amqp::messaging::TerminusExpiryPolicy::SessionEnd => {
                *expiry_policy = RustExpiryPolicy::SessionEnd;
            }
            azure_core_amqp::messaging::TerminusExpiryPolicy::ConnectionClose => {
                *expiry_policy = RustExpiryPolicy::ConnectionClose;
            }
            azure_core_amqp::messaging::TerminusExpiryPolicy::Never => {
                *expiry_policy = RustExpiryPolicy::Never;
            }
        }
        0
    } else {
        // If expiry_policy isn't set, we still return a value, but it's Never
        *expiry_policy = RustExpiryPolicy::SessionEnd;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_timeout(target: *const RustAmqpTarget, timeout: *mut u32) -> i32 {
    let target = &*target;
    if let Some(timeout_val) = target.inner.timeout() {
        *timeout = *timeout_val;
        return 0;
    }
    1
}

#[no_mangle]
unsafe extern "C" fn target_get_dynamic(target: *const RustAmqpTarget, dynamic: *mut bool) -> i32 {
    let target = &*target;
    if let Some(dynamic_val) = target.inner.dynamic() {
        *dynamic = *dynamic_val;
        0
    } else {
        *dynamic = false;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_capabilities(
    target: *const RustAmqpTarget,
    capabilities: &mut *mut RustAmqpValue,
) -> i32 {
    let target = &*target;
    if let Some(c) = target.inner.capabilities() {
        *capabilities = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Array(c.clone()),
        }));
        0
    } else {
        *capabilities = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn target_get_dynamic_node_properties(
    target: *const RustAmqpTarget,
    properties: &mut *mut RustAmqpValue,
) -> i32 {
    let target = &*target;
    if let Some(p) = target.inner.dynamic_node_properties() {
        let p: AmqpOrderedMap<AmqpValue, AmqpValue> = p
            .clone()
            .into_iter()
            .map(|(k, v)| (AmqpValue::String(k.clone()), v.clone()))
            .collect();
        *properties = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Map(p),
        }));
        0
    } else {
        *properties = std::ptr::null_mut();
        1
    }
}

pub struct RustAmqpTargetBuilder {
    inner: AmqpTargetBuilder,
}

#[no_mangle]
unsafe extern "C" fn target_builder_create() -> *mut RustAmqpTargetBuilder {
    Box::into_raw(Box::new(RustAmqpTargetBuilder {
        inner: AmqpTarget::builder(),
    }))
}

#[no_mangle]
unsafe extern "C" fn target_builder_destroy(builder: *mut RustAmqpTargetBuilder) {
    std::mem::drop(Box::from_raw(builder));
}

#[no_mangle]
unsafe extern "C" fn target_builder_build(
    builder: *mut RustAmqpTargetBuilder,
    target: &mut *mut RustAmqpTarget,
) -> i32 {
    let builder = &mut *builder;
    *target = Box::into_raw(Box::new(RustAmqpTarget::new(builder.inner.build())));
    0
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_address(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    address: *const c_char,
) -> *mut RustAmqpTargetBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let address = std::ffi::CStr::from_ptr(address).to_str();
    match address {
        Ok(address) => Box::into_raw(Box::new(RustAmqpTargetBuilder {
            inner: builder.inner.with_address(address.to_string()),
        })),
        Err(e) => {
            warn!("Failed to convert address to string: {:?}", e);
            call_context.set_error(Box::new(e));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_durable(
    _call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    durable: RustTerminusDurability,
) -> *mut RustAmqpTargetBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpTargetBuilder {
        inner: match durable {
            RustTerminusDurability::None => builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::None),
            RustTerminusDurability::Configuration => builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::Configuration),
            RustTerminusDurability::UnsettledState => builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::UnsettledState),
        },
    }))
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_expiry_policy(
    _call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    expiry_policy: RustExpiryPolicy,
) -> *mut RustAmqpTargetBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpTargetBuilder {
        inner: match expiry_policy {
            RustExpiryPolicy::LinkDetach => builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::LinkDetach),
            RustExpiryPolicy::SessionEnd => builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::SessionEnd),
            RustExpiryPolicy::ConnectionClose => builder.inner.with_expiry_policy(
                azure_core_amqp::messaging::TerminusExpiryPolicy::ConnectionClose,
            ),
            RustExpiryPolicy::Never => builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::Never),
        },
    }))
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_timeout(
    _call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    timeout: u32,
) -> *mut RustAmqpTargetBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpTargetBuilder {
        inner: builder.inner.with_timeout(timeout),
    }))
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_dynamic(
    _call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    dynamic: bool,
) -> *mut RustAmqpTargetBuilder {
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpTargetBuilder {
        inner: builder.inner.with_dynamic(dynamic),
    }))
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_capabilities(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    capabilities: *const RustAmqpValue,
) -> *mut RustAmqpTargetBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let capabilities = &*capabilities;
    match &capabilities.inner {
        AmqpValue::Array(value) => Box::into_raw(Box::new(RustAmqpTargetBuilder {
            inner: builder.inner.with_capabilities(value.clone()),
        })),
        _ => {
            warn!("Expected array value for capabilities");
            call_context.set_error(error_from_str("Expected array value for capabilities"));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn target_builder_set_dynamic_node_properties(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpTargetBuilder,
    properties: *const RustAmqpValue,
) -> *mut RustAmqpTargetBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let properties = &*properties;
    match &properties.inner {
        AmqpValue::Map(value) => {
            let value: AmqpOrderedMap<String, AmqpValue> = value
                .clone()
                .into_iter()
                .map(|(k, v)| {
                    (
                        match k {
                            AmqpValue::String(s) => s,
                            _ => panic!("Expected string value in map"),
                        },
                        v.clone(),
                    )
                })
                .collect();
            Box::into_raw(Box::new(RustAmqpTargetBuilder {
                inner: builder.inner.with_dynamic_node_properties(value.clone()),
            }))
        }
        _ => {
            warn!(
                "Expected map value for dynamic node properties, found: {:?}",
                properties.inner
            );
            call_context.set_error(error_from_str(
                "Expected map value for dynamic node properties",
            ));
            std::ptr::null_mut()
        }
    }
}
