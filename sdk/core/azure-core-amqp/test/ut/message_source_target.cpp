// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/models/message_source.hpp>
#include <azure/core/amqp/internal/models/message_target.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>

#include <gtest/gtest.h>

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
    EXPECT_ANY_THROW(MessageSource source(AmqpValue{}));
    AmqpValue val = AmqpArray().AsAmqpValue();
    EXPECT_ANY_THROW(MessageSource source{val});
  }
  {
    EXPECT_ANY_THROW(MessageTarget target(AmqpValue{}));
    AmqpValue val = AmqpArray().AsAmqpValue();
    EXPECT_ANY_THROW(MessageTarget target(val));
  }
}

namespace {
std::string timeToString(std::chrono::system_clock::time_point t)
{
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  char buf[26]{};
  std::strftime(buf, std::extent<decltype(buf)>::value, "%c", std::localtime(&time));
  return buf;
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
    MessageTargetOptions options;
    options.Address = "Address";
    MessageTarget target(options);
    EXPECT_EQ(AmqpValue("Address"), target.GetAddress());
    GTEST_LOG_(INFO) << "Target: " << target;
  }

  {
    MessageTargetOptions options;
    options.Capabilities.push_back(AmqpSymbol{"Test"}.AsAmqpValue());
    MessageTarget target(options);
    EXPECT_EQ(1, target.GetCapabilities().size());
    EXPECT_EQ(AmqpValueType::Symbol, target.GetCapabilities()[0].GetType());
    EXPECT_EQ(target.GetCapabilities()[0].AsSymbol(), "Test");
    GTEST_LOG_(INFO) << "Target: " << target;
  }

