// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words repr amqp amqpsession amqpmanagement

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    model::value::RustAmqpValue,
};
use azure_core::{auth::AccessToken, Result};
use azure_core_amqp::{
    management::{AmqpManagement, AmqpManagementApis},
    value::{AmqpOrderedMap, AmqpValue},
};
use std::time::Duration;
use std::{ffi::c_char, mem, ptr, time::UNIX_EPOCH};
use time::OffsetDateTime;
use tracing::{error, trace};

use super::session::RustAmqpSession;

#[repr(C)]
pub struct RustAccessToken {
    secret: *const c_char,
    expires_on: u64,
}

pub struct RustAmqpManagement {
    inner: AmqpManagement,
}

impl RustAmqpManagement {
    pub fn new(inner: AmqpManagement) -> Self {
        Self { inner }
    }
}

impl AmqpManagementApis for RustAmqpManagement {
    async fn attach(&self) -> Result<()> {
        self.inner.attach().await
    }
    async fn detach(self) -> Result<()> {
        self.inner.detach().await
    }

    async fn call(
        &self,
        operation_type: String,
        application_properties: AmqpOrderedMap<String, AmqpValue>,
    ) -> Result<AmqpOrderedMap<String, AmqpValue>> {
        self.inner
            .call(operation_type, application_properties)
            .await
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmanagement_create(
    call_context: *mut RustCallContext,
    session: *const RustAmqpSession,
    name: *const c_char,
    access_token: *const RustAccessToken,
) -> *mut RustAmqpManagement {
    let call_context = call_context_from_ptr_mut(call_context);
    let session = (*session).get_session();
    let name = std::ffi::CStr::from_ptr(name).to_str().unwrap();
    let rust_access_token: &RustAccessToken = &*access_token;
    let access_token: AccessToken = AccessToken::new(
        std::ffi::CStr::from_ptr(rust_access_token.secret)
            .to_str()
            .unwrap(),
        OffsetDateTime::from(UNIX_EPOCH + Duration::from_secs(rust_access_token.expires_on)),
    );

    let management = AmqpManagement::new(session.clone(), name.to_string(), access_token);
    match management {
        Ok(management) => Box::into_raw(Box::new(RustAmqpManagement::new(management))),
        Err(e) => {
            error!("Failed to create Management: {:?}", e);
            call_context.set_error(Box::new(e));
            ptr::null_mut()
        }
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmanagement_destroy(sender: *mut RustAmqpManagement) {
    if !sender.is_null() {
        trace!("Destroying Management");
        mem::drop(Box::from_raw(sender));
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmanagement_attach(
    call_context: *mut RustCallContext,
    management: *mut RustAmqpManagement,
) -> i32 {
    if call_context.is_null() || management.is_null() {
        error!("Invalid input");
        return -1;
    }

    let management = { &*management };
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to attach management");
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(management.inner.attach());
    trace!("Attached management");
    if let Err(err) = result {
        error!("Failed to attach management: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmanagement_detach_and_release(
    call_context: *mut RustCallContext,
    management: *mut RustAmqpManagement,
) -> i32 {
    if call_context.is_null() || management.is_null() {
        error!("Invalid input");
        return -1;
    }

    let management = Box::from_raw(management);
    let call_context = call_context_from_ptr_mut(call_context);
    trace!("Starting to detach management");
    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(management.detach());
    trace!("Detached management");
    if let Err(err) = result {
        error!("Failed to detach management: {:?}", err);
        call_context.set_error(Box::new(err));
        -1
    } else {
        0
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpmanagement_call(
    call_context: *mut RustCallContext,
    management: *mut RustAmqpManagement,
    operation_type: *const c_char,
    application_properties: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    if call_context.is_null() || management.is_null() {
        error!("Invalid input");
        return ptr::null_mut();
    }

    trace!("Starting to call management");
    let management = { &*management };
    let call_context = call_context_from_ptr_mut(call_context);
    let operation_type = std::ffi::CStr::from_ptr(operation_type).to_str().unwrap();
    let application_properties = { &*application_properties };
    if let AmqpValue::Map(application_properties) = &application_properties.inner {
        let application_properties = application_properties
            .iter()
            .map(|(key, value)| {
                let key = String::from(key);
                let value = value;
                (key, value)
            })
            .collect();
        trace!("Converted application properties.");
        let result = call_context.runtime_context().runtime().block_on(
            management
                .inner
                .call(operation_type.to_string(), application_properties),
        );
        if let Err(err) = result {
            error!("Failed to call management: {:?}", err);
            call_context.set_error(Box::new(err));
            ptr::null_mut()
        } else {
            let result = result.unwrap();
            trace!("Called Management: {:?}", result);
            Box::into_raw(Box::new(RustAmqpValue {
                inner: AmqpValue::Map(
                    result
                        .into_iter()
                        .map(|(key, value)| (AmqpValue::String(key), value))
                        .collect(),
                ),
            }))
        }
    } else {
        error!(
            "Application properties must be a map, found: {:?}",
            application_properties
        );
        ptr::null_mut()
    }
}
