// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/blobs.hpp>
#include <vector>
#include <sstream>

namespace Azure { namespace Messaging { namespace EventHubs {
/**@brief  ListCheckpointsOptions contains optional parameters for the ListCheckpoints function
 */
struct ListCheckpointsOptions
{
  // For future expansion
};

/**@brief  ListOwnershipOptions contains optional parameters for the ListOwnership function
 */
struct ListOwnershipOptions
{
  // For future expansion
};

/**@brief  UpdateCheckpointOptions contains optional parameters for the UpdateCheckpoint function
 */
struct UpdateCheckpointOptions
{
  // For future expansion
};

/**@brief  ClaimOwnershipOptions contains optional parameters for the ClaimOwnership function
 */
struct ClaimOwnershipOptions
{
  // For future expansion
};
/**@brief  Ownership tracks which consumer owns a particular partition.
 */
struct Ownership
{
  std::string ConsumerGroup;
  std::string EventHubName;
  std::string FullyQualifiedNamespace;
  std::string PartitionID;
  std::string OwnerID; // the owner ID of the Processor
  Azure::Nullable<Azure::ETag>
      ETag; // the ETag, used when attempting to claim or update ownership of a partition.
  Azure::Nullable<Azure::DateTime>
      LastModifiedTime; // used when calculating if ownership has expired
};

/**@brief Checkpoint tracks the last successfully processed event in a partition.
 */
struct Checkpoint
{
  std::string ConsumerGroup;
  std::string EventHubName;
  std::string FullyQualifiedNamespace;
  std::string PartitionID;
  Azure::Nullable<int64_t> Offset; // the last successfully processed Offset.
  Azure::Nullable<int64_t> SequenceNumber; // the last successfully processed SequenceNumber.
};

/**@brief  CheckpointStore is used by multiple consumers to coordinate progress and ownership for
 * partitions.
 */
class CheckpointStore {
public:
  /**@brief ClaimOwnership attempts to claim ownership of the partitions in partitionOwnership and
   * returns the actual partitions that    were claimed.
   */
  virtual std::vector<Ownership> ClaimOwnership(
      std::vector<Ownership> partitionOwnership,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ClaimOwnershipOptions const& options = ClaimOwnershipOptions())
      = 0;

  /**@brief  ListCheckpoints lists all the available checkpoints.
   */
  virtual std::vector<Checkpoint> ListCheckpoints(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHubName,
      std::string const& consumerGroup,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ListCheckpointsOptions options = ListCheckpointsOptions())
      = 0;

  /**@brief  ListOwnership lists all ownerships.
   */
  virtual std::vector<Ownership> ListOwnership(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHubName,
      std::string const& consumerGroup,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ListOwnershipOptions options = ListOwnershipOptions())
      = 0;

  /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
   */
  virtual void UpdateCheckpoint(
      Checkpoint const& checkpoint,
      Azure::Core::Context ctx = Azure::Core::Context(),
      UpdateCheckpointOptions options = UpdateCheckpointOptions())
      = 0;
};

class BlobCheckpointStore : public CheckpointStore {

  std::string m_connectionString;
  std::string m_containerName;
  Azure::Storage::Blobs::BlobContainerClient m_containerClient;

  std::string GetOwnershipName(Ownership const& ownership) { 
      if (ownership.PartitionID.empty())
    {
        throw std::exception("missing ownership fields");
    }
    std::stringstream strstr;
    strstr << GetOwnershipPrefixName(ownership)
           << ownership.PartitionID;
    return strstr.str();
  }
  
  std::string GetOwnershipPrefixName(Ownership const& ownership) {
    if (ownership.FullyQualifiedNamespace.empty() || ownership.EventHubName.empty()
        || ownership.ConsumerGroup.empty() )
    {
        throw std::exception("missing ownership fields");
    }
    std::stringstream strstr;
    strstr << ownership.FullyQualifiedNamespace << "/" << ownership.EventHubName << "/"
           << ownership.ConsumerGroup << "/ownership/";

    return strstr.str();
  }

  std::string GetCheckpointBlobPrefixName(Checkpoint const& checkpoint)
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

