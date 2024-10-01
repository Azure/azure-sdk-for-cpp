// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words amqp amqpclaimsbasedsecurity amqpsession

use crate::{
    amqp::session::RustAmqpSession,
    call_context::{call_context_from_ptr_mut, RustCallContext},
};
use azure_core_amqp::cbs::{AmqpClaimsBasedSecurity, AmqpClaimsBasedSecurityApis};
use std::{
    ffi::c_char,
    mem,
    time::{Duration, UNIX_EPOCH},
};
use tracing::{error, trace};

pub struct RustAmqpClaimsBasedSecurity {
    inner: AmqpClaimsBasedSecurity,
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpclaimsbasedsecurity_create(
    call_context: *mut RustCallContext,
    session: *const RustAmqpSession,
    claims_based_security: *mut *mut RustAmqpClaimsBasedSecurity,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let session = unsafe { (*session).get_session() };
    let cbs = AmqpClaimsBasedSecurity::new(session.clone());
    match cbs {
        Ok(cbs) => {
            *claims_based_security =
                Box::into_raw(Box::new(RustAmqpClaimsBasedSecurity { inner: cbs }));
            0
        }
        Err(e) => {
            error!("Failed to create CBS: {:?}", e);
            call_context.set_error(Box::new(e));
            -1
        }
    }
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn amqpclaimsbasedsecurity_destroy(cbs: *mut RustAmqpClaimsBasedSecurity) {
    unsafe {
        trace!("Destroying CBS");
        mem::drop(Box::from_raw(cbs));
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpclaimsbasedsecurity_attach(
    call_context: *mut RustCallContext,
    cbs: *mut RustAmqpClaimsBasedSecurity,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    if cbs.is_null() {
        call_context.set_error(Box::new(azure_core::Error::new(
            azure_core::error::ErrorKind::Other,
            "CBS is null",
        )));
        return -1;
    }
    let cbs = &mut (*cbs).inner;

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(cbs.attach());

    if let Err(e) = result {
        error!("Failed to attach CBS: {:?}", e);
        call_context.set_error(Box::new(e));
        -1
    } else {
        0
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn amqpclaimsbasedsecurity_authorize_path(
    call_context: *mut RustCallContext,
    cbs: *mut RustAmqpClaimsBasedSecurity,
    path: *const c_char,
    secret: *const c_char,
    expires_on: u64,
) -> i32 {
    let call_context = call_context_from_ptr_mut(call_context);
    let cbs = &mut (*cbs).inner;
    let path = unsafe { std::ffi::CStr::from_ptr(path).to_str().unwrap() };
    let secret = unsafe { std::ffi::CStr::from_ptr(secret).to_str().unwrap() };
    let expires_on = UNIX_EPOCH + Duration::from_secs(expires_on);
    let expires_on = time::OffsetDateTime::from(expires_on);

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(cbs.authorize_path(path, secret, expires_on));
    if let Err(e) = result {
        error!("Failed to authorize path: {:?}", e);
        call_context.set_error(Box::new(e));
        -1
    } else {
        0
    }
}
