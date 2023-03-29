// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>

class TestSourceTarget : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Core::Amqp::Models::_internal;

TEST_F(TestSourceTarget, SimpleSourceTarget)
{
  {
    MessageSource source;
    MessageTarget target;
  }
  {
    MessageSource source("test");
    MessageTarget target("test");
  }
  {
    MessageSource source(std::string("test"));
    MessageTarget target(std::string("test"));
  }
  {
    MessageSource source(Messaging::CreateSource("test1"));
    MessageTarget target(Messaging::CreateTarget("test1"));
  }

  {
    EXPECT_ANY_THROW(MessageSource source(Value{}));
    EXPECT_ANY_THROW(MessageSource source(Value::CreateArray()));
  }
  {
    EXPECT_ANY_THROW(MessageTarget target(Value{}));
    EXPECT_ANY_THROW(MessageTarget target(Value::CreateArray()));
  }
}

namespace {
std::string timeToString(std::chrono::system_clock::time_point t)
{
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  std::string time_str = std::ctime(&time);
  time_str.resize(time_str.size() - 1);
  return time_str;
}
} // namespace

TEST_F(TestSourceTarget, TargetProperties)
{
  {
    MessageTarget target;
    EXPECT_ANY_THROW(target.GetAddress());
    EXPECT_EQ(TerminusDurability::None, target.GetTerminusDurability());
    EXPECT_ANY_THROW(target.GetCapabilities());
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, target.GetExpiryPolicy());
    EXPECT_EQ(false, target.GetDynamic());
    EXPECT_ANY_THROW(target.GetDynamicNodeProperties());
  }

  {
    MessageTarget target("test");
    EXPECT_EQ(Value{"test"}, target.GetAddress());
  }
  {
    MessageTarget target;
    target.SetAddress("Address");
    EXPECT_EQ(Value("Address"), target.GetAddress());

    target.SetCapabilities(Value::CreateSymbol("Test"));
    EXPECT_EQ(target.GetCapabilities().GetType(), AmqpValueType::Array);
    EXPECT_EQ(AmqpValueType::Symbol, target.GetCapabilities().GetArrayItem(0).GetType());
    EXPECT_EQ("Test", target.GetCapabilities().GetArrayItem(0).GetSymbol());

    target.SetTerminusDurability(TerminusDurability::None);
    EXPECT_EQ(TerminusDurability::None, target.GetTerminusDurability());
    target.SetTerminusDurability(TerminusDurability::Configuration);
    EXPECT_EQ(TerminusDurability::Configuration, target.GetTerminusDurability());
    target.SetTerminusDurability(TerminusDurability::UnsettledState);
    EXPECT_EQ(TerminusDurability::UnsettledState, target.GetTerminusDurability());
    EXPECT_ANY_THROW(target.SetTerminusDurability(static_cast<TerminusDurability>(655345)));

    target.SetExpiryPolicy(TerminusExpiryPolicy::LinkDetach);
    EXPECT_EQ(TerminusExpiryPolicy::LinkDetach, target.GetExpiryPolicy());
    target.SetExpiryPolicy(TerminusExpiryPolicy::ConnectionClose);
    EXPECT_EQ(TerminusExpiryPolicy::ConnectionClose, target.GetExpiryPolicy());
    target.SetExpiryPolicy(TerminusExpiryPolicy::Never);
    EXPECT_EQ(TerminusExpiryPolicy::Never, target.GetExpiryPolicy());
    target.SetExpiryPolicy(TerminusExpiryPolicy::SessionEnd);
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, target.GetExpiryPolicy());
    EXPECT_ANY_THROW(target.SetExpiryPolicy(static_cast<TerminusExpiryPolicy>(655345)));

    std::chrono::system_clock::time_point expirationTime{
        std::chrono::system_clock::now() + std::chrono::seconds(60)};
    GTEST_LOG_(INFO) << "Expiration time set: " << timeToString(expirationTime);
    target.SetTimeout(expirationTime);
    GTEST_LOG_(INFO) << "Expiration time get: " << timeToString(target.GetTimeout());
    EXPECT_EQ(
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch()),
        std::chrono::duration_cast<std::chrono::seconds>(target.GetTimeout().time_since_epoch()));

    target.SetDynamic(true);
    EXPECT_EQ(true, target.GetDynamic());
    target.SetDynamic(false);
    EXPECT_EQ(false, target.GetDynamic());

    target.SetDynamicNodeProperties("Dynamic properties");
    EXPECT_ANY_THROW(target.GetDynamicNodeProperties());
    auto dynamicMap = Value::CreateMap();
    dynamicMap.SetMapValue("Key", 23);
    target.SetDynamicNodeProperties(dynamicMap);
    EXPECT_EQ(target.GetDynamicNodeProperties().GetType(), AmqpValueType::Map);
    EXPECT_EQ(target.GetDynamicNodeProperties().GetMapValue("Key"), Value(23));

    GTEST_LOG_(INFO) << "Target: " << target;
  }
}

TEST_F(TestSourceTarget, TargetThroughValue)
{
  MessageTarget target("address1");
  Value value(static_cast<const Value>(target));

  MessageTarget target2(value);
  EXPECT_EQ(target.GetAddress(), target2.GetAddress());
}

