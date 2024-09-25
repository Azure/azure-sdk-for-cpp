// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
cspell: words reqwest repr staticlib dylib brotli gzip
*/

use std::ffi::c_char;

pub struct RustError(azure_core::Error);

impl RustError {
    pub fn new(error: azure_core::Error) -> Self {
        RustError(error)
    }
}

#[no_mangle]
pub extern "C" fn rust_error_get_message(error: *const RustError) -> *mut c_char {
    let error = unsafe { error.as_ref().unwrap() };
    let message = error.0.to_string();
    let c_message = std::ffi::CString::new(message).unwrap();
    c_message.into_raw()
}

#[no_mangle]
pub extern "C" fn rust_error_delete(error: *mut RustError) {
    unsafe {
        let _ = Box::from_raw(error);
    }
}
