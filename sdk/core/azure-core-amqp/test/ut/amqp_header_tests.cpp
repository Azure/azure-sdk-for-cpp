// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words reserialized

// #include "../src/models/private/header_impl.hpp"
#include "../src/models/private/header_impl.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;

class TestHeaders : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestHeaders, SimpleCreate)
{
  {
    MessageHeader header;
  }

  {
    MessageHeader header;
    EXPECT_EQ(0, header.DeliveryCount);
    EXPECT_EQ(4, header.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, header.Durable);
    EXPECT_EQ(false, header.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }

  {
    MessageHeader header;
    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }

  {
    MessageHeader header;

    header.DeliveryCount = 123;

    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }

  {
    MessageHeader header;

    header.Durable = false;

    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }

  {
    MessageHeader header;

    header.Priority = 3;

    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }
  {
    MessageHeader header;

    header.IsFirstAcquirer = true;

    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }
  {
    MessageHeader header;

    header.TimeToLive = std::chrono::milliseconds(37);

    auto nativeHeader = _detail::MessageHeaderFactory::ToImplementation(header);
    auto round_trip_header = _detail::MessageHeaderFactory::FromImplementation(nativeHeader);
    EXPECT_EQ(header, round_trip_header);
  }
}

class HeaderSerialization : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HeaderSerialization, SerializeHeaderDurable)
{
  {
    std::vector<uint8_t> buffer;
    MessageHeader header;
    header.Durable = true;
    buffer = MessageHeader::Serialize(header);

    MessageHeader deserialized = MessageHeader::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(header, deserialized);
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority);
    EXPECT_EQ(true, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x70, // Descriptor is for a message header
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
        0xc0, // List
        0x02, // 2 bytes long.
        0x01, // 1 elements.
        0x41, // Boolean True.
    };

    MessageHeader deserialized = MessageHeader::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(true, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(deserialized.TimeToLive.HasValue());

    auto reserialized = MessageHeader::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}
TEST_F(HeaderSerialization, SerializeHeaderPriority)
{
  {
    std::vector<uint8_t> buffer;
    MessageHeader header;
    header.Priority = 8;
    buffer = MessageHeader::Serialize(header);

    MessageHeader deserialized = MessageHeader::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(header, deserialized);
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(8, deserialized.Priority);
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x70, // Descriptor is for a message header
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
        0xc0, // List
        0x04, // 4 bytes long.
        0x02, // 2 elements.
        0x40, // First element Nil.
        0x50, // Second element ubyte.
        0x08 // byte value (8).
    };

    MessageHeader deserialized = MessageHeader::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(8, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(deserialized.TimeToLive.HasValue());
    EXPECT_EQ(MessageHeader::GetSerializedSize(deserialized), testValue.size());

    auto reserialized = MessageHeader::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(HeaderSerialization, SerializeHeaderTtl)
{
  {
    std::vector<uint8_t> buffer;
    MessageHeader header;
    header.TimeToLive = std::chrono::milliseconds(12345);
    buffer = MessageHeader::Serialize(header);

    MessageHeader deserialized = MessageHeader::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(header, deserialized);
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_EQ(deserialized.TimeToLive.Value(), std::chrono::milliseconds(12345));
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x70, // Descriptor is for a message header
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
        0xc0, // List
        0x08, // 8 bytes long.
        0x03, // 3 elements.
        0x40, // First element Nil.
        0x40, // Second element Nil.
        0x70, // 4 byte uint.
        0x00, // Uint data byte 1
        0x00, // Uint data byte 2
        0x30, // Uint data byte 3
        0x39 // Big endian encoded 12345.
    };

    MessageHeader deserialized = MessageHeader::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_EQ(deserialized.TimeToLive.Value(), std::chrono::milliseconds(12345));
    EXPECT_EQ(MessageHeader::GetSerializedSize(deserialized), testValue.size());

    auto reserialized = MessageHeader::Serialize(deserialized);
    EXPECT_EQ(MessageHeader::GetSerializedSize(deserialized), testValue.size());
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(HeaderSerialization, SerializeHeaderFirstAcquirer)
{
  {
    std::vector<uint8_t> buffer;
    MessageHeader header;
    header.IsFirstAcquirer = true;
    buffer = MessageHeader::Serialize(header);

    MessageHeader deserialized = MessageHeader::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(header, deserialized);
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(true, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x70, // Descriptor is for a message header
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
        0xc0, // List
        0x05, // 5 bytes long.
        0x04, // 4 elements.
        0x40, // First element Nil.
        0x40, // Second element Nil.
        0x40, // Third element Nil.
        0x41 // Fourth element boolean true.
    };

    MessageHeader deserialized = MessageHeader::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(0, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(true, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(deserialized.TimeToLive.HasValue());
    EXPECT_EQ(MessageHeader::GetSerializedSize(deserialized), testValue.size());

    auto reserialized = MessageHeader::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(HeaderSerialization, SerializeHeaderDeliveryCount)
{
  {
    std::vector<uint8_t> buffer;
    MessageHeader header;
    header.DeliveryCount = 157;
    buffer = MessageHeader::Serialize(header);

    MessageHeader deserialized = MessageHeader::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(header, deserialized);
    EXPECT_EQ(157, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x70, // Descriptor is for a message header
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
        0xc0, // List
        0x07, // 7 bytes long.
        0x05, // 5 elements.
        0x40, // First element Nil.
        0x40, // Second element Nil.
        0x40, // 3rd Element nil.
        0x40, // 4th element nil.
        0x52, // 5th element small integer
        0x9d // Small integer value.
    };

    MessageHeader deserialized = MessageHeader::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(157, deserialized.DeliveryCount);
    EXPECT_EQ(4, deserialized.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, deserialized.Durable);
    EXPECT_EQ(false, deserialized.IsFirstAcquirer);
    EXPECT_FALSE(deserialized.TimeToLive.HasValue());
    EXPECT_EQ(MessageHeader::GetSerializedSize(deserialized), testValue.size());

    auto reserialized = MessageHeader::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}
