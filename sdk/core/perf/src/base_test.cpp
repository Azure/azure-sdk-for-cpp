// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/perf/base_test.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/http/pipeline.hpp"

#include <functional>
#include <string>
#include <vector>

using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core;
namespace {

bool IsPlayBackMode = false;
std::string RecordId;

class ProxyPolicy final : public HttpPolicy {
private:
  Azure::Core::Url m_proxy;

public:
  ProxyPolicy(std::string const& proxy) : m_proxy(proxy) {}

  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    if (IsPlayBackMode)
    {
      std::cout << "-----From POLICY PLAYBACK" << std::endl;
    }
    else
    {
      std::cout << "-----From POLICY NON PLAYBACK" << std::endl;
    }

    return nextPolicy.Send(request, context);
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<ProxyPolicy>(*this);
  }
};

} // namespace

namespace Azure { namespace Perf {

  void BaseTest::ConfigureCoreClientOptions(Azure::Core::_internal::ClientOptions* clientOptions)
  {
    std::string proxy(m_options.GetOptionOrDefault("Proxy", ""));
    if (!proxy.empty())
    {
      // If proxy is set in the options, the proxy policy is attached in RECORD MODE
      clientOptions->PerRetryPolicies.push_back(std::make_unique<ProxyPolicy>(proxy));
    }
  }

  void BaseTest::PostSetUp()
  {
    std::string proxy(m_options.GetOptionOrDefault("Proxy", ""));

    if (!proxy.empty())
    {
      Azure::Core::_internal::ClientOptions clientOp;
      clientOp.Retry.MaxRetries = 0;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesOp;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesRe;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOp, "PerfFw", "na", std::move(policiesRe), std::move(policiesOp));
      Azure::Core::Context ctx;

      // Send start-record call
      {
        Azure::Core::Url startRecordReq(proxy);
        startRecordReq.AppendPath("record");
        startRecordReq.AppendPath("start");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startRecordReq);
        auto response = pipeline.Send(request, ctx);

        auto const& headers = response->GetHeaders();
        auto findHeader = std::find_if(
            headers.begin(),
            headers.end(),
            [](std::pair<std::string const&, std::string const&> h) {
              return h.first == "x-recording-id";
            });
        RecordId.append(findHeader->second);
      }

      // play one test to generate a recording
      this->Run(ctx);

      // Stop recording
      {
        Azure::Core::Url stopRecordReq(proxy);
        stopRecordReq.AppendPath("record");
        stopRecordReq.AppendPath("stop");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopRecordReq);
        request.SetHeader("x-recording-id", RecordId);
        pipeline.Send(request, ctx);
      }

      // Start playback
      {
        Azure::Core::Url startPlaybak(proxy);
        startPlaybak.AppendPath("playback");
        startPlaybak.AppendPath("start");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startPlaybak);
        request.SetHeader("x-recording-id", RecordId);
        pipeline.Send(request, ctx);
      }
    }
  }

}} // namespace Azure::Perf
