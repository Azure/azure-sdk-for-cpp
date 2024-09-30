// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use crate::model::value::RustAmqpValue;
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

    //    pub(crate) fn get(&self) -> &AmqpSource {
    //        &self.inner
    //    }
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
extern "C" fn source_destroy(source: *mut RustAmqpSource) {
    unsafe {
        std::mem::drop(Box::from_raw(source));
    }
}

#[no_mangle]
extern "C" fn source_clone(source: *const RustAmqpSource) -> *mut RustAmqpSource {
    let source = unsafe { &*source };
    Box::into_raw(Box::new(RustAmqpSource::new(source.inner.clone())))
}

#[no_mangle]
extern "C" fn source_get_address(
    source: *const RustAmqpSource,
    address: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_durable(
    source: *const RustAmqpSource,
    durable: &mut RustTerminusDurability,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_expiry_policy(
    source: *const RustAmqpSource,
    expiry_policy: &mut RustExpiryPolicy,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_timeout(source: *const RustAmqpSource, timeout: &mut u32) -> u32 {
    let source = unsafe { &*source };
    if let Some(timeout_value) = source.inner.timeout {
        *timeout = timeout_value;
        0
    } else {
        *timeout = 0;
        0
    }
}

#[no_mangle]
extern "C" fn source_get_dynamic(source: *const RustAmqpSource, dynamic: &mut bool) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_distribution_mode(
    source: *const RustAmqpSource,
    distribution_mode: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_filter(
    source: *const RustAmqpSource,
    filter: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_default_outcome(
    source: *const RustAmqpSource,
    default_outcome: &mut RustDeliveryOutcome,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_dynamic_node_properties(
    source: *const RustAmqpSource,
    properties: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_outcomes(
    source: *const RustAmqpSource,
    outcomes: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn source_get_capabilities(
    source: *const RustAmqpSource,
    capabilities: &mut *mut RustAmqpValue,
) -> u32 {
    let source = unsafe { &*source };
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
extern "C" fn amqpvalue_create_source(
    source: *const RustAmqpSource,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let source = unsafe { &*source };
    let inner_value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x28),
        source.inner.clone(),
    )));
    unsafe { *value = Box::into_raw(Box::new(RustAmqpValue { inner: inner_value })) };
    0
}

#[no_mangle]
extern "C" fn amqpvalue_get_source(
    value: *const RustAmqpValue,
    source: &mut *mut RustAmqpSource,
) -> i32 {
    let value = unsafe { &*value };
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
extern "C" fn source_builder_destroy(builder: *mut RustAmqpSourceBuilder) {
    unsafe {
        std::mem::drop(Box::from_raw(builder));
    }
}

#[no_mangle]
extern "C" fn source_builder_build(
    builder: *mut RustAmqpSourceBuilder,
    source: &mut *mut RustAmqpSource,
) -> u32 {
    let builder = unsafe { &mut *builder };
    *source = Box::into_raw(Box::new(RustAmqpSource::new(builder.inner.build())));
    0
}

#[no_mangle]
extern "C" fn source_set_address(
    builder: *mut RustAmqpSourceBuilder,
    address: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let address = unsafe { &*address };
    match &address.inner {
        AmqpValue::String(address) => {
            builder.inner.with_address(address.clone());
            0
        }
        _ => {
            warn!("Invalid address type: {:?}", address.inner);
            1
        }
    }
}

#[no_mangle]
extern "C" fn source_set_durable(
    builder: *mut RustAmqpSourceBuilder,
    durable: RustTerminusDurability,
) -> u32 {
    let builder = unsafe { &mut *builder };
    match durable {
        RustTerminusDurability::None => {
            builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::None);
            0
        }
        RustTerminusDurability::Configuration => {
            builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::Configuration);
            0
        }
        RustTerminusDurability::UnsettledState => {
            builder
                .inner
                .with_durable(azure_core_amqp::messaging::TerminusDurability::UnsettledState);
            0
        }
    }
}

#[no_mangle]
extern "C" fn source_set_expiry_policy(
    builder: *mut RustAmqpSourceBuilder,
    expiry_policy: RustExpiryPolicy,
) -> u32 {
    let builder = unsafe { &mut *builder };
    match expiry_policy {
        RustExpiryPolicy::LinkDetach => {
            builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::LinkDetach);
            0
        }
        RustExpiryPolicy::SessionEnd => {
            builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::SessionEnd);
            0
        }
        RustExpiryPolicy::ConnectionClose => {
            builder.inner.with_expiry_policy(
                azure_core_amqp::messaging::TerminusExpiryPolicy::ConnectionClose,
            );
            0
        }
        RustExpiryPolicy::Never => {
            builder
                .inner
                .with_expiry_policy(azure_core_amqp::messaging::TerminusExpiryPolicy::Never);
            0
        }
    }
}

#[no_mangle]
extern "C" fn source_set_timeout(builder: *mut RustAmqpSourceBuilder, timeout: u32) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_timeout(timeout);
    0
}

#[no_mangle]
extern "C" fn source_set_dynamic(builder: *mut RustAmqpSourceBuilder, dynamic: bool) -> u32 {
    let builder = unsafe { &mut *builder };
    builder.inner.with_dynamic(dynamic);
    0
}

#[no_mangle]
extern "C" fn source_set_distribution_mode(
    builder: *mut RustAmqpSourceBuilder,
    distribution_mode: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let distribution_mode = unsafe { &*distribution_mode };
    match &distribution_mode.inner {
        AmqpValue::Symbol(s) => match s.0.as_str() {
            "move" => {
                builder
                    .inner
                    .with_distribution_mode(azure_core_amqp::messaging::DistributionMode::Move);
                0
            }
            "copy" => {
                builder
                    .inner
                    .with_distribution_mode(azure_core_amqp::messaging::DistributionMode::Copy);
                0
            }
            _ => {
                warn!("Invalid distribution mode: {}", s.0);
                1
            }
        },
        _ => {
            warn!(
                "Invalid distribution mode type: {:?}",
                distribution_mode.inner
            );
            1
        }
    }
}

#[no_mangle]
extern "C" fn source_set_filter(
    builder: *mut RustAmqpSourceBuilder,
    filter: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let filter = unsafe { &*filter };
    match &filter.inner {
        AmqpValue::Map(filter) => {
            let map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = filter
                .clone()
                .into_iter()
                .map(|f| (f.0.into(), f.1.clone()))
                .collect();
            builder.inner.with_filter(map);
            0
        }
        _ => {
            warn!("Invalid filter type: {:?}", filter.inner);
            1
        }
    }
}

#[no_mangle]
extern "C" fn source_set_default_outcome(
    builder: *mut RustAmqpSourceBuilder,
    default_outcome: RustDeliveryOutcome,
) -> u32 {
    let builder = unsafe { &mut *builder };
    match default_outcome {
        RustDeliveryOutcome::Accepted => {
            builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Accepted);
            0
        }
        RustDeliveryOutcome::Rejected => {
            builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Rejected);
            0
        }
        RustDeliveryOutcome::Released => {
            builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Released);
            0
        }
        RustDeliveryOutcome::Modified => {
            builder
                .inner
                .with_default_outcome(azure_core_amqp::messaging::AmqpOutcome::Modified);
            0
        }
    }
}

