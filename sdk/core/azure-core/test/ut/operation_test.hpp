// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/response.hpp>

using namespace Azure::Core;

namespace Azure { namespace Core { namespace Test {

  class StringClient;

  class StringOperation : Operation<std::string> {

  private:
    StringClient* m_client;
    bool m_operationComplete = false;
    std::string m_operationToken;

    int count = 0;

  public:
    StringOperation(StringClient* client): m_client(client){};

    bool Done() { return m_operationComplete; }

    std::string GetResumeToken() { return m_operationToken; }

    std::unique_ptr<Http::RawResponse> Poll(Context& context)
    {
      //Artificial delay to require 2 polls
      if (++count == 2)
      {
        m_operationComplete = true;
      }

      return std::make_unique<Http::RawResponse>(
          (uint16_t)1, (uint16_t)0, Http::HttpStatusCode(200), "OK");
    }

    Response<std::string> PollUntilDone(
        Context& context,
        std::chrono::milliseconds period)
    {

      while (!Done())
      {
        //Sleep for the period
        //std::this_thread::sleep(period); // Update to use retry header if it exists
        context.ThrowIfCanceled();
        auto response = Poll(context);
        if (Done())
        {
          std::string value("OperationCompleted");
          return Response<std::string>(value, std::move(response));
        }
      }
    }
  };

  class StringClient {
  public:
    StringOperation StartStringUpdate()
    {
      //Make initial String call
      StringOperation operation = StringOperation(this);
      return operation;
    }
  };

}}} // namespace Azure::Core::Test