  std::string GetCheckpointBlobName(Checkpoint const& checkpoint)
  {
    if (checkpoint.PartitionID.empty())
    {
        throw std::exception("missing checkpoint fields");
    }
    std::stringstream strstr;
    strstr << GetCheckpointBlobPrefixName(checkpoint) << checkpoint.PartitionID;
    return strstr.str();
  }


  void updateCheckpoint(Azure::Storage::Metadata const& metadata, Checkpoint& checkpoint ) {
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

  void updateOwnership(Azure::Storage::Blobs::Models::BlobItem const& blob, Ownership& ownership) 
  {
    std::string temp = blob.Details.Metadata.at("ownerid");
    if (temp.empty())
    {
        throw std::exception("missing sequence number");
    }
    ownership.OwnerID= temp;
    ownership.LastModifiedTime = blob.Details.LastModified;
    ownership.ETag = blob.Details.ETag;
  }

  Azure::Storage::Metadata newCheckpointBlobMetadata(Checkpoint const& checkpoint) {
    Azure::Storage::Metadata metadata;

    if (checkpoint.SequenceNumber.HasValue())
    {
        metadata["sequencenumber"]= std::to_string(checkpoint.SequenceNumber.Value());
    }

    if (checkpoint.Offset.HasValue())
    {
        metadata["offset"]= std::to_string(checkpoint.Offset.Value());
    }
    return metadata;
  }

public:
  BlobCheckpointStore(
      std::string const& connectionString,
      std::string const& containerName)
      : m_connectionString(connectionString), m_containerName(containerName),
        m_containerClient(Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            connectionString,
            containerName))
  {
    m_containerClient.CreateIfNotExists();
  }

  std::vector<Ownership> ClaimOwnership(
      std::vector<Ownership> partitionOwnership,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ClaimOwnershipOptions const& options = ClaimOwnershipOptions())
  {
    (void)options;
    std::vector<Ownership> newOwnerships;

    for (Ownership ownership : partitionOwnership)
    {
        std::string blobName = GetOwnershipName(ownership);
        Azure::Storage::Metadata metadata;
        metadata["ownerId"] = ownership.OwnerID;
        try
        {
          std::pair<Azure::DateTime,Azure::ETag> result
              = setMetadata(blobName, metadata, ownership.ETag.ValueOr(Azure::ETag()), ctx);
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

  std::vector<Checkpoint> ListCheckpoints(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHubName,
      std::string const& consumerGroup,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ListCheckpointsOptions options = ListCheckpointsOptions())
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
          std::string partitionId = blob.Name.substr(blob.Name.rfind('/')+1);
          Checkpoint c
              = Checkpoint{consumerGroup, eventHubName, fullyQualifiedNamespace, partitionId};
          updateCheckpoint(blob.Details.Metadata, c);
          checkpoints.push_back(c);
        }
    }
    
    return checkpoints;
  }

  /**@brief  ListOwnership lists all ownerships.
   */
  std::vector<Ownership> ListOwnership(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHubName,
      std::string const& consumerGroup,
      Azure::Core::Context ctx = Azure::Core::Context(),
      ListOwnershipOptions options = ListOwnershipOptions())
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
          std::string partitionId = blob.Name.substr(blob.Name.rfind('/')+1);
          Ownership o{consumerGroup, eventHubName, fullyQualifiedNamespace, partitionId};
          updateOwnership(blob, o);
          ownerships.push_back(o);
        }
    }

    return ownerships;
  }

  /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
   */
  void UpdateCheckpoint(
      Checkpoint const& checkpoint,
      Azure::Core::Context ctx = Azure::Core::Context(),
      UpdateCheckpointOptions options = UpdateCheckpointOptions())
  {
    (void)options;
    std::string blobName = GetCheckpointBlobName(checkpoint);
    setMetadata(blobName, newCheckpointBlobMetadata(checkpoint), Azure::ETag(), ctx);
  }

  std::pair<Azure::DateTime, Azure::ETag> setMetadata(
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
};
}}} // namespace Azure::Messaging::EventHubs