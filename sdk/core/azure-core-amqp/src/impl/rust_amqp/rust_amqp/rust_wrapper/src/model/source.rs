// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use std::ffi::{c_char, CStr};

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str,
    model::value::RustAmqpValue,
};
use azure_core_amqp::{
    messaging::{builders::AmqpSourceBuilder, AmqpSource},
    value::{AmqpComposite, AmqpDescriptor, AmqpOrderedMap, AmqpSymbol, AmqpValue},
};
use tracing::warn;

pub struct RustAmqpSource {
    inner: AmqpSource,
}

impl RustAmqpSource {
    fn new(source: AmqpSource) -> Self {
        Self { inner: source }
    }

    pub(crate) fn get(&self) -> &AmqpSource {
        &self.inner
    }
}

#[repr(C)]
pub enum RustTerminusDurability {
    None,
    Configuration,
    UnsettledState,
}

#[repr(C)]
pub enum RustExpiryPolicy {
    LinkDetach,
    SessionEnd,
    ConnectionClose,
    Never,
}

#[repr(C)]
pub enum RustDeliveryOutcome {
    Accepted,
    Rejected,
    Released,
    Modified,
}

#[no_mangle]
unsafe extern "C" fn source_destroy(source: *mut RustAmqpSource) {
    std::mem::drop(Box::from_raw(source));
}

#[no_mangle]
unsafe extern "C" fn source_clone(source: *const RustAmqpSource) -> *mut RustAmqpSource {
    let source = &*source;
    Box::into_raw(Box::new(RustAmqpSource::new(source.inner.clone())))
}

