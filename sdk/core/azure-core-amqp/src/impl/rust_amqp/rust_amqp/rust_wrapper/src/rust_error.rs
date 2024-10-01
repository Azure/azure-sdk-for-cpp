// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
cspell: words reqwest repr staticlib dylib brotli gzip
*/

use std::ffi::c_char;

#[derive(Debug)]
pub struct RustError(Box<dyn std::error::Error>);

impl RustError {
    pub fn new(error: Box<dyn std::error::Error>) -> Self {
        RustError(error)
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rust_error_delete(error: *mut RustError) {
    unsafe {
        drop(Box::from_raw(error));
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rust_error_get_message(error: *const RustError) -> *mut c_char {
    let error = unsafe { &*error };
    let message = format!("{:?}", error.0);
    let c_message = std::ffi::CString::new(message).unwrap();
    c_message.into_raw()
}
