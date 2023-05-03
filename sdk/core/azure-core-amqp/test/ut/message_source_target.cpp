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
    EXPECT_ANY_THROW(MessageSource source(AmqpValue{}));
    AmqpValue val = AmqpArray();
    EXPECT_ANY_THROW(MessageSource source{val});
  }
  {
    EXPECT_ANY_THROW(MessageTarget target(AmqpValue{}));
    AmqpValue val = AmqpArray();
    EXPECT_ANY_THROW(MessageTarget target(val));
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
    EXPECT_EQ(AmqpValue{"test"}, target.GetAddress());
  }
  {
    MessageTarget target;
    target.SetAddress("Address");
    EXPECT_EQ(AmqpValue("Address"), target.GetAddress());

    target.SetCapabilities(AmqpSymbol{"Test"});
    EXPECT_EQ(1, target.GetCapabilities().size());
    EXPECT_EQ(AmqpValueType::Symbol, target.GetCapabilities().at(0).GetType());
    EXPECT_EQ(target.GetCapabilities()[0].AsSymbol(), "Test");

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
    AmqpMap dynamicMap;
    dynamicMap["Key"] = 23;
    target.SetDynamicNodeProperties(dynamicMap);

    AmqpMap map2{target.GetDynamicNodeProperties().AsMap()};
    EXPECT_EQ(target.GetDynamicNodeProperties().GetType(), AmqpValueType::Map);
    EXPECT_EQ(dynamicMap["Key"], AmqpValue(23));

    GTEST_LOG_(INFO) << "Target: " << target;
  }
}

TEST_F(TestSourceTarget, TargetThroughValue)
{
  MessageTarget target("address1");
  const AmqpValue v = target;
  AmqpValue value(v);

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
    EXPECT_EQ(AmqpValue{"test"}, source.GetAddress());
  }

  {
    MessageSourceOptions options;
    options.Address = "Address";
    MessageSource source(options);
    EXPECT_EQ(AmqpValue("Address"), source.GetAddress());
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSourceOptions options;
    options.Capabilities.push_back(AmqpSymbol{"Test"});
    MessageSource source(options);
    EXPECT_EQ(1, source.GetCapabilities().size());
    EXPECT_EQ(AmqpValueType::Symbol, source.GetCapabilities()[0].GetType());
    EXPECT_EQ(source.GetCapabilities()[0].AsSymbol(), "Test");
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSourceOptions options;
    options.TerminusDurability = TerminusDurability::None;

    MessageSource source(options);
    EXPECT_EQ(TerminusDurability::None, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusDurability = TerminusDurability::Configuration;
    MessageSource source(options);

    EXPECT_EQ(TerminusDurability::Configuration, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusDurability = TerminusDurability::UnsettledState;
    MessageSource source(options);
    EXPECT_EQ(TerminusDurability::UnsettledState, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusDurability = static_cast<TerminusDurability>(655345);
    EXPECT_ANY_THROW(MessageSource source(options));
  }

  {
    MessageSourceOptions options;
    options.TerminusExpiryPolicy = TerminusExpiryPolicy::LinkDetach;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::LinkDetach, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusExpiryPolicy = TerminusExpiryPolicy::ConnectionClose;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::ConnectionClose, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusExpiryPolicy = TerminusExpiryPolicy::Never;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::Never, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusExpiryPolicy = TerminusExpiryPolicy::SessionEnd;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.TerminusDurability = static_cast<TerminusDurability>(655345);

    EXPECT_ANY_THROW(MessageSource source(options));
  }
  {
    std::chrono::system_clock::time_point expirationTime{
        std::chrono::system_clock::now() + std::chrono::seconds(60)};
    GTEST_LOG_(INFO) << "Expiration time set: " << timeToString(expirationTime);
    MessageSourceOptions options;
    options.Timeout = expirationTime;
    MessageSource source(options);

    GTEST_LOG_(INFO) << "Expiration time get: " << timeToString(source.GetTimeout());
    EXPECT_EQ(
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch()),
        std::chrono::duration_cast<std::chrono::seconds>(source.GetTimeout().time_since_epoch()));
  }
  {
    MessageSourceOptions options;
    options.Dynamic = true;
    MessageSource source(options);

    EXPECT_EQ(true, source.GetDynamic());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.Dynamic = false;
    MessageSource source(options);
    EXPECT_EQ(false, source.GetDynamic());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    AmqpMap dynamicMap;
    dynamicMap["Key"] = 23;

    MessageSourceOptions options;
    options.DynamicNodeProperties["Key"] = 23;
    MessageSource source(options);

    auto map2(source.GetDynamicNodeProperties());
    EXPECT_EQ(map2["Key"], AmqpValue(23));
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.DistributionMode = "A different mode";
    MessageSource source(options);

    EXPECT_EQ("A different mode", source.GetDistributionMode());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.Filter["Key"] = 23;
    MessageSource source(options);
    EXPECT_EQ(source.GetFilter()["Key"], AmqpValue(23));
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.DefaultOutcome = "Default outcome";
    MessageSource source(options);
    EXPECT_EQ(source.GetDefaultOutcome(), "Default outcome");
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSourceOptions options;
    options.Outcomes.push_back(AmqpSymbol("Test"));
    MessageSource source(options);
    EXPECT_EQ(1, source.GetOutcomes().size());
    EXPECT_EQ(AmqpValueType::Symbol, source.GetOutcomes().at(0).GetType());
    EXPECT_EQ(source.GetOutcomes().at(0).AsSymbol(), "Test");
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSource source("address1");
    const AmqpValue v = source;
    AmqpValue value(v);

    MessageSource source2(value);
    EXPECT_EQ(source.GetAddress(), source2.GetAddress());
  }
}
