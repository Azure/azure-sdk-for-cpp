// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/checkpoint_store.hpp"

#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/strings.hpp>

#include <stdexcept>

using namespace Azure::Messaging::EventHubs::Models;

std::string Azure::Messaging::EventHubs::Models::Ownership::GetOwnershipName() const
{
  if (PartitionId.empty())
  {
    throw std::runtime_error("missing ownership fields");
  }
  std::stringstream strstr;
  strstr << GetOwnershipPrefixName() << PartitionId;
  return strstr.str();
}

std::string Azure::Messaging::EventHubs::Models::Ownership::GetOwnershipPrefixName() const
{
  if (FullyQualifiedNamespace.empty() || EventHubName.empty() || ConsumerGroup.empty())
  {
    throw std::runtime_error("missing ownership fields");
  }
  std::stringstream strstr;
  strstr << Azure::Core::_internal::StringExtensions::ToLower(FullyQualifiedNamespace) << "/"
         << Azure::Core::_internal::StringExtensions::ToLower(EventHubName) << "/"
         << Azure::Core::_internal::StringExtensions::ToLower(ConsumerGroup) << "/ownership/";

  return strstr.str();
}

std::string Azure::Messaging::EventHubs::Models::Checkpoint::GetCheckpointBlobPrefixName() const
{
  if (FullyQualifiedNamespaceName.empty() || EventHubName.empty() || ConsumerGroup.empty())
  {
    throw std::runtime_error("missing checkpoint fields");
  }
  std::stringstream strstr;
  strstr << Azure::Core::_internal::StringExtensions::ToLower(FullyQualifiedNamespaceName) << "/"
         << Azure::Core::_internal::StringExtensions::ToLower(EventHubName) << "/"
         << Azure::Core::_internal::StringExtensions::ToLower(ConsumerGroup) << "/checkpoint/";

  return strstr.str();
}

std::string Azure::Messaging::EventHubs::Models::Checkpoint::GetCheckpointBlobName() const
{
  if (PartitionId.empty())
  {
    throw std::runtime_error("missing checkpoint fields");
  }
  return GetCheckpointBlobPrefixName() + PartitionId;
}

