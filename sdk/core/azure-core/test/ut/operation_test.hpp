// Copyright (c) Microsoft Corporation. All rights reserved.
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

  class StringOperation : public Operation<std::string> {

  private:
    std::string m_operationToken;
    std::string m_value;
    std::unique_ptr<Azure::Core::Http::RawResponse> m_rawResponse;

  private:
    int m_count = 0;

  private:
    std::unique_ptr<Http::RawResponse> PollInternal(Context&) override
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
      std::unique_ptr<Http::RawResponse> response;
      while (!IsDone())
      {
        // Sleep for the period
        // Actual clients should respect the retry after header if present
        std::this_thread::sleep_for(period);

        response = Poll(context);
      }

      return Response<std::string>(m_value, std::move(response));
    }

  public:
    Azure::Core::Http::RawResponse* GetRawResponse() const override { return m_rawResponse.get(); }

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
  };

  class StringClient {
  public:
    StringOperation StartStringUpdate()
    {
      // Make initial String call
      StringOperation operation = StringOperation();
      return operation;
    }
  };

}}} // namespace Azure::Core::Test
