// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
cspell: words reqwest repr staticlib dylib brotli gzip
*/

use std::ffi::{c_char, CString};

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

impl tracing::Subscriber for RustTracingSubscriber {
    fn enabled(&self, _metadata: &tracing::Metadata) -> bool {
        true
    }

    fn new_span(&self, span: &tracing::span::Attributes) -> tracing::span::Id {
        tracing::span::Id::from_u64(span.metadata().id().into_u64())
    }

    fn record(&self, _span: &tracing::span::Id, _values: &tracing::span::Record) {}

    fn record_follows_from(&self, _span: &tracing::span::Id, _follows: &tracing::span::Id) {}

    fn event(&self, event: &tracing::Event) {
        self.on_event(event.metadata().name());
    }

    fn enter(&self, _span: &tracing::span::Id) {}

    fn exit(&self, _span: &tracing::span::Id) {}

    fn clone_span(&self, _span: &tracing::span::Id) -> tracing::span::Id {
        tracing::span::Id::from_u64(0)
    }

    fn drop_span(&self, _span: tracing::span::Id) {}
}

#[no_mangle]
pub extern "C" fn register_tracing_callback(callback: extern "C" fn(_: *const c_char)) {
    let message = CString::new("register_tracing_callback").unwrap();
    callback(message.as_ptr());
}

#[no_mangle]
pub extern "C" fn unregister_tracing_callback() {}
