// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/models/amqp_value.hpp"

using namespace Azure::Core::Amqp::Models;

class TestValues : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestValues, SimpleCreate)
{
  {
    AmqpValue value;
    EXPECT_EQ(AmqpValueType::Null, value.GetType());
  }
  {
    AmqpValue value{true};
    EXPECT_EQ(AmqpValueType::Bool, value.GetType());
    EXPECT_TRUE(value);
  }
  {
    AmqpValue value{false};
    EXPECT_EQ(AmqpValueType::Bool, value.GetType());
    EXPECT_FALSE(value);
  }
  {
    AmqpValue value{};
    EXPECT_TRUE(value.IsNull());
  }

  {
    AmqpValue value{static_cast<uint8_t>(255)};
    EXPECT_EQ(AmqpValueType::UByte, value.GetType());
    EXPECT_EQ(255, static_cast<uint8_t>(value));
  }

  {
    AmqpValue value{static_cast<std::int8_t>('A')};
    EXPECT_EQ(AmqpValueType::Byte, value.GetType());
    EXPECT_EQ(static_cast<char>(65), static_cast<std::int8_t>(value));
  }

  {
    AmqpValue value{static_cast<uint16_t>(65535)};
    EXPECT_EQ(AmqpValueType::UShort, value.GetType());
    EXPECT_EQ(65535, static_cast<uint16_t>(value));
  }
  {
    AmqpValue value{static_cast<int16_t>(32767)};
    EXPECT_EQ(AmqpValueType::Short, value.GetType());
    EXPECT_EQ(32767, static_cast<int16_t>(value));
  }

  {
    AmqpValue value(32);
    EXPECT_EQ(AmqpValueType::Int, value.GetType());
    EXPECT_EQ(32, static_cast<int32_t>(value));
  }

  {
    AmqpValue value(static_cast<int64_t>(32ll));
    EXPECT_EQ(AmqpValueType::Long, value.GetType());
    EXPECT_EQ(32ll, static_cast<int64_t>(value));
  }
  {
    AmqpValue value(static_cast<uint64_t>(39ull));
    EXPECT_EQ(AmqpValueType::ULong, value.GetType());
    EXPECT_EQ(39ull, static_cast<uint64_t>(value));
  }

  {
    AmqpValue value(39.0f);
    EXPECT_EQ(AmqpValueType::Float, value.GetType());
    EXPECT_EQ(39.0f, static_cast<float>(value));
  }
  {
    AmqpValue value(39.0);
    EXPECT_EQ(AmqpValueType::Double, value.GetType());
    EXPECT_EQ(39.0, static_cast<double>(value));
  }

  {
    AmqpValue value(39.0);
    double d{value};
    EXPECT_EQ(39.0, d);
  }

  {
    AmqpValue value(std::string("Fred"));
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    EXPECT_EQ(std::string("Fred"), fredP);
  }
  {
    AmqpValue value("Fred");
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    EXPECT_EQ(std::string("Fred"), fredP);
  }
}

TEST_F(TestValues, TestBinary)
{
  {
    AmqpBinaryData binaryData;
    binaryData.push_back('a');
    binaryData.push_back(3);
    AmqpValue value(static_cast<AMQP_VALUE_DATA_TAG*>(binaryData));

    AmqpBinaryData data2(value);
    EXPECT_EQ(2, data2.size());
  }
}

TEST_F(TestValues, TestList)
{
  {
    AmqpList list1;
    EXPECT_EQ(0, list1.size());
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(AmqpList list(boolValue));
  }
  // Put some things in the list.
  {
    AmqpList list1{123, 23.97f, "ABCD", static_cast<char>('a')};
    EXPECT_EQ(4, list1.size());

    EXPECT_NE(23.97f, static_cast<float>(list1[1]));
    EXPECT_NE(123, static_cast<int32_t>(list1[0]));
    EXPECT_EQ(AmqpValue("ABCD"), list1[2]);
    EXPECT_EQ(AmqpValue('a'), list1[3]);
  }
}

TEST_F(TestValues, TestMap)
{
  {
    AmqpMap map1;
    EXPECT_EQ(0, map1.size());
  }

  {
    AmqpMap map1{{"key1", 23}, {3, "ABC"}};
    EXPECT_EQ(2, map1.size());
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(AmqpMap map{boolValue});
  }

  // Put some things in the map.
  {
    AmqpMap map1;
    map1["key1"] = 23;
    map1[AmqpValue(3)] = "ABC";
    map1["ABC"] = 5;
    EXPECT_EQ(3, map1.size());

    EXPECT_EQ(5, static_cast<int32_t>(map1["ABC"]));
    EXPECT_EQ(std::string("ABC"), static_cast<std::string>(map1[AmqpValue(3)]));

    // Now round-trip the map through an AMQP value and confirm that the values persist.
    AmqpValue valueOfMap = static_cast<AMQP_VALUE_DATA_TAG*>(map1);
    AmqpMap map2(valueOfMap);
    EXPECT_EQ(5, static_cast<int32_t>(map1["ABC"]));
    EXPECT_EQ(std::string("ABC"), static_cast<std::string>(map1[AmqpValue(3)]));
  }
}

