// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// cspell: words reqwest repr tokio

use crate::runtime_context::RuntimeContext;
use std::ffi::{c_char, c_void};
use std::future::Future;
use std::time::Duration;

pub struct RustCallContext {
    runtime_context: *mut RuntimeContext,
    error: Option<String>,
    /// The bound for one call. `Duration::MAX` means the call has no bound.
    timeout: Duration,
    cancellation_probe: Option<unsafe extern "C" fn(*const c_void) -> bool>,
    cancellation_state: *const c_void,
}

impl RustCallContext {
    pub fn new(runtime_context: *mut RuntimeContext) -> Self {
        Self {
            runtime_context,
            error: None,
            timeout: Duration::MAX,
            cancellation_probe: None,
            cancellation_state: std::ptr::null(),
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

    fn is_cancelled(&self) -> bool {
        self.cancellation_probe
            .map(|probe| unsafe { probe(self.cancellation_state) })
            .unwrap_or(false)
    }

    /// The callback and its state are borrowed only for this synchronous call.
    /// This future is never spawned onto the runtime's worker threads.
    pub(crate) fn block_on_with_cancellation<F: Future>(
        &self,
        future: F,
    ) -> Result<F::Output, OperationCancelledOrTimedOut> {
        if self.is_cancelled() {
            return Err(OperationCancelledOrTimedOut::Cancelled);
        }
        if self.timeout() == Some(Duration::ZERO) {
            return Err(OperationCancelledOrTimedOut::TimedOut);
        }
        let runtime = unsafe { self.runtime_context() }.runtime();
        runtime.block_on(async {
            let cancellation = async {
                loop {
                    if self.is_cancelled() {
                        return;
                    }
                    tokio::time::sleep(Duration::from_millis(10)).await;
                }
            };
            let operation = async {
                match self.timeout() {
                    Some(duration) => tokio::time::timeout(duration, future)
                        .await
                        .map_err(|_| OperationCancelledOrTimedOut::TimedOut),
                    None => Ok(future.await),
                }
            };
            tokio::select! {
                biased;
                _ = cancellation => Err(OperationCancelledOrTimedOut::Cancelled),
                result = operation => result,
            }
        })
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

#[derive(Debug, PartialEq, Eq)]
pub(crate) enum OperationCancelledOrTimedOut {
    Cancelled,
    TimedOut,
}

impl std::fmt::Display for OperationCancelledOrTimedOut {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Self::Cancelled => "AMQP operation cancelled",
            Self::TimedOut => "AMQP operation timed out",
        })
    }
}
impl std::error::Error for OperationCancelledOrTimedOut {}

/// Register an optional cancellation probe for an operation. The state must
/// remain valid and stationary until the synchronous operation returns.
///
/// # Safety
/// The caller owns the state; the callback must not throw across the C ABI.
#[no_mangle]
pub unsafe extern "C" fn call_context_set_cancellation(
    ctx: *mut RustCallContext,
    probe: Option<unsafe extern "C" fn(*const c_void) -> bool>,
    state: *const c_void,
) {
    (*ctx).cancellation_probe = probe;
    (*ctx).cancellation_state = state;
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

#[cfg(test)]
mod cancellation_tests {
    use super::*;
    use std::pin::Pin;
    use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
    use std::sync::Arc;
    use std::task::{Context, Poll};

    struct PendingOperation {
        polls: Arc<AtomicUsize>,
        drops: Arc<AtomicUsize>,
    }
    impl Future for PendingOperation {
        type Output = ();
        fn poll(self: Pin<&mut Self>, _: &mut Context<'_>) -> Poll<()> {
            self.polls.fetch_add(1, Ordering::SeqCst);
            Poll::Pending
        }
    }
    impl Drop for PendingOperation {
        fn drop(&mut self) {
            self.drops.fetch_add(1, Ordering::SeqCst);
        }
    }
    unsafe extern "C" fn probe(state: *const c_void) -> bool {
        (*(state as *const AtomicBool)).load(Ordering::SeqCst)
    }
    #[test]
    fn call_context_pre_cancel_and_expired_deadline_never_poll() {
        let mut runtime = RuntimeContext::new().unwrap();
        let mut context = RustCallContext::new(&mut runtime);
        let state = AtomicBool::new(true);
        unsafe {
            call_context_set_cancellation(&mut context, Some(probe), &state as *const _ as _);
        }
        for expected in [
            OperationCancelledOrTimedOut::Cancelled,
            OperationCancelledOrTimedOut::TimedOut,
        ] {
            let polls = Arc::new(AtomicUsize::new(0));
            let drops = Arc::new(AtomicUsize::new(0));
            assert_eq!(
                context.block_on_with_cancellation(PendingOperation {
                    polls: polls.clone(),
                    drops: drops.clone()
                }),
                Err(expected)
            );
            assert_eq!(polls.load(Ordering::SeqCst), 0);
            assert_eq!(drops.load(Ordering::SeqCst), 1);
            state.store(false, Ordering::SeqCst);
            context.timeout = Duration::ZERO;
        }
    }
    #[test]
    fn call_context_cancellation_drops_pending_future_once() {
        let mut runtime = RuntimeContext::new().unwrap();
        let mut context = RustCallContext::new(&mut runtime);
        let state = Arc::new(AtomicBool::new(false));
        let polls = Arc::new(AtomicUsize::new(0));
        let drops = Arc::new(AtomicUsize::new(0));
        unsafe {
            call_context_set_cancellation(&mut context, Some(probe), Arc::as_ptr(&state) as _);
        }
        let signal = state.clone();
        let observed = polls.clone();
        let cancel = std::thread::spawn(move || {
            let start = std::time::Instant::now();
            while observed.load(Ordering::SeqCst) == 0 {
                assert!(start.elapsed() < Duration::from_secs(5));
                std::thread::yield_now();
            }
            signal.store(true, Ordering::SeqCst);
        });
        context.timeout = Duration::from_secs(5);
        assert_eq!(
            context.block_on_with_cancellation(PendingOperation {
                polls,
                drops: drops.clone()
            }),
            Err(OperationCancelledOrTimedOut::Cancelled)
        );
        cancel.join().unwrap();
        assert_eq!(drops.load(Ordering::SeqCst), 1);
    }
    #[test]
    fn call_context_success_deadline_and_teardown() {
        let mut runtime = RuntimeContext::new().unwrap();
        let mut context = RustCallContext::new(&mut runtime);
        context.timeout = Duration::from_millis(20);
        assert_eq!(context.block_on_with_cancellation(async { 7 }), Ok(7));
        assert_eq!(
            context.block_on_with_cancellation(std::future::pending::<()>()),
            Err(OperationCancelledOrTimedOut::TimedOut)
        );
        let state = AtomicBool::new(true);
        unsafe {
            call_context_set_cancellation(&mut context, Some(probe), &state as *const _ as _);
        }
        assert_eq!(context.block_on_with_timeout(async { 9 }).unwrap(), 9);
        assert!(context
            .block_on_with_timeout(std::future::pending::<()>())
            .is_err());
    }
}
