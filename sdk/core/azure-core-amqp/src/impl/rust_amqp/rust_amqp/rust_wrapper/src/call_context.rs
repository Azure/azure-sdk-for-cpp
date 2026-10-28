// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// cspell: words reqwest repr tokio

use crate::runtime_context::RuntimeContext;
use std::ffi::c_char;
use std::future::Future;
use std::time::Duration;

pub struct RustCallContext {
    runtime_context: *mut RuntimeContext,
    error: Option<String>,
    /// The bound for one call. `Duration::MAX` means the call has no bound.
    timeout: Duration,
}

impl RustCallContext {
    pub fn new(runtime_context: *mut RuntimeContext) -> Self {
        Self {
            runtime_context,
            error: None,
            timeout: Duration::MAX,
        }
    }

    /// # Safety
    pub unsafe fn runtime_context(&self) -> &RuntimeContext {
        &*self.runtime_context
    }

    pub fn set_error(&mut self, error: Box<dyn std::error::Error + Send + Sync>) {
        self.error = Some(format!("{:?}", error));
    }

    /// The bound for one call, or `None` when the caller set no bound.
    pub(crate) fn timeout(&self) -> Option<Duration> {
        if self.timeout == Duration::MAX {
            None
        } else {
            Some(self.timeout)
        }
    }

    /// Run the future on the runtime, and end it when the bound expires.
    ///
    /// A future that never completes, for example a send on a connection that
    /// the service dropped, holds the calling thread forever without this
    /// bound.
    pub(crate) fn block_on_with_timeout<F: Future>(
        &self,
        future: F,
    ) -> Result<F::Output, tokio::time::error::Elapsed> {
        let runtime = unsafe { self.runtime_context() }.runtime();
        match self.timeout() {
            // The timer belongs to the runtime, so the async block builds it
            // inside block_on. A timeout that this thread builds first panics.
            Some(duration) => {
                runtime.block_on(async move { tokio::time::timeout(duration, future).await })
            }
            None => Ok(runtime.block_on(future)),
        }
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

/// Set the bound for one call, in milliseconds.
///
/// # Safety
///
#[no_mangle]
pub unsafe extern "C" fn call_context_set_timeout_ms(ctx: *mut RustCallContext, timeout_ms: u64) {
    let call_context = &mut *ctx;
    call_context.timeout = Duration::from_millis(timeout_ms);
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

#[test]
fn test_call_context_block_on_with_timeout() {
    let runtime_context = Box::into_raw(Box::new(RuntimeContext::new().unwrap()));
    let mut call_context = RustCallContext::new(runtime_context);
    call_context.timeout = std::time::Duration::from_millis(100);

    let start = std::time::Instant::now();
    let never = call_context.block_on_with_timeout(std::future::pending::<()>());
    let elapsed = start.elapsed();
    assert!(never.is_err());
    assert!(elapsed < std::time::Duration::from_secs(5));

    let seven = call_context.block_on_with_timeout(async { 7 });
    assert!(seven.is_ok());
    assert_eq!(seven.unwrap(), 7);

    unsafe {
        drop(Box::from_raw(runtime_context));
    }
}