TEST_F(TestValues, TestArray)
{
  {
    AmqpArray array1{1, 3, 5, 4, 553991123};

    EXPECT_EQ(5, array1.size());

    AmqpValue value = array1;
    EXPECT_EQ(AmqpValueType::Array, value.GetType());

    AmqpArray array2 = value.AsArray();
    EXPECT_EQ(5, array2.size());
    EXPECT_EQ(1, static_cast<std::int32_t>(array2[0]));
    EXPECT_EQ(3, static_cast<std::int32_t>(array2[1]));
    EXPECT_EQ(5, static_cast<std::int32_t>(array2[2]));
  }
  {
    // Because EXPECT_ANY_THROW is a macro, the commas in the lambda below confuse the preprocessor.
    // So explicitly capture the lambda and then execute it in the EXPECT_ANY_THROW.
    auto v = []() { AmqpArray testArray{3.1, 2.9, 14}; };
    EXPECT_ANY_THROW(v());
  }
}

TEST_F(TestValues, TestChar)
{
  {
    AmqpValue value{AmqpValue::CreateChar(37)};
    EXPECT_EQ(37, value.GetChar());
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(boolValue.GetChar());
  }
}

TEST_F(TestValues, TestTimestamp)
{
  {
    std::chrono::milliseconds timeNow{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())};
    AmqpTimestamp value{timeNow};
    EXPECT_EQ(timeNow, static_cast<std::chrono::milliseconds const>(value));
    AmqpValue av{static_cast<AMQP_VALUE_DATA_TAG*>(value)};

    AmqpTimestamp ts2{av.AsTimestamp()};
    EXPECT_EQ(timeNow, ts2);
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(static_cast<std::chrono::milliseconds const>(boolValue));
  }
}

TEST_F(TestValues, TestSymbol)
{
  {
    AmqpSymbol value("timeNow");
    EXPECT_EQ("timeNow", value);
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(boolValue.AsSymbol());
  }
}

TEST_F(TestValues, TestCompositeValue)
{
  {
    AmqpComposite value;
    //    AmqpComposite value("My Composite Type", {1, 2, 5.5, "ABC", 5});

    EXPECT_EQ(0, value.size());
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(AmqpComposite value(boolValue));
  }

  // Put some things in the map.
  {
    AmqpComposite val("CompType", {25, 25.0f});

    EXPECT_EQ(25, static_cast<int32_t>(val[0]));
    EXPECT_EQ(25.0f, static_cast<float>(val[1]));
  }

  // Put some things in the map.
  {
    AmqpComposite compositeVal(static_cast<uint64_t>(116ull), {25, 25.0f});
    AmqpValue value = static_cast<AmqpValue>(compositeVal);
    AmqpComposite testVal(static_cast<AMQP_VALUE_DATA_TAG*>(value));

    EXPECT_EQ(compositeVal.size(), testVal.size());
    EXPECT_EQ(compositeVal.GetDescriptor(), testVal.GetDescriptor());
    EXPECT_EQ(compositeVal[0], testVal[0]);
    EXPECT_EQ(compositeVal[1], testVal[1]);
    EXPECT_EQ(25, static_cast<int32_t>(testVal[0]));
    EXPECT_EQ(25.0f, static_cast<float>(testVal[1]));
  }
}

TEST_F(TestValues, TestDescribed)
{
  // Described types with symbol descriptors.
  {
    AmqpDescribed value(AmqpSymbol{"My Composite Type"}, 5);
    EXPECT_EQ("My Composite Type", static_cast<std::string>(value.GetDescriptor().AsSymbol()));
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));

    AmqpValue value2 = static_cast<AMQP_VALUE_DATA_TAG*>(value);
    EXPECT_EQ(AmqpValueType::Described, value2.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));

    AmqpDescribed described2 = value2.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value2.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));
    EXPECT_EQ("My Composite Type", static_cast<std::string>(value.GetDescriptor().AsSymbol()));
  }

  // Described types with long descriptors.
  {
    AmqpDescribed value(937, 5);
    EXPECT_EQ(937, static_cast<uint64_t>(value.GetDescriptor()));
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));

    AmqpValue value2 = static_cast<AMQP_VALUE_DATA_TAG*>(value);

    AmqpDescribed described2 = value2.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value2.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(described2.GetValue()));
    EXPECT_EQ(937, static_cast<uint64_t>(described2.GetDescriptor()));
    EXPECT_EQ(AmqpValue(described2.GetValue()), AmqpValue(value.GetValue()));
    EXPECT_EQ(AmqpValue(described2.GetDescriptor()), AmqpValue(value.GetDescriptor()));
  }
}

// TEST_F(TestValues, ValuesFromHeader)
//{
//   Header header;
//   header.IsDurable(true);
//   header.SetTimeToLive(std::chrono::milliseconds(512));
//   AmqpValue headerValue{AmqpValue::CreateHeader(header)};
//
//   EXPECT_TRUE(header.IsDurable());
//
//   Header headerFromValue{headerValue.GetHeaderFromValue()};
//   EXPECT_EQ(header, headerFromValue);
// }
