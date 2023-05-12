// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/messaging/eventhubs/checkpoint_store.hpp"

std::string Azure::Messaging::EventHubs::BlobCheckpointStore::GetOwnershipName(
    Azure::Messaging::EventHubs::Ownership const& ownership)
{
  if (ownership.PartitionID.empty())
  {
    throw std::exception("missing ownership fields");
  }
  std::stringstream strstr;
  strstr << GetOwnershipPrefixName(ownership) << ownership.PartitionID;
  return strstr.str();
}

std::string Azure::Messaging::EventHubs::BlobCheckpointStore::GetOwnershipPrefixName(
    Azure::Messaging::EventHubs::Ownership const& ownership)
{
  if (ownership.FullyQualifiedNamespace.empty() || ownership.EventHubName.empty()
      || ownership.ConsumerGroup.empty())
  {
    throw std::exception("missing ownership fields");
  }
  std::stringstream strstr;
  strstr << ownership.FullyQualifiedNamespace << "/" << ownership.EventHubName << "/"
         << ownership.ConsumerGroup << "/ownership/";

  return strstr.str();
}

std::string Azure::Messaging::EventHubs::BlobCheckpointStore::GetCheckpointBlobPrefixName(
    Azure::Messaging::EventHubs::Checkpoint const& checkpoint)
{
  if (checkpoint.FullyQualifiedNamespace.empty() || checkpoint.EventHubName.empty()
      || checkpoint.ConsumerGroup.empty())
  {
    throw std::exception("missing checkpoint fields");
  }
  std::stringstream strstr;
  strstr << checkpoint.FullyQualifiedNamespace << "/" << checkpoint.EventHubName << "/"
         << checkpoint.ConsumerGroup << "/checkpoint/";

  return strstr.str();
}

std::string Azure::Messaging::EventHubs::BlobCheckpointStore::GetCheckpointBlobName(
    Azure::Messaging::EventHubs::Checkpoint const& checkpoint)
{
  if (checkpoint.PartitionID.empty())
  {
    throw std::exception("missing checkpoint fields");
  }
  std::stringstream strstr;
  strstr << GetCheckpointBlobPrefixName(checkpoint) << checkpoint.PartitionID;
  return strstr.str();
}

void Azure::Messaging::EventHubs::BlobCheckpointStore::UpdateCheckpointImpl(
    Azure::Storage::Metadata const& metadata,
    Azure::Messaging::EventHubs::Checkpoint& checkpoint)
{
  std::string temp = metadata.at("sequencenumber");
  if (temp.empty())
  {
    throw std::exception("missing sequence number");
  }
  checkpoint.SequenceNumber = std::stol(temp);

  temp = metadata.at("offset");
  if (temp.empty())
  {
    throw std::exception("missing offset number");
  }

  checkpoint.Offset = std::stol(temp);
}

void Azure::Messaging::EventHubs::BlobCheckpointStore::UpdateOwnership(
    Azure::Storage::Blobs::Models::BlobItem const& blob,
    Azure::Messaging::EventHubs::Ownership& ownership)
{
  std::string temp = blob.Details.Metadata.at("ownerid");
  if (temp.empty())
  {
    throw std::exception("missing sequence number");
  }
  ownership.OwnerID = temp;
  ownership.LastModifiedTime = blob.Details.LastModified;
  ownership.ETag = blob.Details.ETag;
}

Azure::Storage::Metadata
Azure::Messaging::EventHubs::BlobCheckpointStore::NewCheckpointBlobMetadata(
    Azure::Messaging::EventHubs::Checkpoint const& checkpoint)
{
  Azure::Storage::Metadata metadata;

  if (checkpoint.SequenceNumber.HasValue())
  {
    metadata["sequencenumber"] = std::to_string(checkpoint.SequenceNumber.Value());
  }

  if (checkpoint.Offset.HasValue())
  {
    metadata["offset"] = std::to_string(checkpoint.Offset.Value());
  }
  return metadata;
}

std::vector<Azure::Messaging::EventHubs::Ownership>
Azure::Messaging::EventHubs::BlobCheckpointStore::ClaimOwnership(
    std::vector<Azure::Messaging::EventHubs::Ownership> partitionOwnership,
    Azure::Core::Context ctx,
    Azure::Messaging::EventHubs::ClaimOwnershipOptions const& options)
{
  (void)options;
  std::vector<Azure::Messaging::EventHubs::Ownership> newOwnerships;

  for (Azure::Messaging::EventHubs::Ownership ownership : partitionOwnership)
  {
    std::string blobName = GetOwnershipName(ownership);
    Azure::Storage::Metadata metadata;
    metadata["ownerId"] = ownership.OwnerID;
    try
    {
      std::pair<Azure::DateTime, Azure::ETag> result
          = SetMetadata(blobName, metadata, ownership.ETag.ValueOr(Azure::ETag()), ctx);
      if (result.second.HasValue())
      {

        Azure::Messaging::EventHubs::Ownership newOwnership(ownership);
        newOwnership.ETag = result.second;
        newOwnership.LastModifiedTime = result.first;
        newOwnerships.emplace_back(newOwnership);
      }
    }
    catch (...)
    {
      // we can fail to claim ownership and that's okay - it's expected that clients will
      // attempt to claim with whatever state they hold locally. If they fail it just means
      // someone else claimed ownership before them.
      continue;
    }
  }
  return newOwnerships;
}

