// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

#![crate_type = "staticlib"]

use std::{
    ffi::{c_char, CString},
    mem,
};
pub mod header;
pub mod message;
pub mod message_fields;
pub mod properties;
pub mod source;
pub mod target;
pub mod value;

#[no_mangle]
pub extern "C" fn rust_string_delete(rust_string: *const c_char) {
    unsafe {
        mem::drop(CString::from_raw(rust_string as *mut c_char));
    }
}
