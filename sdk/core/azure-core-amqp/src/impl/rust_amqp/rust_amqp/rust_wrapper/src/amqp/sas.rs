// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.
// cspell: words sastoken servicebus

use crate::{
    call_context::{call_context_from_ptr_mut, RustCallContext},
    error_from_str,
};
use base64::{engine::general_purpose::STANDARD, Engine};
use hmac::{Hmac, Mac};
use sha2::Sha256;
use std::ffi::{c_char, CStr, CString};

/// Percent-encode a string the way `URL_EncodeString` from azure-c-shared-utility does.
///
/// The uAMQP backend builds its SAS tokens with that encoder, so this backend must
/// produce the same bytes. The character set is not RFC 3986. It escapes `~`, which
/// RFC 3986 lists as unreserved, and it passes `!`, `(`, `)`, and `*` through, which
/// RFC 3986 lists as sub-delimiters. The hex digits are lowercase, so the output is
/// `%3a` and not `%3A`.
///
/// One difference is deliberate. azure-c-shared-utility treats each byte at or above
/// 0x80 as a Latin-1 code point and emits the percent-encoded UTF-8 of that code
/// point, which garbles UTF-8 input. This function percent-encodes the raw UTF-8
/// bytes instead. Event Hubs and Service Bus entity names are ASCII, so the two
/// encoders agree in practice.
fn url_encode(value: &str) -> String {
    const HEX_DIGITS: &[u8; 16] = b"0123456789abcdef";

    let mut encoded = String::with_capacity(value.len());
    for byte in value.as_bytes() {
        let is_printable = matches!(byte, b'!' | b'(' | b')' | b'*' | b'-' | b'.' | b'_')
            || byte.is_ascii_digit()
            || byte.is_ascii_uppercase()
            || byte.is_ascii_lowercase();

        if is_printable {
            encoded.push(*byte as char);
        } else {
            encoded.push('%');
            encoded.push(HEX_DIGITS[(byte >> 4) as usize] as char);
            encoded.push(HEX_DIGITS[(byte & 0x0f) as usize] as char);
        }
    }
    encoded
}

/// Generate a Shared Access Signature token for a Service Bus or Event Hubs resource.
///
/// The fields appear in the order `sr`, `sig`, `se`, `skn`, which mirrors
/// `SASToken_Create` in azure-c-shared-utility. The Microsoft documentation shows a
/// different order. Both orders work, because the service reads the fields by name.
/// Matching the uAMQP order lets one expected token cover both backends in a test.
pub fn generate_sas_token(
    resource_uri: &str,
    key_name: &str,
    key: &str,
    expires_on_secs: u64,
) -> String {
    let scope = url_encode(resource_uri);
    let to_sign = format!("{}\n{}", scope, expires_on_secs);

    // The shared access key is a plain text string that looks like base64. The Service
    // Bus specification says not to decode it, so the HMAC key is its raw UTF-8 bytes.
    // The uAMQP path reaches the same result by base64-encoding the key so that
    // SASToken_Create can base64-decode it again.
    let mut mac =
        Hmac::<Sha256>::new_from_slice(key.as_bytes()).expect("HMAC accepts a key of any length");
    mac.update(to_sign.as_bytes());
    let signature = STANDARD.encode(mac.finalize().into_bytes());

    // The key name is never percent-encoded.
    format!(
        "SharedAccessSignature sr={}&sig={}&se={}&skn={}",
        scope,
        url_encode(&signature),
        expires_on_secs,
        key_name
    )
}

/// Create a Shared Access Signature token.
///
/// On success this returns an owned C string. The caller must release it with
/// `rust_string_delete`. On failure this records the reason in the call context and
/// returns null.
///
/// # Safety
///
/// This function is unsafe because it dereferences raw pointers. Every string
/// argument must be a valid, null-terminated C string.
#[no_mangle]
pub unsafe extern "C" fn sastoken_create(
    call_context: *mut RustCallContext,
    resource_uri: *const c_char,
    key_name: *const c_char,
    key: *const c_char,
    expires_on: u64,
) -> *mut c_char {
    let call_context = call_context_from_ptr_mut(call_context);

    if resource_uri.is_null() || key_name.is_null() || key.is_null() {
        call_context.set_error(error_from_str("SAS token arguments must not be null"));
        return std::ptr::null_mut();
    }

    let resource_uri = match CStr::from_ptr(resource_uri).to_str() {
        Ok(value) => value,
        Err(_) => {
            call_context.set_error(error_from_str("Resource URI is not valid UTF-8"));
            return std::ptr::null_mut();
        }
    };
    let key_name = match CStr::from_ptr(key_name).to_str() {
        Ok(value) => value,
        Err(_) => {
            call_context.set_error(error_from_str("Shared access key name is not valid UTF-8"));
            return std::ptr::null_mut();
        }
    };
    let key = match CStr::from_ptr(key).to_str() {
        Ok(value) => value,
        Err(_) => {
            call_context.set_error(error_from_str("Shared access key is not valid UTF-8"));
            return std::ptr::null_mut();
        }
    };

    let token = generate_sas_token(resource_uri, key_name, key, expires_on);
    match CString::new(token) {
        Ok(token) => token.into_raw(),
        Err(_) => {
            call_context.set_error(error_from_str("SAS token contains an interior null byte"));
            std::ptr::null_mut()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // This pins the three ways the encoder departs from RFC 3986: the hex digits are
    // lowercase, `~` is escaped, and `!` is not.
    #[test]
    fn url_encode_matches_azure_c_shared_utility() {
        assert_eq!(url_encode("sb://a.b/c~d!e"), "sb%3a%2f%2fa.b%2fc%7ed!e");
    }

    // The same expected token appears in the C++ test
    // ConnectionStringTest.GenerateSasTokenFixedVector, which runs on both the Rust
    // and the uAMQP backend. The key is fake.
    // cspell: disable
    #[test]
    fn generate_sas_token_matches_golden_vector() {
        let token = generate_sas_token(
            "sb://fake.servicebus.windows.net/eventhub1",
            "FakeKeyName",
            "ZmFrZWtleWZha2VrZXlmYWtla2V5ZmFrZWtleQ==",
            1735689600,
        );

        assert_eq!(
            token,
            "SharedAccessSignature sr=sb%3a%2f%2ffake.servicebus.windows.net%2feventhub1\
             &sig=166yHuTCCC7xv5eXWn%2fzAaC%2fRlB8GsmwNyKJpMehFp0%3d\
             &se=1735689600&skn=FakeKeyName"
        );
    }
    // cspell: enable
}
