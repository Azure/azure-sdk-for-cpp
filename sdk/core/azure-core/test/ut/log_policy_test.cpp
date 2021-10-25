// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <gtest/gtest.h>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Http::Policies::LogOptions;

// cspell:ignore qparam

namespace {
void SendRequest(LogOptions const& logOptions)
{
  using namespace Azure::Core;
  using namespace Azure::Core::IO;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Http::Policies;
  using namespace Azure::Core::Http::Policies::_internal;

  class TestTransportPolicy final : public HttpPolicy {
  public:
    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TestTransportPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(Request&, NextHttpPolicy, Context const&) const override
    {
      static constexpr uint8_t const responseBody[] = "Response Body";
      static constexpr uint8_t const responseBodyStream[] = "Request Body Stream";

      auto response = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "OKAY");

      response->SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));

      response->SetBodyStream(
          std::make_unique<MemoryBodyStream>(responseBodyStream, sizeof(responseBodyStream) - 1));

      return response;
    }
  };

  constexpr uint8_t const requestBodyStream[] = "Request Body Stream";
  auto const bodyStream
      = std::make_unique<MemoryBodyStream>(requestBodyStream, sizeof(requestBodyStream) - 1);

  Request request(
      HttpMethod::Get,
      Url("https://"
          "www.microsoft.com"
          "?qparam1=qVal1"
          "&Qparam2=Qval2"
          "&qParam3=qval3"
          "&qparam%204=qval%204"
          "&qparam%25204=QVAL%25204"),
      bodyStream.get());

  request.SetHeader("hEaDeR1", "HvAlUe1");
  request.SetHeader("HeAdEr2", "hVaLuE2");
  request.SetHeader("x-ms-request-id", "6c536700-4c36-4e22-9161-76e7b3bf8269");

  {
    std::vector<std::unique_ptr<HttpPolicy>> policies;

    policies.emplace_back(std::make_unique<LogPolicy>(logOptions));
    policies.emplace_back(std::make_unique<TestTransportPolicy>());

    HttpPipeline(policies).Send(request, Azure::Core::Context());
  }
}

class TestLogger final {
private:
  static void Deinitialize()
  {
    Logger::SetLevel(Logger::Level::Error);
    Logger::SetListener(nullptr);
  }

  TestLogger(TestLogger const&) = delete;
  void operator=(TestLogger const&) = delete;

public:
  struct LogMessage final
  {
    Logger::Level Level;
    std::string Message;
  };

  std::vector<LogMessage> Entries;

  ~TestLogger() { Deinitialize(); }

  TestLogger()
  try
  {
    Logger::SetLevel(Logger::Level::Verbose);
    Logger::SetListener([&](auto lvl, auto msg) { Entries.push_back({lvl, msg}); });
  }
  catch (...)
  {
    Deinitialize();
    throw;
  }
};

bool StartsWith(std::string const& str, std::string const& with)
{
  return str.substr(0, with.size()) == with;
}

bool EndsWith(std::string const& str, std::string const& with)
{
  if (str.size() < with.size())
  {
    return false;
  }

  return str.substr(str.size() - with.size(), with.size()) == with;
}
} // namespace

TEST(LogPolicy, Default)
{
  TestLogger const Log;
  SendRequest(LogOptions());

  EXPECT_EQ(Log.Entries.size(), 2);

  auto const entry1 = Log.Entries.at(0);
  auto const entry2 = Log.Entries.at(1);

  EXPECT_EQ(entry1.Level, Logger::Level::Informational);
  EXPECT_EQ(entry2.Level, Logger::Level::Informational);

  EXPECT_EQ(
      entry1.Message,
      "HTTP Request : GET https://www.microsoft.com"
      "?Qparam2=REDACTED"
      "&qParam3=REDACTED"
      "&qparam%204=REDACTED"
      "&qparam%25204=REDACTED"
      "&qparam1=REDACTED"
      "\nheader1 : REDACTED"
      "\nheader2 : REDACTED"
      "\nx-ms-request-id : 6c536700-4c36-4e22-9161-76e7b3bf8269");

  EXPECT_TRUE(StartsWith(entry2.Message, "HTTP Response ("));
  EXPECT_TRUE(EndsWith(entry2.Message, "ms) : 200 OKAY"));
}