std::vector<Azure::Messaging::EventHubs::Checkpoint>
Azure::Messaging::EventHubs::BlobCheckpointStore::ListCheckpoints(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHubName,
    std::string const& consumerGroup,
    Azure::Core::Context ctx,
    ListCheckpointsOptions options)
{
  (void)options;

  std::vector<Checkpoint> checkpoints;

  std::string prefix = GetCheckpointBlobPrefixName(
      Checkpoint{consumerGroup, eventHubName, fullyQualifiedNamespace});
  Azure::Storage::Blobs::ListBlobsOptions listOptions;
  listOptions.Prefix = prefix;
  listOptions.Include = Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Metadata;
  for (auto page = m_containerClient.ListBlobs(listOptions, ctx); page.HasPage();
       page.MoveToNextPage())
  {
    for (auto& blob : page.Blobs)
    {
      std::string partitionId = blob.Name.substr(blob.Name.rfind('/') + 1);
      Checkpoint c = Checkpoint{consumerGroup, eventHubName, fullyQualifiedNamespace, partitionId};
      UpdateCheckpointImpl(blob.Details.Metadata, c);
      checkpoints.push_back(c);
    }
  }

  return checkpoints;
}

/**@brief  ListOwnership lists all ownerships.
 */
std::vector<Azure::Messaging::EventHubs::Ownership>
Azure::Messaging::EventHubs::BlobCheckpointStore::ListOwnership(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHubName,
    std::string const& consumerGroup,
    Azure::Core::Context ctx,
    ListOwnershipOptions options)
{
  (void)options;
  std::vector<Ownership> ownerships;
  std::string prefix
      = GetOwnershipPrefixName(Ownership{consumerGroup, eventHubName, fullyQualifiedNamespace});
  Azure::Storage::Blobs::ListBlobsOptions listOptions;
  listOptions.Prefix = prefix;
  listOptions.Include = Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Metadata;

  for (auto page = m_containerClient.ListBlobs(listOptions, ctx); page.HasPage();
       page.MoveToNextPage())
  {
    for (auto& blob : page.Blobs)
    {
      std::string partitionId = blob.Name.substr(blob.Name.rfind('/') + 1);
      Ownership o{consumerGroup, eventHubName, fullyQualifiedNamespace, partitionId};
      UpdateOwnership(blob, o);
      ownerships.push_back(o);
    }
  }

  return ownerships;
}

/**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
 */
void Azure::Messaging::EventHubs::BlobCheckpointStore::UpdateCheckpoint(
    Checkpoint const& checkpoint,
    Azure::Core::Context ctx,
    UpdateCheckpointOptions options)
{
  (void)options;
  std::string blobName = GetCheckpointBlobName(checkpoint);
  SetMetadata(blobName, NewCheckpointBlobMetadata(checkpoint), Azure::ETag(), ctx);
}

std::pair<Azure::DateTime, Azure::ETag>
Azure::Messaging::EventHubs::BlobCheckpointStore::SetMetadata(
    std::string const& blobName,
    Azure::Storage::Metadata const& metadata,
    Azure::ETag const& etag,
    Azure::Core::Context const& context)
{
  auto blobClient = m_containerClient.GetBlockBlobClient(blobName);
  std::pair<Azure::DateTime, Azure::ETag> returnValue;
  Azure::Storage::Blobs::SetBlobMetadataOptions options;

  try
  {
    if (etag.HasValue())
    {
      options.AccessConditions.IfMatch = etag;
    }

    Azure::Storage::Blobs::Models::SetBlobMetadataResult result
        = blobClient.SetMetadata(metadata, options, context).Value;

    returnValue = std::make_pair(result.LastModified, result.ETag);
  }
  catch (std::exception const& ex)
  {
    if (std::string(ex.what()).find("412") == std::string::npos)
    {
      // return code 412 meaning condition could not be met;

      std::string blobContent = "";
      // throws when blob does not exist , we need to upload the blob in order to create it
      std::vector<uint8_t> buffer(blobContent.begin(), blobContent.end());
      Azure::Storage::Blobs::UploadBlockBlobFromOptions upOptions;
      upOptions.Metadata = metadata;
      Azure::Storage::Blobs::Models::UploadBlockBlobFromResult result
          = blobClient.UploadFrom(buffer.data(), buffer.size(), upOptions, context).Value;
      returnValue = std::make_pair(result.LastModified, result.ETag);
    }
  }

  return returnValue;
}