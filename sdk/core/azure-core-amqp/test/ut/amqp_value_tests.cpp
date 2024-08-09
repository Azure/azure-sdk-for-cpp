// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure/core/amqp/internal/common/global_state.hpp>

#include <algorithm>
#include <random>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;

class TestValues : public testing::Test {
protected:
  void SetUp() override
  {
    // Ensure that our logger is hooked up to global state.
    auto globalInstance
        = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
    (void)globalInstance;
  }
  void TearDown() override {}
};

#define TEST_OSTREAM_INSERTER(VALUE, EXPECTED) \
  { \
    std::stringstream ss; \
    ss << VALUE.GetType(); \
    EXPECT_EQ(EXPECTED, ss.str()); \
  }

TEST_F(TestValues, SimpleCreateNull)
{
  {
    AmqpValue value;
    EXPECT_EQ(AmqpValueType::Null, value.GetType());
  }
  {
    AmqpValue value{};
    EXPECT_TRUE(value.IsNull());
    TEST_OSTREAM_INSERTER(value, "Null");
  }
}
TEST_F(TestValues, SimpleCreateBool)
{
  {
    AmqpValue value{true};
    EXPECT_EQ(AmqpValueType::Bool, value.GetType());
    EXPECT_TRUE(value);
    TEST_OSTREAM_INSERTER(value, "Bool");
  }
  {
    AmqpValue value{false};
    EXPECT_EQ(AmqpValueType::Bool, value.GetType());
    EXPECT_FALSE(value);
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<char>(value));
    EXPECT_ANY_THROW((void)static_cast<std::int8_t>(value));
    TEST_OSTREAM_INSERTER(value, "Bool");
  }
  {
    EXPECT_LT(AmqpValue(false), AmqpValue(true));
  }
}
TEST_F(TestValues, SimpleCreateByte)
{

  {
    AmqpValue value{static_cast<int8_t>(-17)};
    EXPECT_EQ(AmqpValueType::Byte, value.GetType());
    EXPECT_EQ(-17, static_cast<int8_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    TEST_OSTREAM_INSERTER(value, "Byte");
    EXPECT_LT(AmqpValue{static_cast<int8_t>(-18)}, value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_ANY_THROW((void)static_cast<unsigned char>(value));
    EXPECT_ANY_THROW((void)static_cast<uint16_t>(value));
    EXPECT_ANY_THROW((void)static_cast<int16_t>(value));
    EXPECT_ANY_THROW((void)static_cast<uint32_t>(value));
    EXPECT_ANY_THROW((void)static_cast<int32_t>(value));
    EXPECT_ANY_THROW((void)static_cast<uint64_t>(value));
    EXPECT_ANY_THROW((void)static_cast<int64_t>(value));
    EXPECT_ANY_THROW((void)static_cast<float>(value));
    EXPECT_ANY_THROW((void)static_cast<double>(value));
    EXPECT_ANY_THROW((void)static_cast<std::string>(value));
    EXPECT_ANY_THROW((void)static_cast<Azure::Core::Uuid>(value));
  }
  {
    AmqpValue value{'D'};
    EXPECT_EQ(AmqpValueType::Byte, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Byte");
    EXPECT_EQ(static_cast<char>(68), static_cast<std::int8_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    char ch{value};
    EXPECT_EQ('D', ch);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue('B'), value);
  }
}
TEST_F(TestValues, SimpleCreateUByte)
{

  {
    AmqpValue value{static_cast<uint8_t>(255)};
    EXPECT_EQ(AmqpValueType::Ubyte, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Ubyte");
    EXPECT_EQ(255, static_cast<uint8_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue{static_cast<uint8_t>(254)}, value);
  }
}
TEST_F(TestValues, SimpleCreateUShort)
{
  {
    AmqpValue value{static_cast<uint16_t>(65535)};
    EXPECT_EQ(AmqpValueType::Ushort, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Ushort");
    EXPECT_EQ(65535, static_cast<uint16_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue{static_cast<uint16_t>(65534)}, value);
  }
}
TEST_F(TestValues, SimpleCreateShort)
{
  {
    AmqpValue value{static_cast<int16_t>(32767)};
    EXPECT_EQ(AmqpValueType::Short, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Short");
    EXPECT_EQ(32767, static_cast<int16_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue{static_cast<int16_t>(32766)}, value);
  }
}
TEST_F(TestValues, SimpleCreateInt)
{
  {
    AmqpValue value(32);
    EXPECT_EQ(AmqpValueType::Int, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Int");
    EXPECT_EQ(32, static_cast<int32_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(31), value);
  }
}
TEST_F(TestValues, SimpleCreateUInt)
{
  {
    AmqpValue value(32u);
    EXPECT_EQ(AmqpValueType::Uint, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Uint");
    EXPECT_EQ(32u, static_cast<uint32_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(31u), value);
  }
}
TEST_F(TestValues, SimpleCreateLong)
{

  {
    AmqpValue value(static_cast<int64_t>(32ll));
    EXPECT_EQ(AmqpValueType::Long, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Long");
    EXPECT_EQ(32ll, static_cast<int64_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(static_cast<int64_t>(31ll)), value);
  }
}
TEST_F(TestValues, SimpleCreateULong)
{
  {
    AmqpValue value(static_cast<uint64_t>(39ull));
    EXPECT_EQ(AmqpValueType::Ulong, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Ulong");
    EXPECT_EQ(39ull, static_cast<uint64_t>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(static_cast<uint64_t>(38ull)), value);
  }
}
TEST_F(TestValues, SimpleCreateFloat)
{

  {
    AmqpValue value(39.0f);
    EXPECT_EQ(AmqpValueType::Float, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Float");
    EXPECT_EQ(39.0f, static_cast<float>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(38.0f), value);
  }
}
TEST_F(TestValues, SimpleCreateDouble)
{

  {
    AmqpValue value(39.0);
    EXPECT_EQ(AmqpValueType::Double, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Double");
    EXPECT_EQ(39.0, static_cast<double>(value));
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue(38.0), value);
  }

  {
    AmqpValue value(39.0);
    TEST_OSTREAM_INSERTER(value, "Double");
    double d{value};
    EXPECT_EQ(39.0, d);
    EXPECT_TRUE(AmqpValue() < value);
  }
}
TEST_F(TestValues, SimpleCreateString)
{

  {
    AmqpValue value(std::string("Fred"));
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    TEST_OSTREAM_INSERTER(value, "String");
    EXPECT_EQ(std::string("Fred"), fredP);
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
    EXPECT_LT(AmqpValue("ABC"), value);
  }
  {
    AmqpValue value("Fred");
    std::string fredP(value);
    EXPECT_EQ(AmqpValueType::String, value.GetType());
    TEST_OSTREAM_INSERTER(value, "String");
    EXPECT_EQ(std::string("Fred"), fredP);
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
  }
}
TEST_F(TestValues, SimpleCreateUuid)
{
  {
    Azure::Core::Uuid uuid = Azure::Core::Uuid::CreateUuid();
    AmqpValue value(uuid);
    EXPECT_EQ(AmqpValueType::Uuid, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Uuid");
    EXPECT_EQ(uuid.ToString(), static_cast<Azure::Core::Uuid>(value).ToString());
    EXPECT_TRUE(AmqpValue() < value);
    EXPECT_ANY_THROW((void)static_cast<bool>(value));
  }

}
TEST_F(TestValues, MoveAmqpValues)
{
  {
    AmqpValue value1{29};
    AmqpValue value2(std::move(value1));
    AmqpValue value3(value2);
    AmqpValue value4;
    value4 = value2;
    EXPECT_EQ(value4, value2);
    GTEST_LOG_(INFO) << value4;
    AmqpValue value5 = std::move(value3);
    GTEST_LOG_(INFO) << value5;
    EXPECT_NE(value5, value3);
  }
}

TEST_F(TestValues, TestBinary)
{
  {
    AmqpBinaryData binaryData;
    binaryData.push_back('a');
    binaryData.push_back(3);
    AmqpValue value{binaryData.AsAmqpValue()};
    EXPECT_EQ(AmqpValueType::Binary, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Binary");

    EXPECT_FALSE(value < binaryData.AsAmqpValue());

    AmqpBinaryData data2(value);
    EXPECT_EQ(2, data2.size());
    EXPECT_TRUE(AmqpValue() < value);

    AmqpBinaryData data3(std::vector<uint8_t>(50));

    GTEST_LOG_(INFO) << "data3.size()=" << data3.size();
    GTEST_LOG_(INFO) << "data3:" << data3;
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

    AmqpValue value{list1.AsAmqpValue()};
    TEST_OSTREAM_INSERTER(value, "List");
    const AmqpList list2(value);

    EXPECT_FALSE(value < list1.AsAmqpValue());

    EXPECT_EQ(4, list2.size());

    EXPECT_EQ(23.97f, static_cast<float>(list2.at(1)));
    EXPECT_EQ(123, static_cast<int32_t>(list2.at(0)));
    EXPECT_EQ(AmqpValue("ABCD"), list2.at(2));
    EXPECT_EQ(AmqpValue('a'), list2.at(3));
    EXPECT_FALSE(list1 < list2);
  }

  {
    AmqpList test;
    AmqpDescribed desc{
        static_cast<uint64_t>(29),
        AmqpList{AmqpValue{"test:error"}, AmqpValue{"test description"}}.AsAmqpValue()};
    test.push_back(desc.AsAmqpValue());
    EXPECT_EQ(1, test.size());
    EXPECT_EQ(AmqpValueType::Described, test[0].GetType());
    TEST_OSTREAM_INSERTER(test[0], "Described");

    AmqpList list2{test};
    EXPECT_EQ(1, list2.size());
    EXPECT_EQ(AmqpValueType::Described, list2[0].GetType());
    AmqpDescribed desc2{list2[0].AsDescribed()};
    EXPECT_EQ(desc2.GetDescriptor(), AmqpValue{static_cast<uint64_t>(29ll)});
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
    AmqpValue valueOfMap = map1.AsAmqpValue();
    TEST_OSTREAM_INSERTER(valueOfMap, "Map");
    AmqpMap map2(valueOfMap);
    EXPECT_FALSE(valueOfMap < map1.AsAmqpValue());

    EXPECT_EQ(5, static_cast<int32_t>(map2["ABC"]));
    EXPECT_EQ(std::string("ABC"), static_cast<std::string>(map2[AmqpValue(3)]));
    EXPECT_FALSE(map1 < map2);
  }
}

TEST_F(TestValues, TestArray)
{
  AmqpArray array1{1, 3, 5, 4, 553991123};

  EXPECT_EQ(5, array1.size());

  AmqpValue value = array1.AsAmqpValue();
  EXPECT_EQ(AmqpValueType::Array, value.GetType());
  TEST_OSTREAM_INSERTER(value, "Array");

  const AmqpArray array2 = value.AsArray();
  EXPECT_EQ(5, array2.size());
  EXPECT_EQ(1, static_cast<std::int32_t>(array2.at(0)));
  EXPECT_EQ(3, static_cast<std::int32_t>(array2.at(1)));
  EXPECT_EQ(5, static_cast<std::int32_t>(array2.at(2)));
  EXPECT_FALSE(array1 < array2);
  {
    EXPECT_FALSE(value < array2.AsAmqpValue());
  }
}

TEST_F(TestValues, TestArrayDifferentTypes)
{
  // Because EXPECT_ANY_THROW is a macro, the commas in the lambda below confuse the
  // preprocessor. So explicitly capture the lambda and then execute it in the EXPECT_ANY_THROW.
  auto v = []() { AmqpArray testArray{3.1, 2.9, 14}; };
  EXPECT_ANY_THROW(v());
}

TEST_F(TestValues, TestChar)
{
  {
    AmqpValue value{U'\U0001f34c'};
    TEST_OSTREAM_INSERTER(value, "Char");
    EXPECT_EQ(U'\U0001f34c', static_cast<char32_t>(value));
    EXPECT_EQ(AmqpValueType::Char, value.GetType());
    EXPECT_FALSE(static_cast<char32_t>(value) < U'\U0001f34c');
  }
  {
    AmqpValue boolValue{false};
    EXPECT_ANY_THROW((void)static_cast<char32_t>(boolValue));
  }
}

TEST_F(TestValues, TestTimestamp)
{
  {
    std::chrono::milliseconds timeNow{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())};
    AmqpTimestamp value{timeNow};
    EXPECT_EQ(static_cast<std::chrono::milliseconds>(value), timeNow);
    AmqpValue av{value.AsAmqpValue()};
    TEST_OSTREAM_INSERTER(av, "Timestamp");

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
    AmqpValue av{value.AsAmqpValue()};
    TEST_OSTREAM_INSERTER(av, "Symbol");
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
    EXPECT_EQ(AmqpValueType::Composite, value.AsAmqpValue().GetType());

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

  {
    AmqpComposite composite1{123, {"StringValue"}};
    AmqpComposite composite2{456, {"StringValue"}};
    EXPECT_NE(composite1, composite2);
  }

  // Put some things in the map.
  {
    AmqpComposite compositeVal(static_cast<uint64_t>(116ull), {25, 25.0f});
    AmqpValue value = compositeVal.AsAmqpValue();
    TEST_OSTREAM_INSERTER(value, "Composite");
    AmqpComposite testVal(value.AsComposite());

    EXPECT_EQ(compositeVal.size(), testVal.size());
    EXPECT_EQ(compositeVal.GetDescriptor(), testVal.GetDescriptor());
    EXPECT_EQ(compositeVal[0], testVal[0]);
    EXPECT_EQ(compositeVal[1], testVal[1]);
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

    AmqpValue value = described1.AsAmqpValue();
    EXPECT_EQ(AmqpValueType::Described, value.GetType());
    TEST_OSTREAM_INSERTER(value, "Described");

    AmqpDescribed described2 = value.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(described2.GetValue()));
    EXPECT_EQ(described2.GetDescriptor().AsSymbol(), "My Composite Type");
    EXPECT_FALSE(described1 < described2);
    EXPECT_TRUE(described1 == described2);
  }

  // Described types with long descriptors.
  {
    AmqpDescribed value(937, 5);
    EXPECT_EQ(937, static_cast<uint64_t>(value.GetDescriptor()));
    EXPECT_EQ(5, static_cast<int32_t>(value.GetValue()));

    AmqpValue value2 = value.AsAmqpValue();

    AmqpDescribed described2 = value2.AsDescribed();
    EXPECT_EQ(AmqpValueType::Described, value2.GetType());
    EXPECT_EQ(5, static_cast<int32_t>(described2.GetValue()));
    EXPECT_EQ(937, static_cast<uint64_t>(described2.GetDescriptor()));
    EXPECT_EQ(AmqpValue(described2.GetValue()), AmqpValue(value.GetValue()));
    EXPECT_EQ(AmqpValue(described2.GetDescriptor()), AmqpValue(value.GetDescriptor()));
  }

  {
    AmqpDescribed described1{123, {"StringValue"}};
    AmqpDescribed described2{456, {"StringValue"}};
    EXPECT_NE(described1, described2);
  }
}

class TestValueSerialization : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// AMQP values are serialized as described in the AMQP spec.

//  Test deserializing a null value (0x40) - section 1.6.1.
TEST_F(TestValueSerialization, SerializeNull)
{
  std::vector<uint8_t> testVector{0x40};
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Null);

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(1, val.size());
  EXPECT_EQ(0x40, val[0]);
}

//  Test deserializing a boolean value (0x56/0x00 or 0x56/0x01) - section 1.6.2.
TEST_F(TestValueSerialization, SerializeBoolean)
{
  // There are two possible encodings for Boolean values: 0x56 followed by a byte with the value
  // 0x00 for false, or 0x56 followed by a byte with the value 0x01 for true.
  // The other possible encoding for Boolean values is 0x41 for true and 0x42 for false.
  {
    std::vector<uint8_t> testVector{0x56, 0x01};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Bool);
    EXPECT_TRUE(value);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x41, val[0]);
  }
  {
    std::vector<uint8_t> testVector{0x56, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Bool);
    EXPECT_FALSE(value);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x42, val[0]);
  }

  {
    std::vector<uint8_t> testVector{0x41};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Bool);
    EXPECT_TRUE(value);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x41, val[0]);
  }
  {
    std::vector<uint8_t> testVector{0x42};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Bool);
    EXPECT_FALSE(value);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x42, val[0]);
  }
}

//  Test deserializing a UByte value (0x50/0xXX) - section 1.6.3.
TEST_F(TestValueSerialization, SerializeUbyte)
{
  {
    std::vector<uint8_t> testVector{0x50, 0x25};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ubyte);
    EXPECT_EQ(0x25, static_cast<uint8_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x50, val[0]);
    EXPECT_EQ(0x25, val[1]);
  }

  {
    std::vector<uint8_t> testVector{0x50, 0x89};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ubyte);
    EXPECT_EQ(0x89, static_cast<uint8_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x50, val[0]);
    EXPECT_EQ(0x89, val[1]);
  }
}

//  Test deserializing a UShort value (0x60/0xXX/0xXX) - section 1.6.4.
// Note: Serialized value is in network byte order.
TEST_F(TestValueSerialization, SerializeUShort)
{
  {
    std::vector<uint8_t> testVector{0x60, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ushort);
    EXPECT_EQ(0, static_cast<std::uint16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }

  {
    std::vector<uint8_t> testVector{0x60, 0x04, 0x00};

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ushort);
    EXPECT_EQ(0x400, static_cast<std::uint16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }

  {
    std::vector<uint8_t> testVector{0x60, 0x04, 0x80};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ushort);
    EXPECT_EQ(0x480, static_cast<std::uint16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a UInt value (0x70/0xXX/0xXX) - section 1.6.5.
// Note that there are three serializations for UInt values:
// The first is as a fixed width value in the form of 0x70/0xXX/0xYY/0xZZ/0xAA with the values in
// network byte order.
// The second applies to values in the range 0..255: 0x52/0xXX
// The third applies to the specific value of 0: 0x43.
TEST_F(TestValueSerialization, SerializeUint)
{
  {
    // Input first form with value == 0. Expected output: 3rd form.
    std::vector<uint8_t> testVector{0x70, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Uint);
    EXPECT_EQ(0, static_cast<std::uint32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x43, val[0]);
  }
  {
    // Third form, value == 0.
    std::vector<uint8_t> testVector{0x43};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Uint);
    EXPECT_EQ(0, static_cast<std::uint32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x43, val[0]);
  }
  {
    // First form, value < 255, expected output: 2nd form.
    std::vector<uint8_t> testVector{0x70, 0x00, 0x00, 0x00, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Uint);
    EXPECT_EQ(0x85, static_cast<std::uint32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x52, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x52, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Uint);
    EXPECT_EQ(0x85, static_cast<std::uint32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x52, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x70, 0x12, 0x34, 0x56, 0x78};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Uint);
    EXPECT_EQ(0x12345678, static_cast<std::uint32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a ULong value - section 1.6.6.
// Note that there are three serializations for UInt values:
// The first is as a fixed width value in the form of 0x80/0xXX/0xYY/0xZZ/0xAA with the values in
// network byte order.
// The second applies to values in the range 0..255: 0x53/0xXX
// The third applies to the specific value of 0: 0x44.
TEST_F(TestValueSerialization, SerializeUlong)
{
  {
    // Input first form with value == 0. Expected output: 3rd form.
    std::vector<uint8_t> testVector{0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ulong);
    EXPECT_EQ(0, static_cast<std::uint64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x44, val[0]);
  }
  {
    // Third form, value == 0.
    std::vector<uint8_t> testVector{0x44};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ulong);
    EXPECT_EQ(0, static_cast<std::uint64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(1, val.size());
    EXPECT_EQ(0x44, val[0]);
  }
  {
    // First form, value < 255, expected output: 2nd form.
    std::vector<uint8_t> testVector{0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ulong);
    EXPECT_EQ(0x85, static_cast<std::uint64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x53, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x53, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ulong);
    EXPECT_EQ(0x85, static_cast<std::uint64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x53, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x80, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Ulong);
    EXPECT_EQ(0x1234567812345678, static_cast<std::uint64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a Byte value (0x51/0xXX) - section 1.6.7.
TEST_F(TestValueSerialization, SerializeByte)
{
  {
    std::vector<uint8_t> testVector{0x51, 0x25};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Byte);
    EXPECT_EQ(0x25, static_cast<int8_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x51, val[0]);
    EXPECT_EQ(0x25, val[1]);
  }

  {
    std::vector<uint8_t> testVector{0x51, 0x89};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Byte);
    EXPECT_EQ(-119, static_cast<int8_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x51, val[0]);
    EXPECT_EQ(0x89, val[1]);
  }
}

//  Test deserializing a UShort value (0x61/0xXX/0xXX) - section 1.6.8.
// Note: Serialized value is in network byte order.
TEST_F(TestValueSerialization, SerializeShort)
{
  {
    std::vector<uint8_t> testVector{0x61, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Short);
    EXPECT_EQ(0, static_cast<std::int16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }

  {
    std::vector<uint8_t> testVector{0x61, 0x04, 0x00};

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Short);
    EXPECT_EQ(0x400, static_cast<std::int16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }

  {
    std::vector<uint8_t> testVector{0x61, 0x04, 0x80};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Short);
    EXPECT_EQ(0x480, static_cast<std::int16_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a Int value - section 1.6.9.
// Note that there are two serializations for Int values:
// The first is as a fixed width value in the form of 0x71/0xXX/0xYY/0xZZ/0xAA with the values in
// network byte order.
// The second applies to values in the range -128..127: 0x54/0xXX
TEST_F(TestValueSerialization, SerializeInt)
{
  {
    // Input first form with value == 0. Expected output: 2nd form.
    std::vector<uint8_t> testVector{0x71, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Int);
    EXPECT_EQ(0, static_cast<std::int32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x54, val[0]);
    EXPECT_EQ(0x00, val[1]);
  }
  {
    // First form, value < 255, expected output: 2nd form.
    std::vector<uint8_t> testVector{0x71, 0x00, 0x00, 0x00, 0x75};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Int);
    EXPECT_EQ(0x75, static_cast<std::int32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x54, val[0]);
    EXPECT_EQ(0x75, val[1]);
  }
  {
    // First form, value < 255, expected output: 2nd form. Note that the value of 0x85 is greater
    // than 127 so it cannot be represented in the second form.
    std::vector<uint8_t> testVector{0x71, 0x00, 0x00, 0x00, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Int);
    EXPECT_EQ(0x85, static_cast<std::int32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x54, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Int);
    EXPECT_EQ(-123, static_cast<std::int32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x54, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x71, 0x12, 0x34, 0x56, 0x78};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Int);
    EXPECT_EQ(0x12345678, static_cast<std::int32_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a Long value - section 1.6.10.
// Note that there are two serializations for Long values:
// The first is as a fixed width value in the form of 0x81/0xXX/0xYY/0xZZ/0xAA with the values in
// network byte order.
// The second applies to values in the range 0..255: 0x55/0xXX
TEST_F(TestValueSerialization, SerializeLong)
{
  {
    // First form, value < 255, expected output: 2nd form.
    std::vector<uint8_t> testVector{0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x75};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Long);
    EXPECT_EQ(0x75, static_cast<std::int64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x55, val[0]);
    EXPECT_EQ(0x75, val[1]);
  }
  {
    // First form, value < 255, expected output: First form because 0x85 cannot be expressed as a
    // signed byte.
    std::vector<uint8_t> testVector{0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Long);
    EXPECT_EQ(133, static_cast<std::int64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x55, 0x85};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Long);
    EXPECT_EQ(-123, static_cast<std::int64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(2, val.size());
    EXPECT_EQ(0x55, val[0]);
    EXPECT_EQ(0x85, val[1]);
  }
  {
    // Second form, value < 255.
    std::vector<uint8_t> testVector{0x81, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Long);
    EXPECT_EQ(0x1234567812345678, static_cast<std::int64_t>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a Float value - section 1.6.11.
TEST_F(TestValueSerialization, SerializeFloat)
{
  std::vector<uint8_t> testVector{0x72, 0x40, 0x49, 0x0f, 0xda};
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Float);
  EXPECT_FLOAT_EQ(3.1415926f, static_cast<float>(value));

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(val, testVector);
}

//  Test deserializing a Double value - section 1.6.12.
TEST_F(TestValueSerialization, SerializeDouble)
{
  std::vector<uint8_t> testVector{0x82, 0x40, 0x09, 0x21, 0xFB, 0x4D, 0x12, 0xD8, 0x4A};
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Double);
  EXPECT_DOUBLE_EQ(3.1415926, static_cast<double>(value));

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(val, testVector);
}

//  Test deserializing a Char value - section 1.6.16.
// Note uAMQP does not appear to have support for encoding and decoding Char values.
#if 0
TEST_F(TestValueSerialization, SerializeChar)
{
  std::vector<uint8_t> testVector{0x73, 0x4D, 0x12, 0xD8, 0x4A};
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Char);
  EXPECT_EQ(0x4d12d84a, static_cast<char32_t>(value));

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(val, testVector);
}
#endif

//  Test deserializing a Milliseconds value - section 1.6.17.
TEST_F(TestValueSerialization, SerializeMilliseconds)
{
  std::vector<uint8_t> testVector{0x83, 0x00, 0x00, 0x00, 0x00, 0x64, 0x41, 0xc0, 0x79};
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Timestamp);
  EXPECT_EQ(0x6441c079, static_cast<std::chrono::milliseconds>(value.AsTimestamp()).count());

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(val, testVector);
}

//  Test deserializing a Uuid value - section 1.6.18.
TEST_F(TestValueSerialization, SerializeUuid)
{
  Azure::Core::Uuid testUuid{Azure::Core::Uuid::CreateUuid()};
  std::vector<uint8_t> testVector{0x98};
  testVector.insert(testVector.end(), testUuid.AsArray().begin(), testUuid.AsArray().end());
  AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
  EXPECT_EQ(value.GetType(), AmqpValueType::Uuid);
  EXPECT_EQ(testUuid.ToString(), static_cast<Azure::Core::Uuid>(value).ToString());

  auto val = AmqpValue::Serialize(value);
  EXPECT_EQ(val, testVector);
}

//  Test deserializing a Binary value - section 1.6.19.
// Note that there are two serializations for Binary values:
// The first is as a variable width value in the form of 0xa0/<1 byte length>/<binary data>
// The second is as a variable width value in the form of 0xb0/<4 byte length>/<binary data>
TEST_F(TestValueSerialization, SerializeBinary)
{
  // First form, serialized as first form.
  {
    Azure::Core::Uuid testUuid{Azure::Core::Uuid::CreateUuid()};
    std::vector<uint8_t> testVector{0xa0, 0x10};
    testVector.insert(testVector.end(), testUuid.AsArray().begin(), testUuid.AsArray().end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Binary);
    EXPECT_EQ(value.AsBinary().size(), 16);
    auto binary(value.AsBinary());
    std::array<uint8_t, 16> valueAsArray{};

    std::copy_n(binary.begin(), 16, valueAsArray.begin());
    EXPECT_EQ(valueAsArray, testUuid.AsArray());

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
  // Second form, serialized as first form.
  {
    Azure::Core::Uuid testUuid{Azure::Core::Uuid::CreateUuid()};
    std::vector<uint8_t> testVector{0xb0, 0x00, 0x00, 0x00, 0x10};
    testVector.insert(testVector.end(), testUuid.AsArray().begin(), testUuid.AsArray().end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Binary);
    EXPECT_EQ(value.AsBinary().size(), 16);
    auto binary(value.AsBinary());
    std::array<uint8_t, 16> valueAsArray{};

    std::copy_n(binary.begin(), 16, valueAsArray.begin());
    EXPECT_EQ(valueAsArray, testUuid.AsArray());

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(18, val.size());
    EXPECT_EQ(0xa0, val[0]);
    EXPECT_EQ(0x10, val[1]);
  }

  {
    char values[255]{};
    std::vector<uint8_t> testVector{0xa0, sizeof(values)};
    testVector.insert(testVector.end(), values, values + sizeof(values));
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Binary);
    EXPECT_EQ(value.AsBinary().size(), 255);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(257, val.size());
    EXPECT_EQ(0xa0, val[0]);
    EXPECT_EQ(0xff, val[1]);
  }
  {
    char values[256]{};
    std::vector<uint8_t> testVector{0xb0, 0x00, 0x00, 0x01, 0x00};
    testVector.insert(testVector.end(), values, values + sizeof(values));
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Binary);
    EXPECT_EQ(value.AsBinary().size(), 256);

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(261, val.size());
    EXPECT_EQ(0xb0, val[0]);
    EXPECT_EQ(0x00, val[1]);
    EXPECT_EQ(0x00, val[2]);
    EXPECT_EQ(0x01, val[3]);
    EXPECT_EQ(0x00, val[4]);
  }
}

//  Test deserializing a String value - section 1.6.20.
// Note that there are two serializations for String values:
// The first is as a variable width value in the form of 0xa1/<1 byte length>/<binary data>
// The second is as a variable width value in the form of 0xb1/<4 byte length>/<binary data>
TEST_F(TestValueSerialization, SerializeString)
{
  // First form, serialized as first form.
  {
    std::string stringValue;
    for (int i = 0; i < 255; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 25]);
    }
    std::vector<uint8_t> testVector{0xa1, 0xff};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::String);
    EXPECT_EQ(static_cast<std::string>(value).size(), 255);
    EXPECT_EQ(stringValue, static_cast<std::string>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
  // Second form, serialized as first form.
  {
    std::string stringValue;
    for (int i = 0; i < 255; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 26]);
    }
    std::vector<uint8_t> testVector{0xb1, 0x00, 0x00, 0x00, 0xff};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::String);
    EXPECT_EQ(static_cast<std::string>(value).size(), 255);
    EXPECT_EQ(stringValue, static_cast<std::string>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 257);
    EXPECT_EQ(val[0], 0xa1);
    EXPECT_EQ(val[1], 0xff);
    EXPECT_EQ(val[2], 'A');
  }
  // Second form, serialized as second form.
  {
    std::string stringValue;
    for (int i = 0; i < 256; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 25]);
    }
    std::vector<uint8_t> testVector{0xb1, 0x00, 0x00, 0x01, 0x00};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::String);
    EXPECT_EQ(static_cast<std::string>(value).size(), 256);
    EXPECT_EQ(stringValue, static_cast<std::string>(value));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing a Symbol value - section 1.6.21.
// Note that there are two serializations for Symbol values:
// The first is as a variable width value in the form of 0xa3/<1 byte length>/<binary data>
// The second is as a variable width value in the form of 0xb3/<4 byte length>/<binary data>
TEST_F(TestValueSerialization, SerializeSymbol)
{
  // First form, serialized as first form.
  {
    std::string stringValue;
    for (int i = 0; i < 255; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 25]);
    }
    std::vector<uint8_t> testVector{0xa3, 0xff};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Symbol);
    EXPECT_EQ(value.AsSymbol().size(), 255);
    EXPECT_EQ(AmqpSymbol(stringValue), value.AsSymbol());

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
  // Second form, serialized as first form.
  {
    std::string stringValue;
    for (int i = 0; i < 255; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 26]);
    }
    std::vector<uint8_t> testVector{0xb3, 0x00, 0x00, 0x00, 0xff};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Symbol);
    EXPECT_EQ(value.AsSymbol().size(), 255);
    EXPECT_EQ(AmqpSymbol(stringValue), value.AsSymbol());

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 257);
    EXPECT_EQ(val[0], 0xa3);
    EXPECT_EQ(val[1], 0xff);
    EXPECT_EQ(val[2], 'A');
  }
  // Second form, serialized as second form.
  {
    std::string stringValue;
    for (int i = 0; i < 256; i += 1)
    {
      stringValue.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % 26]);
    }
    std::vector<uint8_t> testVector{0xb3, 0x00, 0x00, 0x01, 0x00};
    testVector.insert(testVector.end(), stringValue.begin(), stringValue.end());
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Symbol);
    EXPECT_EQ(value.AsSymbol().size(), 256);
    EXPECT_EQ(AmqpSymbol(stringValue), value.AsSymbol());

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

