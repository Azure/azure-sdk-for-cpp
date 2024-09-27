// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use azure_core_amqp::{
    value::{
        AmqpComposite, AmqpDescribed, AmqpDescriptor, AmqpList, AmqpOrderedMap, AmqpSymbol,
        AmqpTimestamp, AmqpValue,
    },
    Deserializable, Serializable,
};

use std::ffi::{c_char, CString};
use std::mem;
use std::ptr::null;
use tracing::error;

#[derive(Debug, Clone)]
pub struct RustAmqpValue {
    pub(crate) inner: AmqpValue,
}

#[repr(C)]
pub enum RustAmqpValueType {
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
pub extern "C" fn amqpvalue_get_type(value: *const RustAmqpValue) -> RustAmqpValueType {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Null => RustAmqpValueType::AmqpValueNull,
        AmqpValue::Boolean(_) => RustAmqpValueType::AmqpValueBoolean,
        AmqpValue::UByte(_) => RustAmqpValueType::AmqpValueUByte,
        AmqpValue::UShort(_) => RustAmqpValueType::AmqpValueUShort,
        AmqpValue::UInt(_) => RustAmqpValueType::AmqpValueUint,
        AmqpValue::ULong(_) => RustAmqpValueType::AmqpValueUlong,
        AmqpValue::Byte(_) => RustAmqpValueType::AmqpValueByte,
        AmqpValue::Short(_) => RustAmqpValueType::AmqpValueShort,
        AmqpValue::Int(_) => RustAmqpValueType::AmqpValueInt,
        AmqpValue::Long(_) => RustAmqpValueType::AmqpValueLong,
        AmqpValue::Float(_) => RustAmqpValueType::AmqpValueFloat,
        AmqpValue::Double(_) => RustAmqpValueType::AmqpValueDouble,
        AmqpValue::Char(_) => RustAmqpValueType::AmqpValueChar,
        AmqpValue::TimeStamp(_) => RustAmqpValueType::AmqpValueTimestamp,
        AmqpValue::Uuid(_) => RustAmqpValueType::AmqpValueUuid,
        AmqpValue::Binary(_) => RustAmqpValueType::AmqpValueBinary,
        AmqpValue::String(_) => RustAmqpValueType::AmqpValueString,
        AmqpValue::Symbol(_) => RustAmqpValueType::AmqpValueSymbol,
        AmqpValue::List(_) => RustAmqpValueType::AmqpValueList,
        AmqpValue::Map(_) => RustAmqpValueType::AmqpValueMap,
        AmqpValue::Array(_) => RustAmqpValueType::AmqpValueArray,
        AmqpValue::Described(_) => RustAmqpValueType::AmqpValueDescribed,
        AmqpValue::Composite(_) => RustAmqpValueType::AmqpValueComposite,
        AmqpValue::Unknown => RustAmqpValueType::AmqpValueUnknown,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_clone(value: *const RustAmqpValue) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let amqp_value = RustAmqpValue {
        inner: value.inner.clone(),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub unsafe extern "C" fn amqpvalue_destroy(value: *mut RustAmqpValue) {
    mem::drop(Box::from(value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_are_equal(
    value1: *const RustAmqpValue,
    value2: *const RustAmqpValue,
) -> bool {
    let value1 = unsafe { &*value1 };
    let value2 = unsafe { &*value2 };
    value1.inner == value2.inner
}

#[no_mangle]
pub extern "C" fn amqpvalue_to_string(value: *const RustAmqpValue) -> *mut c_char {
    let value = unsafe { &*value };
    let s = format!("{:?}", value.inner);
    let c_str = CString::new(s).unwrap();
    c_str.into_raw()
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_null() -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Null,
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_boolean(bool_value: bool) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Boolean(bool_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_boolean(value: *const RustAmqpValue, bool_value: *mut bool) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Boolean(b) => {
            unsafe { *bool_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_ubyte(ubyte_value: u8) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::UByte(ubyte_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_ubyte(value: *const RustAmqpValue, ubyte_value: *mut u8) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::UByte(b) => {
            unsafe { *ubyte_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_byte(byte_value: i8) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Byte(byte_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_byte(value: *const RustAmqpValue, byte_value: *mut i8) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Byte(b) => {
            unsafe { *byte_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_ushort(ushort_value: u16) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::UShort(ushort_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_ushort(value: *const RustAmqpValue, ushort_value: *mut u16) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::UShort(b) => {
            unsafe { *ushort_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_short(short_value: i16) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Short(short_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_short(value: *const RustAmqpValue, short_value: *mut i16) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Short(b) => {
            unsafe { *short_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_uint(uint_value: u32) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::UInt(uint_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_uint(value: *const RustAmqpValue, uint_value: *mut u32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::UInt(b) => {
            unsafe { *uint_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_int(int_value: i32) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Int(int_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_int(value: *const RustAmqpValue, int_value: *mut i32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Int(b) => {
            unsafe { *int_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_ulong(ulong_value: u64) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::ULong(ulong_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_ulong(value: *const RustAmqpValue, ulong_value: *mut u64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::ULong(b) => {
            unsafe { *ulong_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_long(long_value: i64) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Long(long_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_long(value: *const RustAmqpValue, long_value: *mut i64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Long(b) => {
            unsafe { *long_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_float(float_value: f32) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Float(float_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_float(value: *const RustAmqpValue, float_value: *mut f32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Float(b) => {
            unsafe { *float_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_double(double_value: f64) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Double(double_value),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_double(value: *const RustAmqpValue, double_value: *mut f64) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Double(b) => {
            unsafe { *double_value = b };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_char(char_value: u32) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Char(char::from_u32(char_value).unwrap()),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_char(value: *const RustAmqpValue, char_value: *mut u32) -> i32 {
    let value = unsafe { &*value };
    match value.inner {
        AmqpValue::Char(b) => {
            unsafe { *char_value = b as u32 };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_timestamp(timestamp_value: u64) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::TimeStamp(AmqpTimestamp(
            std::time::SystemTime::UNIX_EPOCH + std::time::Duration::from_millis(timestamp_value),
        )),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_timestamp(
    value: *const RustAmqpValue,
    timestamp_value: *mut i64,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::TimeStamp(v) => {
            unsafe {
                *timestamp_value =
                    v.0.duration_since(std::time::SystemTime::UNIX_EPOCH)
                        .unwrap()
                        .as_millis() as i64
            };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_uuid(uuid_value: *const [u8; 16]) -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: unsafe { AmqpValue::Uuid(uuid::Uuid::from_bytes(*uuid_value)) },
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_uuid(value: *const RustAmqpValue, uuid_value: *mut [u8; 16]) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Uuid(b) => {
            unsafe { (*uuid_value).copy_from_slice(b.as_bytes()) };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_string(string_value: *const c_char) -> *mut RustAmqpValue {
    let string_value = unsafe { std::ffi::CStr::from_ptr(string_value).to_str().unwrap() };
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::String(string_value.to_string()),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_string(
    value: *const RustAmqpValue,
    string_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::String(b) => {
            unsafe { *string_value = std::ffi::CString::new(b.clone()).unwrap().into_raw() };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
extern "C" fn amqpvalue_create_symbol(symbol_value: *const c_char) -> *mut RustAmqpValue {
    let symbol_value = unsafe { std::ffi::CStr::from_ptr(symbol_value).to_str().unwrap() };
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Symbol(AmqpSymbol::from(symbol_value.to_string())),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
extern "C" fn amqpvalue_get_symbol(
    value: *const RustAmqpValue,
    symbol_value: *mut *const std::os::raw::c_char,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Symbol(b) => {
            unsafe { *symbol_value = std::ffi::CString::new(b.0.clone()).unwrap().into_raw() };
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_binary(
    binary_value: *const u8,
    size: u32,
) -> *mut RustAmqpValue {
    let mut amqp_value = RustAmqpValue {
        inner: AmqpValue::Binary(Vec::new()),
    };
    for i in 0..size {
        match &mut amqp_value.inner {
            AmqpValue::Binary(v) => {
                v.push(unsafe { *binary_value.wrapping_add(i as usize) });
            }
            _ => {}
        }
    }
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_binary(
    value: *const RustAmqpValue,
    binary_value: *mut *const u8,
    size: *mut u32,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Binary(b) => {
            if b.len() != 0 {
                unsafe {
                    *binary_value = b.as_ptr();
                    *size = b.len() as u32
                };
            } else {
                unsafe {
                    *binary_value = null();
                    *size = 0;
                }
            }
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_array() -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Array(Vec::new()),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_add_array_item(
    value: *mut RustAmqpValue,
    array_item_value: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let array_item_value = unsafe { &*array_item_value };
    match &mut value.inner {
        AmqpValue::Array(v) => {
            v.push(array_item_value.inner.clone());
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_array_item(
    value: *const RustAmqpValue,
    index: u32,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Array(v) => {
            let amqp_value = RustAmqpValue {
                inner: v[index as usize].clone(),
            };
            Box::into_raw(Box::new(amqp_value))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_array_item_count(
    value: *const RustAmqpValue,
    count: *mut u32,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Array(v) => unsafe {
            *count = v.len() as u32;
            0
        },
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_list() -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::List(AmqpList::new()),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_add_list_item(
    value: *mut RustAmqpValue,
    list_item_value: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let list_item_value = unsafe { &*list_item_value };
    match &mut value.inner {
        AmqpValue::List(v) => {
            v.push(list_item_value.inner.clone());
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_list_item(
    value: *const RustAmqpValue,
    index: u32,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::List(v) => {
            let amqp_value = RustAmqpValue {
                inner: v.0[index as usize].clone(),
            };
            Box::into_raw(Box::new(amqp_value))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_list_item_count(
    value: *const RustAmqpValue,
    count: *mut u32,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::List(v) => unsafe {
            *count = v.0.len() as u32;
            0
        },
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_set_list_item_count(value: *mut RustAmqpValue, count: u32) -> i32 {
    let value = unsafe { &mut *value };
    match &mut value.inner {
        AmqpValue::List(v) => {
            v.0.resize(count as usize, AmqpValue::Null);
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_set_list_item(
    value: *mut RustAmqpValue,
    index: u32,
    list_item_value: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let list_item_value = unsafe { &*list_item_value };
    match &mut value.inner {
        AmqpValue::List(v) => {
            v.0[index as usize] = list_item_value.inner.clone();
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_map() -> *mut RustAmqpValue {
    let amqp_value = RustAmqpValue {
        inner: AmqpValue::Map(AmqpOrderedMap::new()),
    };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_add_map_item(
    value: *mut RustAmqpValue,
    key: *mut RustAmqpValue,
    map_item_value: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let key = unsafe { &*key };
    let map_item_value = unsafe { &*map_item_value };
    match &mut value.inner {
        AmqpValue::Map(v) => {
            v.insert(key.inner.clone(), map_item_value.inner.clone());
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_set_map_value(
    value: *mut RustAmqpValue,
    key: *mut RustAmqpValue,
    map_item_value: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let key = unsafe { &*key };
    let map_item_value = unsafe { &*map_item_value };
    match &mut value.inner {
        AmqpValue::Map(v) => {
            v.insert(key.inner.clone(), map_item_value.inner.clone());
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_map_item(
    value: *const RustAmqpValue,
    key: *mut RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let key = unsafe { &*key };
    match &value.inner {
        AmqpValue::Map(v) => {
            let amqp_value = RustAmqpValue {
                inner: v.get(key.inner.clone()).unwrap().clone(),
            };
            Box::into_raw(Box::new(amqp_value))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_map_key_value_pair(
    value: *const RustAmqpValue,
    index: u32,
    key: *mut *mut RustAmqpValue,
    map_item_value: *mut *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Map(v) => {
            let (k, m) = v.iter().nth(index as usize).unwrap();
            let amqp_key = RustAmqpValue { inner: k.clone() };
            let amqp_map_item = RustAmqpValue { inner: m.clone() };
            unsafe {
                *key = Box::into_raw(Box::new(amqp_key));
                *map_item_value = Box::into_raw(Box::new(amqp_map_item));
                0
            }
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_map_pair_count(
    value: *const RustAmqpValue,
    count: *mut u32,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Map(v) => unsafe {
            *count = v.len() as u32;
            0
        },
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_described(
    descriptor: *const RustAmqpValue,
    value: *const RustAmqpValue,
) -> *mut RustAmqpValue {
    let value = unsafe { &*value };
    let descriptor = unsafe { &*descriptor };
    let amqp_value = match &descriptor.inner {
        AmqpValue::ULong(v) => AmqpValue::Described(Box::new(AmqpDescribed::new(
            AmqpDescriptor::Code(*v),
            value.inner.clone(),
        ))),
        AmqpValue::Symbol(v) => AmqpValue::Described(Box::new(AmqpDescribed::new(
            AmqpDescriptor::Name(v.clone()),
            value.inner.clone(),
        ))),
        _ => AmqpValue::Null,
    };
    let amqp_value = RustAmqpValue { inner: amqp_value };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_inplace_descriptor(
    value: *const RustAmqpValue,
    descriptor: *mut *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Described(d) => {
            let amqp_value = RustAmqpValue {
                inner: match d.descriptor() {
                    AmqpDescriptor::Code(v) => AmqpValue::ULong(*v),
                    AmqpDescriptor::Name(v) => AmqpValue::Symbol(v.clone()),
                },
            };
            unsafe {
                *descriptor = Box::into_raw(Box::new(amqp_value));
            }
            0
        }
        AmqpValue::Composite(c) => {
            let amqp_value = RustAmqpValue {
                inner: match c.descriptor() {
                    AmqpDescriptor::Code(v) => AmqpValue::ULong(*v),
                    AmqpDescriptor::Name(v) => AmqpValue::Symbol(v.clone()),
                },
            };
            unsafe {
                *descriptor = Box::into_raw(Box::new(amqp_value));
            }
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_inplace_described_value(
    value: *const RustAmqpValue,
    described_value: *mut *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Described(d) => {
            let amqp_value = RustAmqpValue {
                inner: d.value().clone(),
            };
            unsafe {
                *described_value = Box::into_raw(Box::new(amqp_value));
            }
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_create_composite(
    descriptor: *const RustAmqpValue,
    size: usize,
) -> *mut RustAmqpValue {
    let descriptor = unsafe { &*descriptor };
    let amqp_value = match &descriptor.inner {
        AmqpValue::ULong(v) => AmqpValue::Composite(Box::new(AmqpComposite::new(
            AmqpDescriptor::Code(*v),
            AmqpList::with_capacity(size),
        ))),
        AmqpValue::Symbol(v) => AmqpValue::Composite(Box::new(AmqpComposite::new(
            AmqpDescriptor::Name(v.clone()),
            AmqpList::with_capacity(size),
        ))),
        AmqpValue::String(v) => AmqpValue::Composite(Box::new(AmqpComposite::new(
            AmqpDescriptor::Name(AmqpSymbol::from(v.clone())),
            AmqpList::with_capacity(size),
        ))),
        _ => AmqpValue::Null,
    };
    let amqp_value = RustAmqpValue { inner: amqp_value };
    Box::into_raw(Box::new(amqp_value))
}

#[no_mangle]
pub extern "C" fn amqpvalue_set_composite_item(
    value: *mut RustAmqpValue,
    index: u32,
    composite_item: *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &mut *value };
    let composite_item = unsafe { &*composite_item };

    if match &value.inner {
        AmqpValue::Composite(v) => v.value().0.capacity(),
        _ => return -1,
    } <= index as usize
    {
        return -1;
    }

    match &mut value.inner {
        AmqpValue::Composite(v) => v.value.0.resize((index + 1) as usize, AmqpValue::Null),
        _ => return -1,
    };

    match &mut value.inner {
        AmqpValue::Composite(v) => {
            v.value.0[index as usize] = composite_item.inner.clone();
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_composite_item_count(
    value: *const RustAmqpValue,
    count: *mut u32,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Composite(v) => unsafe {
            *count = v.value().len() as u32;
            0
        },
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_composite_item_in_place(
    value: *const RustAmqpValue,
    index: u32,
    composite_item: *mut *mut RustAmqpValue,
) -> i32 {
    let value = unsafe { &*value };
    match &value.inner {
        AmqpValue::Composite(v) => {
            let amqp_value = RustAmqpValue {
                inner: v.value().0[index as usize].clone(),
            };
            unsafe {
                *composite_item = Box::into_raw(Box::new(amqp_value));
            }
            0
        }
        _ => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_get_encoded_size(value: *const RustAmqpValue, size: *mut usize) -> u32 {
    let value = unsafe { &*value };
    let encoded_size = Serializable::encoded_size(&value.inner);
    if encoded_size.is_ok() {
        unsafe {
            *size = encoded_size.unwrap();
        }
        0
    } else {
        error!("Unable to compute encoded size: {:?}", encoded_size.err().unwrap());
        1
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_encode(
    value: *const RustAmqpValue,
    buffer: *mut u8,
    size: usize,
) -> i32 {
    let value = unsafe { &*value };
    let buffer = unsafe { std::slice::from_raw_parts_mut(buffer, size) };
    match Serializable::serialize(&value.inner, buffer) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn amqpvalue_decode_bytes(
    buffer: *const u8,
    size: usize,
    value: *mut *mut RustAmqpValue,
) -> i32 {
    let buffer = unsafe { std::slice::from_raw_parts(buffer, size) };
    match <AmqpValue as Deserializable<AmqpValue>>::decode(buffer) {
        Ok(v) => {
            let amqp_value = RustAmqpValue { inner: v };
            unsafe {
                *value = Box::into_raw(Box::new(amqp_value));
            }
            0
        }
        Err(e) => {
            println!("Decode error: {:?}", e);
            -1
        }
    }
}

#[test]
fn test_decode_bytes() {
    //    let encoded = vec![0xe0, 0x02, 0x00, 0x40];
    let value_to_encode = fe2o3_amqp_types::primitives::Value::Array(
        fe2o3_amqp_types::primitives::Array::from(vec![]),
    );
    let buffer = serde_amqp::to_vec(&value_to_encode).unwrap();

    //    let encoded = vec![0xe0, 1, 0x00];
    //    assert_eq!(buffer, encoded);

    let value: Result<fe2o3_amqp_types::primitives::Value, serde_amqp::error::Error> =
        serde_amqp::from_slice(buffer.as_slice());

    if value.is_err() {
        println!("Error: {:?}", value);
    }
    assert!(value.is_ok());

    match value.unwrap() {
        fe2o3_amqp_types::primitives::Value::Array(a) => assert_eq!(a.len(), 0),
        _ => panic!("Invalid value"),
    }

    //    assert_eq!(buffer, encoded);

    let value: Result<fe2o3_amqp_types::primitives::Value, serde_amqp::error::Error> =
        serde_amqp::from_slice(buffer.as_slice());

    assert!(value.is_ok());

    assert_eq!(value.unwrap(), value_to_encode);
}
