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

namespace Azure { namespace Core { namespace Test {

  class StringClient;

  class StringOperation : public Operation<std::string> {

  private:
    StringClient* m_client;
    std::string m_operationToken;
    std::string m_value;

  private:
    int m_count = 0;

  private:
    std::unique_ptr<Http::RawResponse> PollInternal(Context& context) override
    {
      // Artificial delay to require 2 polls
      if (++m_count == 2)
      {
        m_status = OperationStatus::Succeeded;
        m_value = "OperationCompleted";
      }
      // Tests fake out the network and might throw
      //  context.ThrowIfCanceled();

      return std::make_unique<Http::RawResponse>(
          (uint16_t)1, (uint16_t)0, Http::HttpStatusCode(200), "OK");
    }

    Response<std::string> PollUntilDoneInternal(std::chrono::milliseconds period, Context& context)
        override
    {
      std::unique_ptr<Http::RawResponse> response;
      while (!Done())
      {
        // Sleep for the period
        // std::this_thread::sleep(period); // Update to use retry header if it exists

        response = Poll(context);
      }

      return Response<std::string>(m_value, std::move(response));
    }

  public:
    StringOperation(StringClient* client) : m_client(client) {}

    std::string GetResumeToken() const override { return m_operationToken; }

    std::string Value() const override
    {
      if (m_status != OperationStatus::Succeeded)
      {
        throw std::runtime_error("InvalidOperation");
      }

      return m_value;
    }
  };

  class StringClient {
  public:
    StringOperation StartStringUpdate()
    {
      // Make initial String call
      StringOperation operation = StringOperation(this);
      return operation;
    }
  };

}}} // namespace Azure::Core::Test
