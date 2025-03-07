// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/value_impl.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqpvalue.h>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models::_internal;
using namespace Azure::Core::Amqp::Models::_detail;
using namespace Azure::Core::Amqp::Models;

class TestValue : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

#define TEST_C_INSERTER(VALUE, ENUMERATOR) \
  { \
    std::stringstream ss; \
    ss << amqpvalue_get_type(VALUE); \
    EXPECT_EQ(#ENUMERATOR, ss.str()); \
  }

TEST_F(TestValue, SimpleCreate)
{
  {
    AmqpValue value;
  }

  GTEST_LOG_(INFO) << AMQP_TYPE_INVALID;
  GTEST_LOG_(INFO) << AMQP_TYPE_UNKNOWN;
  GTEST_LOG_(INFO) << static_cast<AMQP_VALUE>(nullptr);

  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_null()};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_NULL);
  }

  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_byte('q')};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_BYTE);
  }

  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_boolean, bool, bool_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_boolean(true)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_BOOL);
  }
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_boolean(false)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_BOOL);
  }

  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ubyte, unsigned char, ubyte_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_ubyte(225)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_UBYTE);
  }

  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ushort, uint16_t, ushort_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_ushort(32769)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_USHORT);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uint, uint32_t, uint_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_uint(1235125)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_UINT);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ulong, uint64_t, ulong_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_ulong(13421266651)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_ULONG);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_byte, char, byte_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_byte('q')};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_BYTE);
  }

  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_short, int16_t, short_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_short(225)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_SHORT);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_int, int32_t, int_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_int(1151551)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_INT);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_long, int64_t, long_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_long(1551516661161)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_LONG);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_float, float, float_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_float(16.5)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_FLOAT);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_double, double, double_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_double(100515.021)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_DOUBLE);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_char, uint32_t, char_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_char('9')};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_CHAR);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_timestamp, int64_t, timestamp_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_timestamp(1569)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_TIMESTAMP);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uuid, uuid, uuid_value);
  {
    Azure::Core::Uuid uuid = Azure::Core::Uuid::CreateUuid();
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_uuid(
        reinterpret_cast<unsigned char*>(const_cast<uint8_t*>(uuid.AsArray().data())))};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_UUID);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_binary, amqp_binary, binary_value);
  {
    amqp_binary binary;
    binary.bytes = reinterpret_cast<const unsigned char*>("Hello World");
    binary.length = 11;
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_binary(binary)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_BINARY);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_string, const char*, string_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_string("binary")};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_STRING);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_symbol, const char*, symbol_value);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_symbol("binary")};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_SYMBOL);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_list);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_list()};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_LIST);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_map);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_map()};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_MAP);
  }
  // MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_array);
  {
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_array()};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: " << amqpvalue_get_type(handle.get())
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_ARRAY);
  }

  {
    AmqpValue descriptor(static_cast<uint64_t>(237ll));
    AmqpValue descriptorValue("Value");
    _detail::UniqueAmqpValueHandle handle{amqpvalue_create_described(
        amqpvalue_clone(_detail::AmqpValueFactory::ToImplementation(descriptor)),
        amqpvalue_clone(_detail::AmqpValueFactory::ToImplementation(descriptorValue)))};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: "
                     << amqpvalue_get_type(amqpvalue_get_inplace_descriptor(handle.get()))
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_DESCRIBED);
  }
  {
    AmqpValue descriptor(static_cast<uint64_t>(235ll));
    _detail::UniqueAmqpValueHandle handle{
        amqpvalue_create_composite(_detail::AmqpValueFactory::ToImplementation(descriptor), 21)};

    AmqpValue value{_detail::AmqpValueFactory::FromImplementation(handle)};
    GTEST_LOG_(INFO) << "Handle Type: "
                     << amqpvalue_get_type(amqpvalue_get_inplace_descriptor(handle.get()))
                     << " Value: " << static_cast<AMQP_VALUE>(handle.get());
    TEST_C_INSERTER(handle.get(), AMQP_TYPE_COMPOSITE);
  }
}
