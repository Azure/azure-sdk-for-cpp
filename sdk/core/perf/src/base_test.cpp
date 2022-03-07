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

class ProxyPolicy final : public HttpPolicy {
private:
  Azure::Perf::BaseTest* m_testContext;

public:
  ProxyPolicy(Azure::Perf::BaseTest* proxyManager) : m_testContext(proxyManager) {}

  // copy
  ProxyPolicy(ProxyPolicy const& other) : ProxyPolicy{other.m_testContext} {}

  // move
  ProxyPolicy(ProxyPolicy&& other) : m_testContext{other.m_testContext} {}

  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    std::string const recordId(m_testContext->m_recordId);
    if (recordId.empty())
    {
      return nextPolicy.Send(request, context);
    }

    // Use a new request to redirect
    auto redirectRequest = Azure::Core::Http::Request(
        request.GetMethod(), Azure::Core::Url(m_testContext->m_proxy), request.GetBodyStream());
    if (!request.ShouldBufferResponse())
    {
      // This is a download with keep connection open. Let's switch the request
      redirectRequest = Azure::Core::Http::Request(
          request.GetMethod(), Azure::Core::Url(m_testContext->m_proxy), false);
    }
    redirectRequest.GetUrl().SetPath(request.GetUrl().GetPath());

    // Copy all headers
    for (auto& header : request.GetHeaders())
    {
      redirectRequest.SetHeader(header.first, header.second);
    }
    // QP
    for (auto const& qp : request.GetUrl().GetQueryParameters())
    {
      redirectRequest.GetUrl().AppendQueryParameter(qp.first, qp.second);
    }
    // Set x-recording-upstream-base-uri
    {
      auto const& url = request.GetUrl();
      auto const port = url.GetPort();
      auto const host
          = url.GetScheme() + "://" + url.GetHost() + (port != 0 ? ":" + std::to_string(port) : "");
      redirectRequest.SetHeader("x-recording-upstream-base-uri", host);
    }
    // Set recording-id
    redirectRequest.SetHeader("x-recording-id", recordId);
    redirectRequest.SetHeader("x-recording-remove", "false");

    // Using recordId, find out MODE
    if (m_testContext->m_isPlayBackMode)
    {
      // PLAYBACK mode
      redirectRequest.SetHeader("x-recording-mode", "playback");
    }
    else
    {
      // RECORDING mode
      redirectRequest.SetHeader("x-recording-mode", "record");
    }

    return nextPolicy.Send(redirectRequest, context);
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
    if (!m_proxy.empty())
    {
      clientOptions->PerRetryPolicies.push_back(std::make_unique<ProxyPolicy>(this));
    }
  }

  void BaseTest::PostSetUp()
  {

    if (!m_proxy.empty())
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
        Azure::Core::Url startRecordReq(m_proxy);
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
        m_recordId = findHeader->second;
      }

      // play one test to generate a recording
      this->Run(ctx);
      // Run twice to align with how all other SDK perf langs run
      this->Run(ctx);

      // Stop recording
      {
        Azure::Core::Url stopRecordReq(m_proxy);
        stopRecordReq.AppendPath("record");
        stopRecordReq.AppendPath("stop");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopRecordReq);
        request.SetHeader("x-recording-id", m_recordId);
        pipeline.Send(request, ctx);
      }

      // Start playback
      {
        Azure::Core::Url startPlayback(m_proxy);
        startPlayback.AppendPath("playback");
        startPlayback.AppendPath("start");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startPlayback);
        request.SetHeader("x-recording-id", m_recordId);
        auto response = pipeline.Send(request, ctx);

        auto const& headers = response->GetHeaders();
        auto findHeader = std::find_if(
            headers.begin(),
            headers.end(),
            [](std::pair<std::string const&, std::string const&> h) {
              return h.first == "x-recording-id";
            });
        m_recordId = findHeader->second;
        m_isPlayBackMode = true;
      }
    }
  }

  void BaseTest::PreCleanUp()
  {
    if (!m_recordId.empty())
    {
      Azure::Core::_internal::ClientOptions clientOp;
      clientOp.Retry.MaxRetries = 0;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesOp;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesRe;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOp, "PerfFw", "na", std::move(policiesRe), std::move(policiesOp));
      Azure::Core::Context ctx;

      // Stop playback
      {
        Azure::Core::Url stopPlaybackReq(m_proxy);
        stopPlaybackReq.AppendPath("playback");
        stopPlaybackReq.AppendPath("stop");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopPlaybackReq);
        request.SetHeader("x-recording-id", m_recordId);
        request.SetHeader("x-purge-inmemory-recording", "true"); // cspell:disable-line

        pipeline.Send(request, ctx);

        m_recordId.clear();
        m_isPlayBackMode = false;
      }
    }
  }

}} // namespace Azure::Perf