#[no_mangle]
unsafe extern "C" fn source_get_address(
    source: *const RustAmqpSource,
    address: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(source_address) = &source.inner.address {
        *address = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::String(source_address.clone()),
        }));
        0
    } else {
        *address = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_durable(
    source: *const RustAmqpSource,
    durable: &mut RustTerminusDurability,
) -> i32 {
    let source = &*source;
    if let Some(durable_value) = &source.inner.durable {
        match durable_value {
            azure_core_amqp::messaging::TerminusDurability::None => {
                *durable = RustTerminusDurability::None
            }
            azure_core_amqp::messaging::TerminusDurability::Configuration => {
                *durable = RustTerminusDurability::Configuration
            }
            azure_core_amqp::messaging::TerminusDurability::UnsettledState => {
                *durable = RustTerminusDurability::UnsettledState
            }
        }
        0
    } else {
        *durable = RustTerminusDurability::None;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_expiry_policy(
    source: *const RustAmqpSource,
    expiry_policy: &mut RustExpiryPolicy,
) -> i32 {
    let source = &*source;
    if let Some(expiry_policy_value) = &source.inner.expiry_policy {
        match expiry_policy_value {
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
        *expiry_policy = RustExpiryPolicy::SessionEnd;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_timeout(source: *const RustAmqpSource, timeout: &mut u32) -> i32 {
    let source = &*source;
    if let Some(timeout_value) = source.inner.timeout {
        *timeout = timeout_value;
        0
    } else {
        *timeout = 0;
        0
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_dynamic(source: *const RustAmqpSource, dynamic: &mut bool) -> i32 {
    let source = &*source;
    if let Some(dynamic_value) = source.inner.dynamic {
        *dynamic = dynamic_value;
        0
    } else {
        *dynamic = false;
        0
    }
}

// Returns 0 == DistributionMode::Move
// Returns 1 == DistributionMode::Copy
#[no_mangle]
unsafe extern "C" fn source_get_distribution_mode(
    source: *const RustAmqpSource,
    distribution_mode: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(distribution_mode_value) = &source.inner.distribution_mode {
        match distribution_mode_value {
            azure_core_amqp::messaging::DistributionMode::Move => {
                *distribution_mode = Box::into_raw(Box::new(RustAmqpValue {
                    inner: AmqpValue::Symbol(AmqpSymbol("move".to_string())),
                }));
            }
            azure_core_amqp::messaging::DistributionMode::Copy => {
                *distribution_mode = Box::into_raw(Box::new(RustAmqpValue {
                    inner: AmqpValue::Symbol(AmqpSymbol("copy".to_string())),
                }));
            }
        }
        0
    } else {
        *distribution_mode = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_filter(
    source: *const RustAmqpSource,
    filter: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(source_filter) = &source.inner.filter {
        let map: AmqpOrderedMap<AmqpValue, AmqpValue> = source_filter
            .clone()
            .into_iter()
            .map(|f| (AmqpValue::from(f.0), f.1))
            .collect();
        *filter = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Map(map),
        }));
        0
    } else {
        *filter = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_default_outcome(
    source: *const RustAmqpSource,
    default_outcome: &mut RustDeliveryOutcome,
) -> i32 {
    let source = &*source;
    if let Some(source_default_outcome) = &source.inner.default_outcome {
        match source_default_outcome {
            azure_core_amqp::messaging::AmqpOutcome::Accepted => {
                *default_outcome = RustDeliveryOutcome::Accepted;
            }
            azure_core_amqp::messaging::AmqpOutcome::Rejected => {
                *default_outcome = RustDeliveryOutcome::Rejected;
            }
            azure_core_amqp::messaging::AmqpOutcome::Released => {
                *default_outcome = RustDeliveryOutcome::Released;
            }
            azure_core_amqp::messaging::AmqpOutcome::Modified => {
                *default_outcome = RustDeliveryOutcome::Modified;
            }
        }
        0
    } else {
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_dynamic_node_properties(
    source: *const RustAmqpSource,
    properties: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(source_properties) = &source.inner.dynamic_node_properties {
        let map: AmqpOrderedMap<AmqpValue, AmqpValue> = source_properties
            .clone()
            .into_iter()
            .map(|f| (AmqpValue::from(f.0), f.1))
            .collect();
        *properties = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Map(map),
        }));
        0
    } else {
        *properties = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_outcomes(
    source: *const RustAmqpSource,
    outcomes: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(source_outcomes) = &source.inner.outcomes {
        let list: Vec<AmqpValue> = source_outcomes
            .clone()
            .into_iter()
            .map(AmqpValue::from)
            .collect();
        *outcomes = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Array(list),
        }));
        0
    } else {
        *outcomes = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn source_get_capabilities(
    source: *const RustAmqpSource,
    capabilities: &mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    if let Some(source_capabilities) = &source.inner.capabilities {
        let list: Vec<AmqpValue> = source_capabilities
            .clone()
            .into_iter()
            .map(AmqpValue::from)
            .collect();
        *capabilities = Box::into_raw(Box::new(RustAmqpValue {
            inner: AmqpValue::Array(list),
        }));
        0
    } else {
        *capabilities = std::ptr::null_mut();
        1
    }
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_create_source(
    source: *const RustAmqpSource,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let source = &*source;
    let inner_value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x28),
        source.inner.clone(),
    )));
    *value = Box::into_raw(Box::new(RustAmqpValue { inner: inner_value }));
    0
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_get_source(
    value: *const RustAmqpValue,
    source: &mut *mut RustAmqpSource,
) -> i32 {
    let value = &*value;
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x28) => {
                let h = value.value();
                match h {
                    AmqpValue::List(c) => {
                        *source = Box::into_raw(Box::new(RustAmqpSource {
                            inner: AmqpSource::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *source = std::ptr::null_mut();
                        println!("Unexpected source value: {:?}", value);
                        1
                    }
                }
            }
            _ => {
                println!("Unexpected source descriptor code: {:?}", value);
                *source = std::ptr::null_mut();
                1
            }
        },
        AmqpValue::Composite(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x28) => {
                let h = value.value();
                *source = Box::into_raw(Box::new(RustAmqpSource {
                    inner: AmqpSource::from(h.clone()),
                }));
                0
            }
            _ => {
                println!("Unexpected source descriptor code: {:?}", value);
                *source = std::ptr::null_mut();
                1
            }
        },
        _ => {
            println!("Unexpected source type: {:?}", value.inner);
            1
        }
    }
}

pub struct RustAmqpSourceBuilder {
    inner: AmqpSourceBuilder,
}

#[no_mangle]
extern "C" fn source_builder_create() -> *mut RustAmqpSourceBuilder {
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
        inner: AmqpSource::builder(),
    }))
}

#[no_mangle]
unsafe extern "C" fn source_builder_destroy(builder: *mut RustAmqpSourceBuilder) {
    std::mem::drop(Box::from_raw(builder));
}

#[no_mangle]
unsafe extern "C" fn source_builder_build(
    builder: *mut RustAmqpSourceBuilder,
    source: &mut *mut RustAmqpSource,
) -> i32 {
    let builder = &mut *builder;
    *source = Box::into_raw(Box::new(RustAmqpSource::new(builder.inner.build())));
    0
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_address(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    address: *const c_char,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let address = CStr::from_ptr(address).to_str();
    match address {
        Ok(address) => Box::into_raw(Box::new(RustAmqpSourceBuilder {
            inner: builder.inner.with_address(address.to_string()),
        })),
        Err(err) => {
            call_context.set_error(Box::new(err));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_durable(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    durable: RustTerminusDurability,
) -> *mut RustAmqpSourceBuilder {
    let _call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
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
unsafe extern "C" fn source_builder_set_expiry_policy(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    expiry_policy: RustExpiryPolicy,
) -> *mut RustAmqpSourceBuilder {
    let _call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
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
unsafe extern "C" fn source_builder_set_timeout(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    timeout: u32,
) -> *mut RustAmqpSourceBuilder {
    let _call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
        inner: builder.inner.with_timeout(timeout),
    }))
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_dynamic(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    dynamic: bool,
) -> *mut RustAmqpSourceBuilder {
    let _call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
        inner: builder.inner.with_dynamic(dynamic),
    }))
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_distribution_mode(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    distribution_mode: *const RustAmqpValue,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let distribution_mode = &*distribution_mode;
    match &distribution_mode.inner {
        AmqpValue::Symbol(s) => match s.0.as_str() {
            "move" => Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder
                    .inner
                    .with_distribution_mode(azure_core_amqp::messaging::DistributionMode::Move),
            })),
            "copy" => Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder
                    .inner
                    .with_distribution_mode(azure_core_amqp::messaging::DistributionMode::Copy),
            })),
            _ => {
                warn!("Invalid distribution mode: {}", s.0);
                call_context.set_error(error_from_str("Invalid distribution mode"));
                std::ptr::null_mut()
            }
        },
        _ => {
            warn!(
                "Invalid distribution mode type: {:?}",
                distribution_mode.inner
            );
            call_context.set_error(error_from_str("Distribution mode must be a symbol."));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_filter(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    filter: *const RustAmqpValue,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let filter = &*filter;
    match &filter.inner {
        AmqpValue::Map(filter) => {
            let map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = filter
                .clone()
                .into_iter()
                .map(|f| (f.0.into(), f.1.clone()))
                .collect();
            Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder.inner.with_filter(map),
            }))
        }
        _ => {
            warn!("Invalid filter type: {:?}", filter.inner);
            call_context.set_error(error_from_str("Filter type must be a map."));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_default_outcome(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    default_outcome: RustDeliveryOutcome,
) -> *mut RustAmqpSourceBuilder {
    let _call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    Box::into_raw(Box::new(RustAmqpSourceBuilder {
        inner: match default_outcome {
            RustDeliveryOutcome::Accepted => builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Accepted),
            RustDeliveryOutcome::Rejected => builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Rejected),
            RustDeliveryOutcome::Released => builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Released),
            RustDeliveryOutcome::Modified => builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Modified),
        },
    }))
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_dynamic_node_properties(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    properties: *const RustAmqpValue,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let properties = &*properties;
    match &properties.inner {
        AmqpValue::Map(properties) => {
            let map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
                .clone()
                .into_iter()
                .map(|f| (f.0.into(), f.1))
                .collect();
            Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder.inner.with_dynamic_node_properties(map),
            }))
        }
        _ => {
            warn!(
                "Invalid dynamic node properties type: {:?}",
                properties.inner
            );
            call_context.set_error(error_from_str("Dynamic node properties must be a map."));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_outcomes(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    outcomes: *const RustAmqpValue,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let outcomes = &*outcomes;
    match &outcomes.inner {
        AmqpValue::List(outcomes) => {
            let list: Vec<azure_core_amqp::value::AmqpSymbol> = outcomes
                .0
                .clone()
                .into_iter()
                .map(|f| match f {
                    AmqpValue::Symbol(f) => f,
                    _ => {
                        warn!("Invalid outcome type: {:?}", f);
                        AmqpSymbol("invalid".to_string())
                    }
                })
                .collect();
            Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder.inner.with_outcomes(list),
            }))
        }
        AmqpValue::Array(outcomes) => {
            let list: Vec<azure_core_amqp::value::AmqpSymbol> = outcomes
                .clone()
                .into_iter()
                .map(|f| match f {
                    AmqpValue::Symbol(f) => f,
                    _ => {
                        warn!("Invalid outcome type: {:?}", f);
                        AmqpSymbol("invalid".to_string())
                    }
                })
                .collect();
            Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder.inner.with_outcomes(list),
            }))
        }
        _ => {
            call_context.set_error(error_from_str("Outcomes must be an array or a list."));
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
unsafe extern "C" fn source_builder_set_capabilities(
    call_context: *mut RustCallContext,
    builder: *mut RustAmqpSourceBuilder,
    capabilities: *const RustAmqpValue,
) -> *mut RustAmqpSourceBuilder {
    let call_context = call_context_from_ptr_mut(call_context);
    let builder = Box::from_raw(builder);
    let capabilities = &*capabilities;
    match &capabilities.inner {
        AmqpValue::Array(capabilities) => {
            let list: Vec<AmqpSymbol> = capabilities
                .clone()
                .into_iter()
                .map(|f| match f {
                    AmqpValue::Symbol(f) => f.clone(),
                    _ => {
                        warn!("Invalid capability type: {:?}", f);
                        AmqpSymbol("invalid".to_string())
                    }
                })
                .collect();
            Box::into_raw(Box::new(RustAmqpSourceBuilder {
                inner: builder.inner.with_capabilities(list),
            }))
        }
        _ => {
            warn!("Invalid capabilities type: {:?}", capabilities.inner);
            call_context.set_error(error_from_str("Capabilities must be an array."));
            std::ptr::null_mut()
        }
    }
}