  {
    MessageTargetOptions options;
    options.TerminusDurabilityValue = TerminusDurability::None;

    MessageTarget target(options);
    EXPECT_EQ(TerminusDurability::None, target.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusDurabilityValue = TerminusDurability::Configuration;
    MessageTarget target(options);

    EXPECT_EQ(TerminusDurability::Configuration, target.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusDurabilityValue = TerminusDurability::UnsettledState;
    MessageTarget target(options);
    EXPECT_EQ(TerminusDurability::UnsettledState, target.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusDurabilityValue = static_cast<TerminusDurability>(655345);
    EXPECT_ANY_THROW(MessageTarget target(options));
  }

  {
    MessageTargetOptions options;
    options.TerminusExpiryPolicyValue = TerminusExpiryPolicy::LinkDetach;
    MessageTarget target(options);
    EXPECT_EQ(TerminusExpiryPolicy::LinkDetach, target.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusExpiryPolicyValue = TerminusExpiryPolicy::ConnectionClose;
    MessageTarget target(options);
    EXPECT_EQ(TerminusExpiryPolicy::ConnectionClose, target.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusExpiryPolicyValue = TerminusExpiryPolicy::Never;
    MessageTarget target(options);
    EXPECT_EQ(TerminusExpiryPolicy::Never, target.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusExpiryPolicyValue = TerminusExpiryPolicy::SessionEnd;
    MessageTarget target(options);
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, target.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.TerminusDurabilityValue = static_cast<TerminusDurability>(655345);

    EXPECT_ANY_THROW(MessageTarget target(options));
  }
  {
    std::chrono::system_clock::time_point expirationTime{
        std::chrono::system_clock::now() + std::chrono::seconds(60)};
    GTEST_LOG_(INFO) << "Expiration time set: " << timeToString(expirationTime);
    MessageTargetOptions options;
    options.Timeout = expirationTime;
    MessageTarget target(options);

    GTEST_LOG_(INFO) << "Expiration time get: " << timeToString(target.GetTimeout());
    EXPECT_EQ(
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch()),
        std::chrono::duration_cast<std::chrono::seconds>(target.GetTimeout().time_since_epoch()));
  }
  {
    MessageTargetOptions options;
    options.Dynamic = true;
    MessageTarget target(options);

    EXPECT_EQ(true, target.GetDynamic());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    MessageTargetOptions options;
    options.Dynamic = false;
    MessageTarget target(options);
    EXPECT_EQ(false, target.GetDynamic());
    GTEST_LOG_(INFO) << "Target: " << target;
  }
  {
    AmqpMap dynamicMap;
    dynamicMap["Key"] = 23;

    MessageTargetOptions options;
    options.DynamicNodeProperties["Key"] = 23;
    MessageTarget target(options);

    auto map2(target.GetDynamicNodeProperties());
    EXPECT_EQ(map2["Key"], AmqpValue(23));
    GTEST_LOG_(INFO) << "Target: " << target;
  }

  {
    MessageTarget target("address1");
    const AmqpValue v = target.AsAmqpValue();
    AmqpValue value(v);

    MessageTarget target2(value);
    EXPECT_EQ(target.GetAddress(), target2.GetAddress());
  }
}

TEST_F(TestSourceTarget, TargetCreateCopy)
{
  {
    MessageTarget target("address1");
    const AmqpValue v = target.AsAmqpValue();
    //    AmqpValue value(v);

    //    MessageTarget target2(value);
    MessageTarget target2(v);
    EXPECT_EQ(target.GetAddress(), target2.GetAddress());
  }
}

TEST_F(TestSourceTarget, TargetThroughValue)
{
  MessageTarget target("address1");
  const AmqpValue v = target.AsAmqpValue();
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
    options.Capabilities.push_back(AmqpSymbol{"Test"}.AsAmqpValue());
    MessageSource source(options);
    EXPECT_EQ(1, source.GetCapabilities().size());
    EXPECT_EQ(AmqpValueType::Symbol, source.GetCapabilities()[0].GetType());
    EXPECT_EQ(source.GetCapabilities()[0].AsSymbol(), "Test");
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSourceOptions options;
    options.SourceTerminusDurability = TerminusDurability::None;

    MessageSource source(options);
    EXPECT_EQ(TerminusDurability::None, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusDurability = TerminusDurability::Configuration;
    MessageSource source(options);

    EXPECT_EQ(TerminusDurability::Configuration, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusDurability = TerminusDurability::UnsettledState;
    MessageSource source(options);
    EXPECT_EQ(TerminusDurability::UnsettledState, source.GetTerminusDurability());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusDurability = static_cast<TerminusDurability>(655345);
    EXPECT_ANY_THROW(MessageSource source(options));
  }

  {
    MessageSourceOptions options;
    options.SourceTerminusExpiryPolicy = TerminusExpiryPolicy::LinkDetach;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::LinkDetach, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusExpiryPolicy = TerminusExpiryPolicy::ConnectionClose;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::ConnectionClose, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusExpiryPolicy = TerminusExpiryPolicy::Never;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::Never, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusExpiryPolicy = TerminusExpiryPolicy::SessionEnd;
    MessageSource source(options);
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, source.GetExpiryPolicy());
    GTEST_LOG_(INFO) << "Source: " << source;
  }
  {
    MessageSourceOptions options;
    options.SourceTerminusDurability = static_cast<TerminusDurability>(655345);

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
    options.Outcomes.push_back(AmqpSymbol("Test").AsAmqpValue());
    MessageSource source(options);
    EXPECT_EQ(1, source.GetOutcomes().size());
    EXPECT_EQ(AmqpValueType::Symbol, source.GetOutcomes().at(0).GetType());
    EXPECT_EQ(source.GetOutcomes().at(0).AsSymbol(), "Test");
    GTEST_LOG_(INFO) << "Source: " << source;
  }

  {
    MessageSource source("address1");
    const AmqpValue v = source.AsAmqpValue();
    AmqpValue value(v);

    MessageSource source2(value);
    EXPECT_EQ(source.GetAddress(), source2.GetAddress());
  }
}