TEST(LogPolicy, Headers)
{
  TestLogger const Log;

  {
    auto logOptions = LogOptions();
    logOptions.AllowedHttpHeaders.insert({"HeAder1", "heaDer3"});
    SendRequest(logOptions);
  }

  EXPECT_EQ(Log.Entries.size(), 2);

  auto const entry1 = Log.Entries.at(0);
  auto const entry2 = Log.Entries.at(1);

  EXPECT_EQ(entry1.Level, Logger::Level::Informational);
  EXPECT_EQ(entry2.Level, Logger::Level::Informational);

  EXPECT_EQ(
      entry1.Message,
      "HTTP Request : GET https://www.microsoft.com"
      "?Qparam2=REDACTED"
      "&qParam3=REDACTED"
      "&qparam%204=REDACTED"
      "&qparam%25204=REDACTED"
      "&qparam1=REDACTED"
      "\nheader1 : HvAlUe1"
      "\nheader2 : REDACTED"
      "\nx-ms-request-id : 6c536700-4c36-4e22-9161-76e7b3bf8269");

  EXPECT_TRUE(StartsWith(entry2.Message, "HTTP Response ("));
  EXPECT_TRUE(EndsWith(entry2.Message, "ms) : 200 OKAY"));
}

TEST(LogPolicy, QueryParams)
{
  TestLogger const Log;
  SendRequest(LogOptions({{"qparam1", "qparam2", "qParam3"}, {}}));

  EXPECT_EQ(Log.Entries.size(), 2);

  auto const entry1 = Log.Entries.at(0);
  auto const entry2 = Log.Entries.at(1);

  EXPECT_EQ(entry1.Level, Logger::Level::Informational);
  EXPECT_EQ(entry2.Level, Logger::Level::Informational);

  EXPECT_EQ(
      entry1.Message,
      "HTTP Request : GET https://www.microsoft.com"
      "?Qparam2=REDACTED"
      "&qParam3=qval3"
      "&qparam%204=REDACTED"
      "&qparam%25204=REDACTED"
      "&qparam1=qVal1"
      "\nheader1 : REDACTED"
      "\nheader2 : REDACTED"
      "\nx-ms-request-id : REDACTED");

  EXPECT_TRUE(StartsWith(entry2.Message, "HTTP Response ("));
  EXPECT_TRUE(EndsWith(entry2.Message, "ms) : 200 OKAY"));
}

TEST(LogPolicy, QueryParamsUnencoded)
{
  TestLogger const Log;
  SendRequest(LogOptions({{"qparam 4"}, {}}));

  EXPECT_EQ(Log.Entries.size(), 2);

  auto const entry1 = Log.Entries.at(0);
  auto const entry2 = Log.Entries.at(1);

  EXPECT_EQ(entry1.Level, Logger::Level::Informational);
  EXPECT_EQ(entry2.Level, Logger::Level::Informational);

  EXPECT_EQ(
      entry1.Message,
      "HTTP Request : GET https://www.microsoft.com"
      "?Qparam2=REDACTED"
      "&qParam3=REDACTED"
      "&qparam%204=qval%204"
      "&qparam%25204=REDACTED"
      "&qparam1=REDACTED"
      "\nheader1 : REDACTED"
      "\nheader2 : REDACTED"
      "\nx-ms-request-id : REDACTED");

  EXPECT_TRUE(StartsWith(entry2.Message, "HTTP Response ("));
  EXPECT_TRUE(EndsWith(entry2.Message, "ms) : 200 OKAY"));
}

TEST(LogPolicy, QueryParamsEncoded)
{
  TestLogger const Log;
  SendRequest(LogOptions({{"qparam%204"}, {}}));

  EXPECT_EQ(Log.Entries.size(), 2);

  auto const entry1 = Log.Entries.at(0);
  auto const entry2 = Log.Entries.at(1);

  EXPECT_EQ(entry1.Level, Logger::Level::Informational);
  EXPECT_EQ(entry2.Level, Logger::Level::Informational);

  EXPECT_EQ(
      entry1.Message,
      "HTTP Request : GET https://www.microsoft.com"
      "?Qparam2=REDACTED"
      "&qParam3=REDACTED"
      "&qparam%204=REDACTED"
      "&qparam%25204=QVAL%25204"
      "&qparam1=REDACTED"
      "\nheader1 : REDACTED"
      "\nheader2 : REDACTED"
      "\nx-ms-request-id : REDACTED");

  EXPECT_TRUE(StartsWith(entry2.Message, "HTTP Response ("));
  EXPECT_TRUE(EndsWith(entry2.Message, "ms) : 200 OKAY"));
}
