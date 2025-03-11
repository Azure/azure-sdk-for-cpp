// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// cspell: words reqwest repr tokio

use azure_core::{error::ErrorKind, Error, Result};
use std::mem;

#[derive(Debug)]
pub struct RuntimeContext {
    runtime: tokio::runtime::Runtime,
}

impl RuntimeContext {
    pub fn new() -> Result<Self> {
        Ok(Self {
            //            runtime: match tokio::runtime::Builder::new_current_thread()
            runtime: match tokio::runtime::Builder::new_multi_thread()
                .enable_all()
                .build()
                .map_err(|err| Error::new(ErrorKind::Other, err))
            {
                Ok(it) => it,
                Err(err) => return Err(err),
            },
        })
    }
    pub fn runtime(&self) -> &tokio::runtime::Runtime {
        &self.runtime
    }
}

#[no_mangle]
pub extern "C" fn runtime_context_new() -> *mut RuntimeContext {
    match RuntimeContext::new() {
        Ok(ctx) => Box::into_raw(Box::new(ctx)),
        Err(_) => std::ptr::null_mut(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn runtime_context_delete(ctx: *mut RuntimeContext) {
    mem::drop(Box::from_raw(ctx))
}

#[test]
fn test_runtime_context_new() {
    let ctx = runtime_context_new();
    assert_ne!(ctx, std::ptr::null_mut());
    unsafe {
        runtime_context_delete(ctx);
    }
}

#[test]
fn test_runtime_context_delete() {
    let ctx = runtime_context_new();
    assert_ne!(ctx, std::ptr::null_mut());
    unsafe {
        runtime_context_delete(ctx);
    }
}
