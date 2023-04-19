// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/models/amqp_value.hpp"
#include <cuchar>

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
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value{};
    EXPECT_TRUE(value.IsNull());
  }

  {
    AmqpValue value{static_cast<uint8_t>(255)};
    EXPECT_EQ(AmqpValueType::UByte, value.GetType());
    EXPECT_EQ(255, static_cast<uint8_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value{'A'};
    EXPECT_EQ(AmqpValueType::Byte, value.GetType());
    EXPECT_EQ(static_cast<char>(65), static_cast<std::int8_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    char ch{value};
    EXPECT_EQ('A', ch);
  }

  {
    AmqpValue value{static_cast<uint16_t>(65535)};
    EXPECT_EQ(AmqpValueType::UShort, value.GetType());
    EXPECT_EQ(65535, static_cast<uint16_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value{static_cast<int16_t>(32767)};
    EXPECT_EQ(AmqpValueType::Short, value.GetType());
    EXPECT_EQ(32767, static_cast<int16_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value(32);
    EXPECT_EQ(AmqpValueType::Int, value.GetType());
    EXPECT_EQ(32, static_cast<int32_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value(32u);
    EXPECT_EQ(AmqpValueType::UInt, value.GetType());
    EXPECT_EQ(32u, static_cast<uint32_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value(static_cast<int64_t>(32ll));
    EXPECT_EQ(AmqpValueType::Long, value.GetType());
    EXPECT_EQ(32ll, static_cast<int64_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value(static_cast<uint64_t>(39ull));
    EXPECT_EQ(AmqpValueType::ULong, value.GetType());
    EXPECT_EQ(39ull, static_cast<uint64_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value(39.0f);
    EXPECT_EQ(AmqpValueType::Float, value.GetType());
    EXPECT_EQ(39.0f, static_cast<float>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value(39.0);
    EXPECT_EQ(AmqpValueType::Double, value.GetType());
    EXPECT_EQ(39.0, static_cast<double>(value));
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value(39.0);
    double d{value};
    EXPECT_EQ(39.0, d);
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    AmqpValue value(std::string("Fred"));
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    EXPECT_EQ(std::string("Fred"), fredP);
    EXPECT_TRUE(AmqpValue() < value);
  }
  {
    AmqpValue value("Fred");
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    EXPECT_EQ(std::string("Fred"), fredP);
    EXPECT_TRUE(AmqpValue() < value);
  }

  {
    Azure::Core::Uuid uuid = Azure::Core::Uuid::CreateUuid();
    AmqpValue value(uuid);
    EXPECT_EQ(AmqpValueType::Uuid, value.GetType());
    EXPECT_EQ(uuid.ToString(), static_cast<Azure::Core::Uuid>(value).ToString());
    EXPECT_TRUE(AmqpValue() < value);
  }
}

TEST_F(TestValues, TestBinary)
{
  {
    AmqpBinaryData binaryData;
    binaryData.push_back('a');
    binaryData.push_back(3);
    AmqpValue value(static_cast<UniqueAmqpValueHandle>(binaryData).get());

    AmqpBinaryData data2(value);
    EXPECT_EQ(2, data2.size());
    EXPECT_TRUE(AmqpValue() < value);
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
    const AmqpList list1{123, 23.97f, "ABCD", 'a'};
    EXPECT_EQ(4, list1.size());

    EXPECT_EQ(23.97f, static_cast<float>(list1.at(1)));
    EXPECT_EQ(123, static_cast<int32_t>(list1.at(0)));
    EXPECT_EQ(AmqpValue("ABCD"), list1.at(2));
    EXPECT_EQ(AmqpValueType::Byte, list1[3].GetType());
    EXPECT_EQ(AmqpValue('a'), list1[3]);

    AmqpValue value(static_cast<UniqueAmqpValueHandle>(list1).get());
    const AmqpList list2(value);

    EXPECT_EQ(4, list2.size());

    EXPECT_EQ(23.97f, static_cast<float>(list2.at(1)));
    EXPECT_EQ(123, static_cast<int32_t>(list2.at(0)));
    EXPECT_EQ(AmqpValue("ABCD"), list2.at(2));
    EXPECT_EQ(AmqpValue('a'), list2.at(3));
    EXPECT_FALSE(list1 < list2);
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
    AmqpValue valueOfMap = static_cast<UniqueAmqpValueHandle>(map1).get();
    AmqpMap map2(valueOfMap);
    EXPECT_EQ(5, static_cast<int32_t>(map2["ABC"]));
    EXPECT_EQ(std::string("ABC"), static_cast<std::string>(map2[AmqpValue(3)]));
    EXPECT_FALSE(map1 < map2);
  }
}

TEST_F(TestValues, TestArray)
{
  {
    AmqpArray array1{1, 3, 5, 4, 553991123};

    EXPECT_EQ(5, array1.size());

    AmqpValue value = array1;
    EXPECT_EQ(AmqpValueType::Array, value.GetType());

    const AmqpArray array2 = value.AsArray();
    EXPECT_EQ(5, array2.size());
    EXPECT_EQ(1, static_cast<std::int32_t>(array2.at(0)));
    EXPECT_EQ(3, static_cast<std::int32_t>(array2.at(1)));
    EXPECT_EQ(5, static_cast<std::int32_t>(array2.at(2)));
    EXPECT_FALSE(array1 < array2);
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
    AmqpValue value{U'\U0001f34c'};
    EXPECT_EQ(U'\U0001f34c', static_cast<char32_t>(value));
    EXPECT_FALSE(static_cast<char32_t>(value) < U'\U0001f34c');
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(static_cast<char32_t>(boolValue));
  }
}

TEST_F(TestValues, TestTimestamp)
{
  {
    std::chrono::milliseconds timeNow{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())};
    AmqpTimestamp value{timeNow};
    EXPECT_EQ(static_cast<std::chrono::milliseconds>(value), timeNow);
    AmqpValue av{value};

    AmqpTimestamp ts2{av.AsTimestamp()};
    EXPECT_EQ(timeNow, static_cast<std::chrono::milliseconds>(ts2));
    EXPECT_FALSE(value < ts2);
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(boolValue.AsTimestamp());
  }
}

TEST_F(TestValues, TestSymbol)
{
  {
    AmqpSymbol value("timeNow");
    EXPECT_EQ(value, "timeNow");
    EXPECT_FALSE(value < AmqpSymbol("timeNow"));
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(boolValue.AsSymbol());
  }
}

TEST_F(TestValues, TestCompositeValue)
{
  {
    AmqpComposite value("My Composite Type", {1, 2, 5.5, "ABC", 5});

    EXPECT_EQ(5, value.size());
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW(AmqpComposite value(boolValue));
  }

  // Put some things in the map.
  {
    AmqpComposite val("CompType", {25, 25.0f});

    EXPECT_EQ(25, static_cast<int32_t>(val.at(0)));
    EXPECT_EQ(25.0f, static_cast<float>(val.at(1)));
  }

  // Put some things in the map.
  {
    AmqpComposite compositeVal(static_cast<uint64_t>(116ull), {25, 25.0f});
    AmqpValue value = compositeVal;
    AmqpComposite testVal(value.AsComposite());

    EXPECT_EQ(compositeVal.size(), testVal.size());
    EXPECT_EQ(compositeVal.GetDescriptor(), testVal.GetDescriptor());
    EXPECT_EQ(compositeVal.at(0), testVal.at(0));
    EXPECT_EQ(compositeVal.at(1), testVal.at(1));
    EXPECT_EQ(25, static_cast<int32_t>(testVal.at(0)));
    EXPECT_EQ(25.0f, static_cast<float>(testVal.at(1)));
    EXPECT_FALSE(compositeVal < testVal);
  }
}

TEST_F(TestValues, TestDescribed)
{
  // Described types with symbol descriptors.
  {
    AmqpDescribed described1(AmqpSymbol{"My Composite Type"}, 5);
    EXPECT_EQ(AmqpSymbol("My Composite Type"), described1.GetDescriptor().AsSymbol());
    EXPECT_EQ(5, static_cast<int32_t>(described1.GetValue()));

    AmqpValue value = described1;
    EXPECT_EQ(AmqpValueType::Described, value.GetType());

    AmqpDescribed described2 = value.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(described2.GetValue()));
    EXPECT_EQ(described2.GetDescriptor().AsSymbol(), "My Composite Type");
    EXPECT_FALSE(described1 < described2);
    //    EXPECT_TRUE(described1 == described2);
  }

  // Described types with long descriptors.
  {
    AmqpDescribed value(937, 5);
    EXPECT_EQ(937, static_cast<uint64_t>(value.GetDescriptor()));
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));

    AmqpValue value2 = value;

    AmqpDescribed described2 = value2.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value2.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(described2.GetValue()));
    EXPECT_EQ(937, static_cast<uint64_t>(described2.GetDescriptor()));
    EXPECT_EQ(AmqpValue(described2.GetValue()), AmqpValue(value.GetValue()));
    EXPECT_EQ(AmqpValue(described2.GetDescriptor()), AmqpValue(value.GetDescriptor()));
  }
}
