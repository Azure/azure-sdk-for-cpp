// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp

#![crate_type = "staticlib"]

use std::{
    ffi::{c_char, CString},
    mem,
};

pub mod amqp;
pub mod call_context;
pub mod model;
pub mod runtime_context;
//pub mod rust_error;
pub mod tracing;

#[no_mangle]
pub extern "C" fn rust_string_delete(rust_string: *const c_char) {
    unsafe {
        mem::drop(CString::from_raw(rust_string as *mut c_char));
    }
}
