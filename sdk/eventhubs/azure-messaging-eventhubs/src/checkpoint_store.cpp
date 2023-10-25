// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/checkpoint_store.hpp"

#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/strings.hpp>

#include <stdexcept>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  std::string Ownership::GetOwnershipName() const
  {
    if (PartitionId.empty())
    {
      throw std::runtime_error("missing ownership fields");
    }
    std::stringstream strstr;
    strstr << GetOwnershipPrefixName() << PartitionId;
    return strstr.str();
  }

  std::string Ownership::GetOwnershipPrefixName() const
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

  std::string Checkpoint::GetCheckpointBlobName() const
  {
    if (PartitionId.empty())
    {
      throw std::runtime_error("missing checkpoint fields");
    }
    return GetCheckpointBlobPrefixName() + PartitionId;
  }

  std::ostream& operator<<(std::ostream& os, Ownership const& value)
  {
    os << "Ownership = (";
    os << "ConsumerGroup = " << value.ConsumerGroup << ", ";
    os << "EventHubName = " << value.EventHubName << ", ";
    os << "FullyQualifiedNamespace = " << value.FullyQualifiedNamespace << ", ";
    os << "PartitionId = " << value.PartitionId << ", ";
    os << "OwnerId = " << value.OwnerId;
    if (value.ETag.HasValue())
    {
      os << ", ETag = " << value.ETag.Value().ToString();
    }
    if (value.LastModifiedTime.HasValue())
    {
      os << ", LastModifiedTime = " << value.LastModifiedTime.Value().ToString();
    }
    os << ")";

    return os;
  }

  std::ostream& operator<<(std::ostream& os, Checkpoint const& value)
  {

    os << "Checkpoint = (";
    os << "ConsumerGroup = " << value.ConsumerGroup << ", ";
    os << "EventHubName = " << value.EventHubName << ", ";
    os << "FullyQualifiedNamespaceName = " << value.FullyQualifiedNamespaceName << ", ";
    os << "PartitionId = " << value.PartitionId;
    if (value.Offset.HasValue())
    {
      os << ", Offset = " << value.Offset.Value();
    }
    if (value.SequenceNumber.HasValue())
    {
      os << ", SequenceNumber = " << value.SequenceNumber.Value();
    }
    os << ")";
    return os;
  }
}}}} // namespace Azure::Messaging::EventHubs::Models