#[no_mangle]
extern "C" fn source_set_dynamic_node_properties(
    builder: *mut RustAmqpSourceBuilder,
    properties: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let properties = unsafe { &*properties };
    match &properties.inner {
        AmqpValue::Map(properties) => {
            let map: AmqpOrderedMap<AmqpSymbol, AmqpValue> = properties
                .clone()
                .into_iter()
                .map(|f| {
                    (
                        match f.0 {
                            AmqpValue::Symbol(f) => f,
                            _ => {
                                warn!("Invalid dynamic node properties key: {:?}", f.0);
                                AmqpSymbol("invalid".to_string())
                            }
                        },
                        f.1.clone(),
                    )
                })
                .collect();
            builder.inner.with_dynamic_node_properties(map);
            0
        }
        _ => {
            warn!(
                "Invalid dynamic node properties type: {:?}",
                properties.inner
            );
            1
        }
    }
}

#[no_mangle]
extern "C" fn source_set_outcomes(
    builder: *mut RustAmqpSourceBuilder,
    outcomes: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let outcomes = unsafe { &*outcomes };
    match &outcomes.inner {
        AmqpValue::List(outcomes) => {
            let list: Vec<azure_core_amqp::value::AmqpSymbol> = outcomes
                .0
                .clone()
                .into_iter()
                .map(|f| match f {
                    AmqpValue::Symbol(f) => f.clone(),
                    _ => {
                        warn!("Invalid outcome type: {:?}", f);
                        AmqpSymbol("invalid".to_string())
                    }
                })
                .collect();
            builder.inner.with_outcomes(list);
            0
        }
        AmqpValue::Array(outcomes) => {
            let list: Vec<azure_core_amqp::value::AmqpSymbol> = outcomes
                .clone()
                .into_iter()
                .map(|f| match f {
                    AmqpValue::Symbol(f) => f.clone(),
                    _ => {
                        warn!("Invalid outcome type: {:?}", f);
                        AmqpSymbol("invalid".to_string())
                    }
                })
                .collect();
            builder.inner.with_outcomes(list);
            0
        }
        _ => {
            warn!("Invalid outcomes type: {:?}", outcomes.inner);
            1
        }
    }
}

#[no_mangle]
extern "C" fn source_set_capabilities(
    builder: *mut RustAmqpSourceBuilder,
    capabilities: *const RustAmqpValue,
) -> u32 {
    let builder = unsafe { &mut *builder };
    let capabilities = unsafe { &*capabilities };
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
            builder.inner.with_capabilities(list);
            0
        }
        _ => {
            warn!("Invalid capabilities type: {:?}", capabilities.inner);
            1
        }
    }
}
