// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
cspell: words reqwest repr staticlib dylib brotli gzip
*/

use std::{
    ffi::{c_char, CString},
    fmt::Debug,
};

struct RustTracingSubscriber {
    tracing_callback: Option<extern "C" fn(_: *const c_char)>,
}

impl RustTracingSubscriber {
    fn new() -> Self {
        Self {
            tracing_callback: None,
        }
    }

    fn set_tracing_callback(&mut self, callback: extern "C" fn(_: *const c_char)) {
        self.tracing_callback = Some(callback);
    }

    fn on_event(&self, message: &str) {
        if let Some(callback) = self.tracing_callback {
            let c_message = CString::new(message).unwrap();
            callback(c_message.as_ptr());
        }
    }
}

impl Debug for RustTracingSubscriber {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("RustTracingSubscriber")
            .field("tracing_callback", &self.tracing_callback.is_some())
            .finish()
    }
}

#[no_mangle]
pub extern "C" fn register_tracing_callback(callback: extern "C" fn(_: *const c_char)) {
    let message = CString::new("register_tracing_callback").unwrap();
    callback(message.as_ptr());
}

#[no_mangle]
pub extern "C" fn unregister_tracing_callback() {}

#[no_mangle]
pub extern "C" fn enable_tracing_integration() {
    tracing_subscriber::fmt::init();
}
