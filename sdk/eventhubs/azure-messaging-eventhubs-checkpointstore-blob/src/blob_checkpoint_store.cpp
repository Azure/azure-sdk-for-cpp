// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/checkpointstore_blob/blob_checkpoint_store.hpp"

#include "azure/messaging/eventhubs/checkpoint_store.hpp"

#include <azure/core/internal/diagnostics/log.hpp>

#include <stdexcept>

// cspell: ignore ownerid

using namespace Azure::Messaging::EventHubs::Models;

void Azure::Messaging::EventHubs::BlobCheckpointStore::UpdateCheckpointImpl(
    Azure::Storage::Metadata const& metadata,
    Checkpoint& checkpoint)
{
  std::string temp = metadata.at("sequencenumber");
  if (temp.empty())
  {
    throw std::runtime_error("missing sequence number");
  }
  checkpoint.SequenceNumber = std::stol(temp);

  temp = metadata.at("offset");
  if (temp.empty())
  {
    throw std::runtime_error("missing offset number");
  }

  checkpoint.Offset = temp;
}

void Azure::Messaging::EventHubs::BlobCheckpointStore::UpdateOwnership(
    Azure::Storage::Blobs::Models::BlobItem const& blob,
    Ownership& ownership)
{
  std::string temp = blob.Details.Metadata.at("ownerid");
  if (temp.empty())
  {
    throw std::runtime_error("missing sequence number");
  }
  ownership.OwnerId = temp;
  ownership.LastModifiedTime = blob.Details.LastModified;
  ownership.ETag = blob.Details.ETag;
}

Azure::Storage::Metadata
Azure::Messaging::EventHubs::BlobCheckpointStore::CreateCheckpointBlobMetadata(
    Checkpoint const& checkpoint)
{
  Azure::Storage::Metadata metadata;

  if (checkpoint.SequenceNumber.HasValue())
  {
    metadata["sequencenumber"] = std::to_string(checkpoint.SequenceNumber.Value());
  }

  if (checkpoint.Offset.HasValue())
  {
    metadata["offset"] = checkpoint.Offset.Value();
  }
  return metadata;
}

std::vector<Ownership> Azure::Messaging::EventHubs::BlobCheckpointStore::ClaimOwnership(
    std::vector<Ownership> const& partitionOwnership,
    Core::Context const& context)
{
  std::vector<Ownership> newOwnerships;

  for (Ownership ownership : partitionOwnership)
  {
    std::string blobName = ownership.GetOwnershipName();
    Azure::Storage::Metadata metadata;
    metadata["ownerId"] = ownership.OwnerId;
    try
    {
      std::pair<Azure::DateTime, Azure::ETag> result
          = SetMetadata(blobName, metadata, ownership.ETag.ValueOr(Azure::ETag()), context);
      if (result.second.HasValue())
      {

        Ownership newOwnership(ownership);
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

std::vector<Checkpoint> Azure::Messaging::EventHubs::BlobCheckpointStore::ListCheckpoints(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHubName,
    std::string const& consumerGroup,
    Core::Context const& context)
{
  std::vector<Checkpoint> checkpoints;

  std::string prefix = Models::Checkpoint{consumerGroup, eventHubName, fullyQualifiedNamespace}
                           .GetCheckpointBlobPrefixName();
  Azure::Storage::Blobs::ListBlobsOptions listOptions;
  listOptions.Prefix = prefix;
  listOptions.Include = Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Metadata;
  for (auto page = m_containerClient.ListBlobs(listOptions, context); page.HasPage();
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
std::vector<Ownership> Azure::Messaging::EventHubs::BlobCheckpointStore::ListOwnership(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHubName,
    std::string const& consumerGroup,
    Core::Context const& context)
{
  std::vector<Ownership> ownerships;
  std::string prefix
      = Ownership{consumerGroup, eventHubName, fullyQualifiedNamespace}.GetOwnershipPrefixName();
  Azure::Storage::Blobs::ListBlobsOptions listOptions;
  listOptions.Prefix = prefix;
  listOptions.Include = Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Metadata;

  for (auto page = m_containerClient.ListBlobs(listOptions, context); page.HasPage();
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
    Core::Context const& context)
{
  std::string blobName = checkpoint.GetCheckpointBlobName();
  SetMetadata(blobName, CreateCheckpointBlobMetadata(checkpoint), Azure::ETag(), context);
}

std::pair<Azure::DateTime, Azure::ETag>
Azure::Messaging::EventHubs::BlobCheckpointStore::SetMetadata(
    std::string const& blobName,
    Azure::Storage::Metadata const& metadata,
    Azure::ETag const& etag,
    Core::Context const& context)
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
  catch (Azure::Core::RequestFailedException const& ex)
  {
    // Ignore HTTP code 412 meaning condition could not be met;
    if (ex.StatusCode == Azure::Core::Http::HttpStatusCode::PreconditionFailed)
    {
    }
    if (ex.StatusCode == Azure::Core::Http::HttpStatusCode::NotFound)
    {
      Azure::Core::Diagnostics::_internal::Log::Write(
          Azure::Core::Diagnostics::Logger::Level::Warning,
          "Set Metadata failed with PreconditionFailed or NotFound.; Upload blob content");

      std::string blobContent = "";
      // throws when blob does not exist , we need to upload the blob in order to create it
      std::vector<uint8_t> buffer(blobContent.begin(), blobContent.end());
      Azure::Storage::Blobs::UploadBlockBlobFromOptions upOptions;
      upOptions.Metadata = metadata;
      Azure::Storage::Blobs::Models::UploadBlockBlobFromResult result
          = blobClient.UploadFrom(buffer.data(), buffer.size(), upOptions, context).Value;
      returnValue = std::make_pair(result.LastModified, result.ETag);
    }
    else
    {
      throw;
    }
  }

  return returnValue;
}
