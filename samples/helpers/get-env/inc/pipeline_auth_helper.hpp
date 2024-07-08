// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>

#include <memory>
#include <string>
#include <utility>

class PipelineAuthHelper final {
public:
  static const std::shared_ptr<Azure::Core::Credentials::TokenCredential> GetSampleCredentials()
  {
    try
    {
      // the ENVs are defined only by the pipeline and not by the user thus this will throw when
      // trying to get ENVs outside of the pipeline thus will fall back on the default azure
      // credential
      return std::make_shared<Azure::Identity::AzurePipelinesCredential>(
          Azure::Core::_internal::Environment::GetVariable("AZURESUBSCRIPTION_TENANT_ID"),
          Azure::Core::_internal::Environment::GetVariable("AZURESUBSCRIPTION_CLIENT_ID"),
          Azure::Core::_internal::Environment::GetVariable(
              "AZURESUBSCRIPTION_SERVICE_CONNECTION_ID"),
          Azure::Core::_internal::Environment::GetVariable("SYSTEM_ACCESSTOKEN"));
    }
    catch (...)
    {
      return std::make_shared<Azure::Identity::DefaultAzureCredential>();
    };
  }
};