template <typename T> T GenerateRandomValue()
{
  // Generate a random value of type T using std::random_device.
  // This is not a cryptographically secure random number generator, but it is good enough for our
  // purposes. The random number generator is seeded with a random value from std::random_device.
  // The random number generator is then used to generate a random value of type T.
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> dis(
      (std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)());
  return dis(gen);
}

// Append a value of type T to a buffer.
// The value is appended in network byte order.
// The value is appended as a fixed width value.
// T
void AppendValue(std::vector<uint8_t>& buffer, uint16_t value)
{
  buffer.push_back((value >> 8) & 0xff);
  buffer.push_back(value & 0xff);
}
void AppendValue(std::vector<uint8_t>& buffer, uint32_t value)
{
  buffer.push_back((value >> 24) & 0xff);
  buffer.push_back((value >> 16) & 0xff);
  buffer.push_back((value >> 8) & 0xff);
  buffer.push_back(value & 0xff);
}

//  Test deserializing a List value - section 1.6.22.
// Note that there are three serializations for List values:
// The first is a fixed value in the form of 0x45 which represents an empty list.
// The second is a compound value in the form of 0xc0/<1 byte size>/<1 byte count>/<list of values>
// for list elements with a total size less than 255 octets. The third is a compound value in the
// form of 0xd0/<4 byte size in network byte order>/<4 byte count in network byte order>/<list of
// values> for list elements with a total size less than 2^32 octets.
TEST_F(TestValueSerialization, SerializeList)
{
  // First form, serialized as first form.
  {
    std::vector<unsigned char> testVector{0x45};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), 0);
  }

  // First form, serialized as first form.
  {
    AmqpList emptyList;
    AmqpValue value{emptyList.AsAmqpValue()};
    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 1);
    EXPECT_EQ(0x45, val[0]);
  }

  // Second form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xc0, 0x01, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::List);
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 1);
    EXPECT_EQ(0x45, val[0]);
  }
  // Third form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xd0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::List);
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 1);
    EXPECT_EQ(0x45, val[0]);
  }

  // Second form serialized as second form.
  {
    constexpr size_t valueCount = 0x15;
    // Create an array of random values.
    std::vector<AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.push_back(AmqpValue(static_cast<uint32_t>(GenerateRandomValue<long>())));
    }
    size_t totalSize = 1; // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize += AmqpValue::GetSerializedSize(val);
    }
    EXPECT_LE(totalSize, 255);
    std::vector<uint8_t> testVector{0xc0};
    testVector.push_back(static_cast<uint8_t>(totalSize));
    testVector.push_back(static_cast<uint8_t>(valueCount));

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      auto serializedVal{AmqpValue::Serialize(val)};
      testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::List);
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), values.size());
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin(), list.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }

  // Third form, serialized as second form.
  {
    constexpr size_t valueCount = 0x10;
    // Create an array of random values.
    std::vector<AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.push_back(AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize = 4; // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize += AmqpValue::GetSerializedSize(val);
    }
    EXPECT_LE(totalSize, 255);
    std::vector<uint8_t> testVector{0xd0};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount));

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      auto serializedVal{AmqpValue::Serialize(val)};
      testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::List);
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), values.size());
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin(), list.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 147);
    EXPECT_EQ(val[0], 0xc0);
    EXPECT_EQ(
        val[1],
        totalSize - 3 /* Account for the difference in size between the input and output size.*/);
    EXPECT_EQ(val[2], valueCount);
  }

  // Third form, serialized as third form.
  {
    constexpr size_t valueCount = 0x210;
    // Create an array of random values.
    std::vector<AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.push_back(AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize = 4; // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize += AmqpValue::GetSerializedSize(val);
    }
    EXPECT_GE(totalSize, 256);
    std::vector<uint8_t> testVector{0xd0};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount));

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      auto serializedVal{AmqpValue::Serialize(val)};
      testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::List);
    AmqpList list(value.AsList());
    EXPECT_EQ(list.size(), values.size());
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin(), list.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing an Array value - section 1.6.24.
// Note that there are two serializations for Arrayvalues:
// The first is an array value in the form of 0xe0/<1 byte count>/<list of values>
// for arrays with a total count of less than 255 octets. The second is an array value in the
// form of 0xf0/<4 byte count in network byte order>/<list of
// values> for list elements with a total size less than 2^32 octets.
TEST_F(TestValueSerialization, SerializeArray)
{
  // First form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xe0, 0x01, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Array);
    AmqpArray array(value.AsArray());
    EXPECT_EQ(array.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(testVector, val);
  }
  // Second form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xf0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Array);
    AmqpArray array(value.AsArray());
    EXPECT_EQ(array.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 3);
    EXPECT_EQ(0xe0, val[0]);
    EXPECT_EQ(0x01, val[1]);
    EXPECT_EQ(0x00, val[2]);
  }

  // Second form, serialized as first form.
  {
    constexpr size_t valueCount = 0x10;
    // Create an array of random values.
    std::vector<AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.push_back(AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize
        = (static_cast<size_t>(4) + 1); // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize
          += (AmqpValue::GetSerializedSize(val)
              - 1); // We're not going to serialize the constructor on the serialized values.
    }
    EXPECT_LE(totalSize, 255);
    std::vector<uint8_t> testVector{0xf0};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount));
    testVector.push_back(0x81); // Insert the constructor for the elements.

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      auto serializedVal{AmqpValue::Serialize(val)};
      // We want to skip over the constructor value in the serialized constructor.
      testVector.insert(testVector.end(), serializedVal.begin() + 1, serializedVal.end());
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Array);
    AmqpArray array(value.AsArray());
    EXPECT_EQ(array.size(), values.size());
    EXPECT_TRUE(std::equal(values.begin(), values.end(), array.begin(), array.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 132);
    EXPECT_EQ(val[0], 0xe0);
    EXPECT_EQ(
        val[1],
        totalSize - 3 /* Account for the difference in size between the input and output size.*/);
    EXPECT_EQ(val[2], valueCount);
  }

  // Second form, serialized as second form.
  {
    constexpr size_t valueCount = 0x210;
    // Create an array of random values.
    std::vector<AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.push_back(AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize
        = (static_cast<size_t>(4) + 1); // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize += (AmqpValue::GetSerializedSize(val) - 1);
    }
    EXPECT_GE(totalSize, 256);
    std::vector<uint8_t> testVector{0xf0};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount));
    testVector.push_back(0x81); // element constructor - 0x80 == long.

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      auto serializedVal{AmqpValue::Serialize(val)};
      testVector.insert(testVector.end(), serializedVal.begin() + 1, serializedVal.end());
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Array);
    AmqpArray array(value.AsArray());
    EXPECT_EQ(array.size(), values.size());
    size_t i = 0;
    for (auto const& val : array)
    {
      if (val != values[i])
      {
        GTEST_LOG_(ERROR) << "Mismatch in decoded array at offset " << i
                          << "Original value: " << values[i] << " Transformed value: " << val;
      }
      i += 1;
    }
    EXPECT_TRUE(std::equal(values.begin(), values.end(), array.begin(), array.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}

//  Test deserializing an Map value - section 1.6.23.
// Note that there are two serializations for Map values:
// The first is an array value in the form of 0xc1/<1 byte count>/<list of values>
// for maps with a total count of less than 255 values. The second is a value in the
// form of 0xd1/<4 byte count in network byte order>/<list of
// values> for map elements with a total size less than 2^32 octets.
TEST_F(TestValueSerialization, SerializeMap)
{
  // First form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xc1, 0x01, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Map);
    AmqpMap map(value.AsMap());
    EXPECT_EQ(map.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(testVector, val);
  }

  // Second form, serialized as first form.
  {
    std::vector<uint8_t> testVector{0xd1, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Map);
    AmqpMap map(value.AsMap());
    EXPECT_EQ(map.size(), 0);

    std::vector<uint8_t> val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 3);
    EXPECT_EQ(0xc1, val[0]);
    EXPECT_EQ(0x01, val[1]);
    EXPECT_EQ(0x00, val[2]);
  }

  // Second form, serialized as first form.
  {
    constexpr size_t valueCount = 0x10;
    // Create an map of random values.
    std::map<AmqpValue, AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.emplace(
          AmqpValue(std::to_string(i)),
          AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize = 4; // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize
          += (AmqpValue::GetSerializedSize(val.first)
              + AmqpValue::GetSerializedSize(val.second)); // We're not going to serialize the
                                                           // constructor on the serialized values.
    }
    EXPECT_LE(totalSize, 255);
    std::vector<uint8_t> testVector{0xd1};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount * 2));

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      {
        auto serializedVal{AmqpValue::Serialize(val.first)};
        testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
      }
      {
        auto serializedVal{AmqpValue::Serialize(val.second)};
        testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
      }
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Map);
    AmqpMap map(value.AsMap());
    EXPECT_EQ(map.size(), values.size());
    EXPECT_TRUE(std::equal(values.begin(), values.end(), map.begin(), map.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val.size(), 201);
    EXPECT_EQ(val[0], 0xc1);
    EXPECT_EQ(val[1], totalSize - 3);
    EXPECT_EQ(val[2], valueCount * 2);
  }

  // Second form, serialized as second form.
  {
    constexpr size_t valueCount = 0x210;
    // Create an map of random values.
    std::map<AmqpValue, AmqpValue> values;
    for (size_t i = 0; i < valueCount; i += 1)
    {
      values.emplace(
          AmqpValue(std::to_string(i)),
          AmqpValue(static_cast<int64_t>(GenerateRandomValue<long long>())));
    }
    size_t totalSize = 4; // Include the size of the list count in the size
    for (auto const& val : values)
    {
      totalSize
          += (AmqpValue::GetSerializedSize(val.first) + AmqpValue::GetSerializedSize(val.second));
    }
    EXPECT_GE(totalSize, 256);
    std::vector<uint8_t> testVector{0xd1};
    AppendValue(testVector, static_cast<uint32_t>(totalSize));
    AppendValue(testVector, static_cast<uint32_t>(valueCount * 2));

    // Now append all the values to the list.
    for (auto const& val : values)
    {
      {
        auto serializedVal{AmqpValue::Serialize(val.first)};
        testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
      }
      {
        auto serializedVal{AmqpValue::Serialize(val.second)};
        testVector.insert(testVector.end(), serializedVal.begin(), serializedVal.end());
      }
    }

    AmqpValue value{AmqpValue::Deserialize(testVector.data(), testVector.size())};
    EXPECT_EQ(value.GetType(), AmqpValueType::Map);
    AmqpMap map(value.AsMap());
    EXPECT_EQ(map.size(), values.size());
    int i = 0;
    auto valIterator = values.begin();
    for (auto const& val : map)
    {
      EXPECT_NE(valIterator, values.end());
      if (val.first != valIterator->first)
      {
        GTEST_LOG_(ERROR) << "Key Mismatch in decoded map at offset " << i
                          << ". Original key: " << i << " Transformed key: " << val.first;
      }
      if (val.second != valIterator->second)
      {
        GTEST_LOG_(ERROR) << "Value Mismatch in decoded map at offset " << i
                          << ". Original value: " << values[i]
                          << " Transformed value: " << val.first;
      }
      i += 1;
      valIterator++;
    }
    EXPECT_TRUE(std::equal(values.begin(), values.end(), map.begin(), map.end()));

    auto val = AmqpValue::Serialize(value);
    EXPECT_EQ(val, testVector);
  }
}
