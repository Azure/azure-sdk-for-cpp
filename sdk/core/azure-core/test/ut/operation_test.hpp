//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>
#include <memory>
#include <string>
#include <thread>

namespace Azure { namespace Core { namespace Test {

  class StringClient;

  class StringOperation final : public Operation<std::string> {

  private:
    std::string m_operationToken;
    std::string m_value;
    int32_t m_count = 0;

  private:
    std::unique_ptr<Http::RawResponse> PollInternal(Context const&) override
    {
      // Artificial delay to require 2 polls
      if (++m_count == 2)
      {
        m_status = OperationStatus::Succeeded;
        m_value = "StringOperation-Completed";
      }

      // The contents of the response are irrelevant for testing purposes
      // Need only ensure that a RawResponse is returned
      return std::make_unique<Http::RawResponse>(1, 0, Http::HttpStatusCode(200), "OK");
    }

    Response<std::string> PollUntilDoneInternal(std::chrono::milliseconds period, Context& context)
        override
    {
      while (!IsDone())
      {
        // Sleep for the period
        // Actual clients should respect the retry after header if present
        std::this_thread::sleep_for(period);

        // Poll gets a new rawResponse and replace it inside the Operation. Then return a ref to
        // that raw response.
        auto const& response = Poll(context);
        // We can use the rawResponse from the Operation here
        // status code is mocked on `PollInternal`
        EXPECT_EQ("OK", response.GetReasonPhrase());
      }

      return Response<std::string>(m_value, std::make_unique<Http::RawResponse>(*m_rawResponse));
    }

    StringOperation(std::string const& resumeToken, StringClient const&)
        : m_operationToken(resumeToken)
    {
    }

  public:
    StringOperation() = default;

    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::string GetResumeToken() const override { return m_operationToken; }

    std::string Value() const override
    {
      if (m_status != OperationStatus::Succeeded)
      {
        throw std::runtime_error("InvalidOperation");
      }

      return m_value;
    }

    // This is a helper method to allow testing of the underlying operation<T> behaviors
    //  ClientOperations would not expose a way to control status
    void SetOperationStatus(OperationStatus status) { m_status = status; }

    static StringOperation CreateFromResumeToken(
        std::string const& resumeToken,
        StringClient const& client)
    {
      StringOperation operation(resumeToken, client);
      operation.Poll();
      return operation;
    }
  };

  class StringClient final {
  public:
    StringOperation StartStringUpdate()
    {
      // Make initial String call
      StringOperation operation = StringOperation();
      return operation;
    }
  };

}}} // namespace Azure::Core::Test
