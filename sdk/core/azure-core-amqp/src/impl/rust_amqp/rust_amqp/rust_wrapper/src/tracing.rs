// Copyright (c) Microsoft Corp. All Rights Reserved
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
cspell: words reqwest repr staticlib dylib brotli gzip
*/


#[no_mangle]
pub extern "C" fn enable_tracing_integration() {
    tracing_subscriber::fmt::init();
}
