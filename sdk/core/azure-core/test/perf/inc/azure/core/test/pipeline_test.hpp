// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the HTTP send performance.
 *
 */

#pragma once

#include "../../../core/perf/inc/azure/perf.hpp"

#include <azure/core.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <algorithm>
#include <memory>
using namespace Azure::Core;
using namespace Azure::Core::_internal;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace Azure { namespace Core { namespace Test {

  class TestPolicy : public HttpPolicy {

  public:
    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TestPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(
        Request& request,
        NextHttpPolicy nextPolicy,
        Context const& context) const override
    {
      return nextPolicy.Send(request, context);
    };
  };

  /**
   * @brief Measure the http pipeline / policies performance.
   */
  class PipelineTest : public Azure::Perf::PerfTest {
    std::unique_ptr<HttpPipeline> m_pipeline;

    std::vector<std::string> SplitString(const std::string& s, char separator)
    {
      std::vector<std::string> result;

      const auto len = s.size();
      size_t start = 0;
      while (start < len)
      {
        auto end = s.find(separator, start);
        if (end == std::string::npos)
        {
          end = len;
        }

        result.push_back(s.substr(start, end - start));

        start = end + 1;
      }

      return result;
    }

  public:
    /**
     * @brief Construct a new PipelineTest test.
     *
     * @param options The test options.
     */
    PipelineTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    void Setup() override
    {
      const std::string packageName = "test";
      const std::string packageVersion = "1.0.0";
      const std::string testPolicyName = "TestPolicy";
      const std::string retryPolicyName = "RetryPolicy";
      const std::string requestIdPolicyName = "RequestIdPolicy";
      const std::string requestActivityPolicyName = "RequestActivityPolicy";
      const std::string telemetryPolicyName = "TelemetryPolicy";
      const std::string logPolicyName = "LogPolicy";

      HttpSanitizer httpSanitizer;

      std::vector<std::unique_ptr<HttpPolicy>> policies;
      std::vector<std::unique_ptr<HttpPolicy>> policies2;

      auto const total = m_options.GetMandatoryOption<int>("Count");
      auto const policyNames
          = SplitString(m_options.GetOptionOrDefault<std::string>("Policies", "TestPolicy"), ',');
      // we want a total number of policies added to the pipeline
      // thus for loop total / number , depends on rounding but close enough
      // since in each loop we add the whole set of desired policies
      // we also get stack overflow with lots of policies since the pipeline is a two level
      // recursion
      for (int i = 0; i < total / policyNames.size(); i++)
      {
        if (std::find(policyNames.begin(), policyNames.end(), testPolicyName) != policyNames.end())
        {
          policies.push_back(std::make_unique<TestPolicy>());
          policies2.push_back(std::make_unique<TestPolicy>());
        }
        if (std::find(policyNames.begin(), policyNames.end(), retryPolicyName) != policyNames.end())
        {
          policies.push_back(std::make_unique<RetryPolicy>(RetryOptions{}));
          policies2.push_back(std::make_unique<RetryPolicy>(RetryOptions{}));
        }
        if (std::find(policyNames.begin(), policyNames.end(), requestIdPolicyName)
            != policyNames.end())
        {
          policies.push_back(std::make_unique<RequestIdPolicy>());
          policies2.push_back(std::make_unique<RequestIdPolicy>());
        }
        if (std::find(policyNames.begin(), policyNames.end(), requestActivityPolicyName)
            != policyNames.end())
        {
          policies.push_back(std::make_unique<RequestActivityPolicy>(httpSanitizer));
          policies2.push_back(std::make_unique<RequestActivityPolicy>(httpSanitizer));
        }
        if (std::find(policyNames.begin(), policyNames.end(), telemetryPolicyName)
            != policyNames.end())
        {
          policies.push_back(std::make_unique<TelemetryPolicy>(packageName, packageVersion));
          policies2.push_back(std::make_unique<TelemetryPolicy>(packageName, packageVersion));
        }
        if (std::find(policyNames.begin(), policyNames.end(), logPolicyName) != policyNames.end())
        {
          policies.push_back(std::make_unique<LogPolicy>(LogOptions()));
          policies2.push_back(std::make_unique<LogPolicy>(LogOptions()));
        }
      }

      m_pipeline = std::make_unique<HttpPipeline>(
          ClientOptions(), packageName, packageVersion, std::move(policies), std::move(policies2));
    }

    /**
     * @brief Executes the pipeline
     *
     */
    void Run(Context const&) override
    {
      try
      {
        Azure::Core::Http::Request request(
            HttpMethod::Get, Url("http://127.0.0.1:5000/admin/isalive"));
        Context context;
        m_pipeline->Send(request, context);
      }
      catch (std::exception const&)
      {
        // don't print exceptions, they are happening at each request, this is the point of the test
      }
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"Count", {"--count"}, "The number of policy objects to be created.", 1, true},
          {"Policies",
           {"--policies"},
           "The policies to be added to the pipeline. Allows multiple values comma separated.\n"
           "default:TestPolicy \n others: "
           "RetryPolicy,RequestIdPolicy,RequestActivityPolicy,TelemetryPolicy,LogPolicy",
           1,
           false}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "PipelineBaseTest",
          "Measures HTTP pipeline and policies performance",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::PipelineTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test
