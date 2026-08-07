// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/common/runtime_context.hpp"

#include <azure/core/url.hpp>

#include <cstdint>
#include <stdexcept>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Generate a Shared Access Signature token for a ServiceBus client.
  //
  // The spec for a SharedAccessSignature is here:
  // https://learn.microsoft.com/en-us/azure/service-bus-messaging/service-bus-sas#generate-a-shared-access-signature-token
  // Samples for SAS generation are here:
  // https://learn.microsoft.com/en-us/rest/api/eventhub/generate-sas-token
  //
  std::string ServiceBusSasConnectionStringCredential::GenerateSasToken(
      std::chrono::system_clock::time_point const& expirationTime) const
  {
    // These two lines are duplicated from the uAMQP backend on purpose. Azure::Core::Url
    // lowercases the scheme and drops a trailing slash, so both backends must build the
    // resource URI the same way to produce the same token.
    Azure::Core::Url resourceUri{GetEndpoint()};
    resourceUri.AppendPath(GetEntityPath());
    std::string const absoluteUri{resourceUri.GetAbsoluteUrl()};

    auto const expiresOn{
        std::chrono::duration_cast<std::chrono::seconds>(expirationTime.time_since_epoch())
            .count()};

    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), {});

    // The Rust implementation percent-encodes the resource URI, so pass it unencoded.
    char* token = Azure::Core::Amqp::RustInterop::_detail::sastoken_create(
        callContext.GetCallContext(),
        absoluteUri.c_str(),
        GetSharedAccessKeyName().c_str(),
        GetSharedAccessKey().c_str(),
        static_cast<uint64_t>(expiresOn));
    if (token == nullptr)
    {
      // Never return an empty token. An empty token must not reach the CBS layer.
      throw std::runtime_error("Could not create SAS token: " + callContext.GetError());
    }

    std::string tokenString{token};
    Azure::Core::Amqp::RustInterop::_detail::rust_string_delete(token);
    return tokenString;
  }
}}}} // namespace Azure::Core::Amqp::_internal
