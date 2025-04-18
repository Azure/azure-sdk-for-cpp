// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// cspell: words reqwest repr tokio

use crate::runtime_context::RuntimeContext;
use std::ffi::c_char;

pub struct RustCallContext {
    runtime_context: *mut RuntimeContext,
    error: Option<String>,
}

impl RustCallContext {
    pub fn new(runtime_context: *mut RuntimeContext) -> Self {
        Self {
            runtime_context,
            error: None,
        }
    }

    /// # Safety
    pub unsafe fn runtime_context(&self) -> &RuntimeContext {
        &*self.runtime_context
    }

    pub fn set_error(&mut self, error: Box<dyn std::error::Error + Send + Sync>) {
        self.error = Some(format!("{:?}", error));
    }
}

#[no_mangle]
pub extern "C" fn call_context_new(runtime_context: *mut RuntimeContext) -> *mut RustCallContext {
    Box::into_raw(Box::new(RustCallContext::new(runtime_context)))
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn call_context_delete(ctx: *mut RustCallContext) {
    drop(Box::from_raw(ctx));
}

/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn call_context_get_error(ctx: *const RustCallContext) -> *mut c_char {
    let call_context = &*ctx;
    match call_context.error {
        Some(ref error) => {
            let c_message = std::ffi::CString::new(error.clone()).unwrap();
            c_message.into_raw()
        }
        None => std::ptr::null_mut(),
    }
}

pub(crate) unsafe fn call_context_from_ptr_mut<'a>(
    ctx: *mut RustCallContext,
) -> &'a mut RustCallContext {
    &mut *ctx
}

#[test]
fn test_call_context_get_error() {
    unsafe {
        let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        assert_ne!(runtime_context, std::ptr::null_mut());
        let call_context = Box::into_raw(Box::new(RustCallContext::new(runtime_context)));
        let error = call_context_get_error(call_context);
        assert_eq!(error, std::ptr::null_mut());
        drop(Box::from_raw(call_context));
        drop(Box::from_raw(runtime_context));
    }
}

#[test]
fn test_call_context_set_error() {
    unsafe {
        let ctx = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
        let mut call_context = RustCallContext::new(ctx);
        call_context.set_error(Box::new(azure_core::Error::new(
            azure_core::error::ErrorKind::Other,
            "test",
        )));
        let error = call_context_get_error(&call_context);
        assert_ne!(error, std::ptr::null_mut());
    }
}
