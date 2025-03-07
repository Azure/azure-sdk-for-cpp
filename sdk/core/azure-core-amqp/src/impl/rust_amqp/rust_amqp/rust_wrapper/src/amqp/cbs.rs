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

pub struct RustAmqpClaimsBasedSecurity<'a> {
    inner: AmqpClaimsBasedSecurity<'a>,
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
    let session = (*session).get_session();
    let cbs = AmqpClaimsBasedSecurity::new(session);
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
    trace!("Destroying CBS");
    mem::drop(Box::from_raw(cbs));
}

#[no_mangle]
/// Attach an AMQP Claims Based Security (CBS) node.
///
/// # Safety
///
/// This function is unsafe because it dereferences raw pointers.
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
pub unsafe extern "C" fn amqpclaimsbasedsecurity_detach_and_release(
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
    let cbs = Box::from_raw(cbs);

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(cbs.inner.detach());

    if let Err(e) = result {
        error!("Failed to detach CBS: {:?}", e);
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
    let path = std::ffi::CStr::from_ptr(path).to_str().unwrap();
    let secret = std::ffi::CStr::from_ptr(secret).to_str().unwrap();
    let expires_on = UNIX_EPOCH + Duration::from_secs(expires_on);
    let expires_on = time::OffsetDateTime::from(expires_on);

    let result = call_context
        .runtime_context()
        .runtime()
        .block_on(cbs.authorize_path(path.to_string(), None, secret.to_string(), expires_on));
    if let Err(e) = result {
        error!("Failed to authorize path: {:?}", e);
        call_context.set_error(Box::new(e));
        -1
    } else {
        0
    }
}
#[cfg(test)]
mod tests {
    use azure_core_amqp::connection::{AmqpConnection, AmqpConnectionApis};

    use crate::amqp::connection::RustAmqpConnection;
    use crate::amqp::session::{amqpsession_begin, amqpsession_create};
    use crate::runtime_context::RuntimeContext;

    use super::*;
    use std::ffi::CString;

    use std::time::{Duration, UNIX_EPOCH};

    #[test]
    fn test_amqpclaimsbasedsecurity_authorize_path_success() {
        unsafe {
            let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
            let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
            let session = amqpsession_create();
            let connection = AmqpConnection::new();

            call_context_from_ptr_mut(call_context)
                .runtime_context()
                .runtime()
                .block_on(connection.open(
                    "testConnection".to_string(),
                    url::Url::parse("amqp://localhost:25672").unwrap(),
                    None,
                ))
                .unwrap();

            let connection = Box::into_raw(Box::new(RustAmqpConnection::new(connection)));
            let result = amqpsession_begin(call_context, session, connection, std::ptr::null_mut());
            assert_eq!(result, 0);

            let claims_based_security = Box::into_raw(Box::new(RustAmqpClaimsBasedSecurity {
                inner: AmqpClaimsBasedSecurity::new((*session).get_session()).unwrap(),
            }));

            let result = amqpclaimsbasedsecurity_attach(call_context, claims_based_security);
            assert_eq!(result, 0);

            let path = CString::new("test_path").unwrap();
            let secret = CString::new("test_secret").unwrap();
            let expires_on = UNIX_EPOCH + Duration::from_secs(1000);

            let result = amqpclaimsbasedsecurity_authorize_path(
                call_context,
                claims_based_security,
                path.as_ptr(),
                secret.as_ptr(),
                expires_on.duration_since(UNIX_EPOCH).unwrap().as_secs(),
            );

            assert_eq!(result, 0);
        }
    }
}
