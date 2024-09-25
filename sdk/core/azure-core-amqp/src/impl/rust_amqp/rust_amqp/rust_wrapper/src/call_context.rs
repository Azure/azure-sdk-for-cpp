// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// cspell: words reqwest repr tokio

use crate::{runtime_context::RuntimeContext, rust_error::RustError};

pub struct RustCallContext {
    runtime_context: *mut RuntimeContext,
    error: Option<RustError>,
}

impl RustCallContext {
    pub fn new(runtime_context: *mut RuntimeContext) -> Self {
        Self {
            runtime_context,
            error: None,
        }
    }

    pub fn runtime_context(&self) -> &RuntimeContext {
        unsafe { &*self.runtime_context }
    }

    pub fn set_error(&mut self, error: Box<dyn std::error::Error + Send + Sync>) {
        self.error = Some(RustError::new(error));
    }
}

#[no_mangle]
pub extern "C" fn call_context_new(runtime_context: *mut RuntimeContext) -> *mut RustCallContext {
    Box::into_raw(Box::new(RustCallContext::new(runtime_context)))
}

#[no_mangle]
pub extern "C" fn call_context_delete(ctx: *mut RustCallContext) {
    unsafe {
        drop(Box::from_raw(ctx));
    }
}

#[no_mangle]
pub extern "C" fn call_context_get_error(ctx: *const RustCallContext) -> *mut RustError {
    let ctx = unsafe { &*ctx };
    match &ctx.error {
        Some(err) => Box::into_raw(Box::new(err)).cast(),
        None => std::ptr::null::<RustError>().cast_mut(),
    }
}

pub(crate) fn call_context_from_ptr_mut<'a>(ctx: *mut RustCallContext) -> &'a mut RustCallContext {
    unsafe { &mut *ctx }
}

#[test]
fn test_call_context_get_error() {
    let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
    assert_ne!(runtime_context, std::ptr::null_mut());
    let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
    let error = call_context_get_error(call_context);
    assert_eq!(error, std::ptr::null_mut());
    unsafe {
        drop(Box::from_raw(call_context));
        drop(Box::from_raw(runtime_context));
    }
}

#[test]
fn test_call_context_set_error() {
    let ctx = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
    let mut call_context = RustCallContext::new(ctx);
    call_context.set_error(Box::new(azure_core::Error::new(
        azure_core::error::ErrorKind::Other,
        "test",
    )));
    let error = call_context_get_error(&call_context);
    assert_ne!(error, std::ptr::null_mut());
}