TEST_F(TestSourceTarget, SourceProperties)
{
  {
    MessageSource source;
    EXPECT_ANY_THROW(source.GetAddress());
    EXPECT_EQ(TerminusDurability::None, source.GetTerminusDurability());
    EXPECT_ANY_THROW(source.GetCapabilities());
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, source.GetExpiryPolicy());
    EXPECT_EQ(false, source.GetDynamic());
    EXPECT_ANY_THROW(source.GetDynamicNodeProperties());
  }

  {
    MessageSource source("test");
    EXPECT_EQ(Value{"test"}, source.GetAddress());
  }

  {
    MessageSource source;
    source.SetAddress("Address");
    EXPECT_EQ(Value("Address"), source.GetAddress());

    source.SetCapabilities(Value::CreateSymbol("Test"));
    EXPECT_EQ(source.GetCapabilities().GetType(), AmqpValueType::Array);
    EXPECT_EQ(AmqpValueType::Symbol, source.GetCapabilities().GetArrayItem(0).GetType());
    EXPECT_EQ("Test", source.GetCapabilities().GetArrayItem(0).GetSymbol());

    source.SetTerminusDurability(TerminusDurability::None);
    EXPECT_EQ(TerminusDurability::None, source.GetTerminusDurability());
    source.SetTerminusDurability(TerminusDurability::Configuration);
    EXPECT_EQ(TerminusDurability::Configuration, source.GetTerminusDurability());
    source.SetTerminusDurability(TerminusDurability::UnsettledState);
    EXPECT_EQ(TerminusDurability::UnsettledState, source.GetTerminusDurability());
    EXPECT_ANY_THROW(source.SetTerminusDurability(static_cast<TerminusDurability>(655345)));

    source.SetExpiryPolicy(TerminusExpiryPolicy::LinkDetach);
    EXPECT_EQ(TerminusExpiryPolicy::LinkDetach, source.GetExpiryPolicy());
    source.SetExpiryPolicy(TerminusExpiryPolicy::ConnectionClose);
    EXPECT_EQ(TerminusExpiryPolicy::ConnectionClose, source.GetExpiryPolicy());
    source.SetExpiryPolicy(TerminusExpiryPolicy::Never);
    EXPECT_EQ(TerminusExpiryPolicy::Never, source.GetExpiryPolicy());
    source.SetExpiryPolicy(TerminusExpiryPolicy::SessionEnd);
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, source.GetExpiryPolicy());
    EXPECT_ANY_THROW(source.SetExpiryPolicy(static_cast<TerminusExpiryPolicy>(655345)));

    std::chrono::system_clock::time_point expirationTime{
        std::chrono::system_clock::now() + std::chrono::seconds(60)};
    GTEST_LOG_(INFO) << "Expiration time set: " << timeToString(expirationTime);
    source.SetTimeout(expirationTime);
    GTEST_LOG_(INFO) << "Expiration time get: " << timeToString(source.GetTimeout());
    EXPECT_EQ(
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch()),
        std::chrono::duration_cast<std::chrono::seconds>(source.GetTimeout().time_since_epoch()));

    source.SetDynamic(true);
    EXPECT_EQ(true, source.GetDynamic());
    source.SetDynamic(false);
    EXPECT_EQ(false, source.GetDynamic());

    source.SetDynamicNodeProperties("Dynamic properties");
    EXPECT_ANY_THROW(source.GetDynamicNodeProperties());
    auto dynamicMap = Value::CreateMap();
    dynamicMap.SetMapValue("Key", 23);
    source.SetDynamicNodeProperties(dynamicMap);
    EXPECT_EQ(source.GetDynamicNodeProperties().GetType(), AmqpValueType::Map);
    EXPECT_EQ(source.GetDynamicNodeProperties().GetMapValue("Key"), Value(23));

    source.SetDistributionMode("A different mode");
    EXPECT_EQ("A different mode", source.GetDistributionMode());

    // A filter set is a map.
    source.SetFilter("Dynamic properties");
    EXPECT_ANY_THROW(source.GetFilter());
    auto filterMap = Value::CreateMap();
    filterMap.SetMapValue("Key", 23);
    source.SetFilter(filterMap);
    EXPECT_EQ(source.GetFilter().GetType(), AmqpValueType::Map);
    EXPECT_EQ(source.GetFilter().GetMapValue("Key"), Value(23));

    source.SetDefaultOutcome("Default outcome");
    EXPECT_EQ(source.GetDefaultOutcome(), "Default outcome");

    source.SetOutcomes(Value::CreateSymbol("Test"));
    EXPECT_EQ(source.GetOutcomes().GetType(), AmqpValueType::Array);
    EXPECT_EQ(AmqpValueType::Symbol, source.GetOutcomes().GetArrayItem(0).GetType());
    EXPECT_EQ("Test", source.GetOutcomes().GetArrayItem(0).GetSymbol());

    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSource source("address1");
    Value value(static_cast<const Value>(source));

    MessageSource source2(value);
    EXPECT_EQ(source.GetAddress(), source2.GetAddress());
  }
}
