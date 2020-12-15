// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

using namespace Azure::Core;

namespace Azure { namespace Core { namespace Test {

  class StringClient;

  class StringOperation : public Operation<std::string> {

  private:
    StringClient* m_client;
    std::string m_operationToken;
    std::string m_value;

  private:
    int count = 0;

  public:
    StringOperation(StringClient* client){ m_client = client; }


    std::string GetResumeToken() { return m_operationToken; }

    std::unique_ptr<Http::RawResponse> Poll(Context& context = Context())
    {

      // Artificial delay to require 2 polls
      if (++count == 2)
      {
        m_status = OperationStatus::Succeeded;
      }
      // Tests fake out the network and might throw
      //  context.ThrowIfCanceled();

      return std::make_unique<Http::RawResponse>(
          (uint16_t)1, (uint16_t)0, Http::HttpStatusCode(200), "OK");
    }

    std::string Value()
    {
      if (m_status != OperationStatus::Succeeded)
        throw std::runtime_error("InvalidOperation");
      
      return m_value;
    }

    Response<std::string> PollUntilDone(std::chrono::milliseconds period, Context& context = Context())
    {
      std::unique_ptr<Http::RawResponse> response;
      while (!Done())
      {
        // Sleep for the period
        // std::this_thread::sleep(period); // Update to use retry header if it exists

        response = Poll(context);
      }

      m_value = "OperationCompleted";
      return Response<std::string>(m_value, std::move(response));
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
