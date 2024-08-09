// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr


use azure_core_amqp::value::AmqpValue as Value;
use azure_core_amqp::value::AmqpTimestamp;

use std::ffi::{CString, c_char};
use std::mem;

#[no_mangle]
pub extern "C" fn rust_string_delete(rust_string: *mut c_char) {
    unsafe {
        mem::drop(CString::from_raw(rust_string));
    }
}

pub struct AmqpValue {
    inner: Value,
}

#[repr(C)]
pub enum AmqpValueType {
    AmqpValueInvalid,
    AmqpValueNull,
    AmqpValueBoolean,
    AmqpValueUByte,
    AmqpValueUShort,
    AmqpValueUint,
    AmqpValueUlong,
    AmqpValueByte,
    AmqpValueShort,
    AmqpValueInt,
    AmqpValueLong,
    AmqpValueFloat,
    AmqpValueDouble,
    AmqpValueChar,
    AmqpValueTimestamp,
    AmqpValueUuid,
    AmqpValueBinary,
    AmqpValueString,
    AmqpValueSymbol,
    AmqpValueList,
    AmqpValueMap,
    AmqpValueArray,
    AmqpValueComposite,
    AmqpValueDescribed,
    AmqpValueUnknown,
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_type(value:*const AmqpValue) -> AmqpValueType
{
    let value = unsafe { &*value };
    match value.inner {
        Value::Null => AmqpValueType::AmqpValueNull,
        Value::Boolean(_) => AmqpValueType::AmqpValueBoolean,
        Value::UByte(_) => AmqpValueType::AmqpValueUByte,
        Value::UShort(_) => AmqpValueType::AmqpValueUShort,
        Value::UInt(_) => AmqpValueType::AmqpValueUint,
        Value::ULong(_) => AmqpValueType::AmqpValueUlong,
        Value::Byte(_) => AmqpValueType::AmqpValueByte,
        Value::Short(_) => AmqpValueType::AmqpValueShort,
        Value::Int(_) => AmqpValueType::AmqpValueInt,
        Value::Long(_) => AmqpValueType::AmqpValueLong,
        Value::Float(_) => AmqpValueType::AmqpValueFloat,
        Value::Double(_) => AmqpValueType::AmqpValueDouble,
        Value::Char(_) => AmqpValueType::AmqpValueChar,
        Value::TimeStamp(_) => AmqpValueType::AmqpValueTimestamp,
        Value::Uuid(_) => AmqpValueType::AmqpValueUuid,
        Value::Binary(_) => AmqpValueType::AmqpValueBinary,
        Value::String(_) => AmqpValueType::AmqpValueString,
        Value::Symbol(_) => AmqpValueType::AmqpValueSymbol,
        Value::List(_) => AmqpValueType::AmqpValueList,
        Value::Map(_) => AmqpValueType::AmqpValueMap,
        Value::Array(_) => AmqpValueType::AmqpValueArray,
        Value::Described(_) => AmqpValueType::AmqpValueDescribed,
//        Value::Composite(_) => AmqpValueType::AmqpValueComposite,
        Value::Unknown => AmqpValueType::AmqpValueUnknown,
    }
}


#[no_mangle]
pub extern "C" fn amqpvalue_create_null() -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Null };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_boolean(bool_value: bool) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Boolean(bool_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_boolean(value: *const AmqpValue, bool_value: *mut bool) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Boolean(b) => {
            unsafe { *bool_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_ubyte(ubyte_value: u8) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::UByte(ubyte_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_ubyte(value: *const AmqpValue, ubyte_value: *mut u8) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::UByte(b) => {
            unsafe { *ubyte_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_byte(byte_value: i8) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Byte(byte_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_byte(value: *const AmqpValue, byte_value: *mut i8) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Byte(b) => {
            unsafe { *byte_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_ushort(ushort_value: u16) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::UShort(ushort_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_ushort(value: *const AmqpValue, ushort_value: *mut u16) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::UShort(b) => {
            unsafe { *ushort_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_short(short_value: i16) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Short(short_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_short(value: *const AmqpValue, short_value: *mut i16) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Short(b) => {
            unsafe { *short_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_uint(uint_value: u32) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::UInt(uint_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_uint(value: *const AmqpValue, uint_value: *mut u32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::UInt(b) => {
            unsafe { *uint_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_int(int_value: i32) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Int(int_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_int(value: *const  AmqpValue, int_value: *mut i32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Int(b) => {
            unsafe { *int_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_ulong(ulong_value: u64) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::ULong(ulong_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_ulong(value: *const AmqpValue, ulong_value: *mut u64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::ULong(b) => {
            unsafe { *ulong_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_long(long_value: i64) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Long(long_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_long(value: *const AmqpValue, long_value: *mut i64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Long(b) => {
            unsafe { *long_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_float(float_value: f32) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Float(float_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_float(value: *const AmqpValue, float_value: *mut f32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Float(b) => {
            unsafe { *float_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_double(double_value: f64) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Double(double_value) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_double(value: *const AmqpValue, double_value: *mut f64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Double(b) => {
            unsafe { *double_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_char(char_value: u32) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Char(char::from_u32(char_value).unwrap()) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_char(value: *const AmqpValue, char_value: *mut u32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        Value::Char(b) => {
            unsafe { *char_value = b as u32};
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_timestamp(timestamp_value: u64) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::TimeStamp(AmqpTimestamp(std::time::SystemTime::UNIX_EPOCH + std::time::Duration::from_millis(timestamp_value))) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_timestamp(value: *const AmqpValue, timestamp_value: *mut i64) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        Value::TimeStamp(v) => {
            unsafe { *timestamp_value = v.0.duration_since(std::time::SystemTime::UNIX_EPOCH).unwrap().as_millis() as i64 };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_uuid(uuid_value: *const [u8;16]) -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: unsafe {Value::Uuid(uuid::Uuid::from_bytes(*uuid_value)) }};
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_uuid(value: *const AmqpValue, uuid_value: *mut [u8;16]) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        Value::Uuid(b) => {
            unsafe { (*uuid_value).copy_from_slice(b.as_bytes()) };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_string(string_value: *const c_char) -> *mut AmqpValue {
    let string_value = unsafe { std::ffi::CStr::from_ptr(string_value).to_str().unwrap() };
    let amqp_value = AmqpValue { inner: Value::String(string_value.to_string()) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_string(value: *const AmqpValue, string_value: *mut *const std::os::raw::c_char) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        Value::String(b) => {
            unsafe { *string_value = std::ffi::CString::new(b.clone()).unwrap().into_raw() };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_clone(value: *mut AmqpValue) -> *mut AmqpValue {
    let value = unsafe { &*value };
    let amqp_value = AmqpValue { inner: value.inner.clone() };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub unsafe extern "C" fn amqpvalue_destroy(value: *mut AmqpValue) {
    mem::drop(Box::from(value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_are_equal(value1: *const AmqpValue, value2: *const AmqpValue) -> bool {
    let value1 = unsafe { &*value1 };
    let value2 = unsafe { &*value2 };
    value1.inner == value2.inner
}

#[no_mangle]
pub extern "C" fn amqpvalue_to_string(value: *const AmqpValue) -> *mut c_char {
    let value = unsafe { &*value };
    let s = format!("{:?}", value.inner);
    let c_str = CString::new(s).unwrap();
    c_str.into_raw()
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_array() -> *mut AmqpValue {
    let amqp_value = AmqpValue { inner: Value::Array(Vec::new()) };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_add_array_item(value: *mut AmqpValue, array_item_value: *mut AmqpValue) -> i32 {
    let value = unsafe { &mut *value };
    let array_item_value = unsafe { &*array_item_value };
    match &mut value.inner {
        Value::Array(v) => {
            v.push(array_item_value.inner.clone());
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_array_item(value: *const AmqpValue, index: u32) -> *mut AmqpValue {
    let value = unsafe { &*value };
    match &value.inner {
        Value::Array(v) => {
            let amqp_value = AmqpValue { inner: v[index as usize].clone() };
            Box::into_raw(Box::new(amqp_value))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_array_item_count(value: *const AmqpValue, count: *mut u32) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        Value::Array(v) => unsafe {*count = v.len() as u32; 0},
        _ => -1,
    }
}

// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_binary, amqp_binary, binary_value);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_binary, AMQP_VALUE, value, amqp_binary*, binary_value);
// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_symbol, const char*, symbol_value);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_symbol, AMQP_VALUE, value, const char**, symbol_value);
// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_list);
// MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item_count, AMQP_VALUE, list, uint32_t, count);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_list_item_count, AMQP_VALUE, list, uint32_t*, count);
// MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item, AMQP_VALUE, list, uint32_t, index, AMQP_VALUE, list_item_value);
// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_list_item, AMQP_VALUE, list, size_t, index);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_list, AMQP_VALUE, from_value, AMQP_VALUE*, list);
// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_map);
// MOCKABLE_FUNCTION(, int, amqpvalue_set_map_value, AMQP_VALUE, map, AMQP_VALUE, key, AMQP_VALUE, value);
// MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_map_value, AMQP_VALUE, map, AMQP_VALUE, key);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_map_pair_count, AMQP_VALUE, map, uint32_t*, pair_count);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_map_key_value_pair, AMQP_VALUE, map, uint32_t, index, AMQP_VALUE*, key, AMQP_VALUE*, value);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_map, AMQP_VALUE, from_value, AMQP_VALUE*, map);
// MOCKABLE_FUNCTION(, int, amqpvalue_get_array, AMQP_VALUE, value, AMQP_VALUE*, array_value);
// MOCKABLE_FUNCTION(, AMQP_TYPE, amqpvalue_get_type, AMQP_VALUE, value);
