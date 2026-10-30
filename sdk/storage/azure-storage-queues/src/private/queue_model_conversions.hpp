// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../generated/queues_client.hpp"
#include "azure/storage/queues/queue_responses.hpp"

namespace Azure { namespace Storage { namespace Queues { namespace _detail {

  namespace PublicModels = Azure::Storage::Queues::Models;

  inline Models::RetentionPolicy ToGenerated(PublicModels::RetentionPolicy value)
  {
    Models::RetentionPolicy result;
    result.Enabled = value.IsEnabled;
    result.Days = value.Days;
    return result;
  }

  inline PublicModels::RetentionPolicy ToPublic(Models::RetentionPolicy value)
  {
    PublicModels::RetentionPolicy result;
    result.IsEnabled = value.Enabled;
    result.Days = value.Days;
    return result;
  }

  inline Models::Metrics ToGenerated(PublicModels::Metrics value)
  {
    Models::Metrics result;
    result.Version = std::move(value.Version);
    result.Enabled = value.IsEnabled;
    result.IncludeApis = value.IncludeApis;
    result.RetentionPolicy = ToGenerated(std::move(value.RetentionPolicy));
    return result;
  }

  inline PublicModels::Metrics ToPublic(Models::Metrics value)
  {
    PublicModels::Metrics result;
    if (value.Version.HasValue())
    {
      result.Version = std::move(value.Version.Value());
    }
    result.IsEnabled = value.Enabled;
    result.IncludeApis = value.IncludeApis;
    if (value.RetentionPolicy.HasValue())
    {
      result.RetentionPolicy = ToPublic(std::move(value.RetentionPolicy.Value()));
    }
    return result;
  }

  inline Models::CorsRule ToGenerated(PublicModels::CorsRule value)
  {
    Models::CorsRule result;
    result.AllowedOrigins = std::move(value.AllowedOrigins);
    result.AllowedMethods = std::move(value.AllowedMethods);
    result.AllowedHeaders = std::move(value.AllowedHeaders);
    result.ExposedHeaders = std::move(value.ExposedHeaders);
    result.MaxAgeInSeconds = value.MaxAgeInSeconds;
    return result;
  }

  inline PublicModels::CorsRule ToPublic(Models::CorsRule value)
  {
    PublicModels::CorsRule result;
    result.AllowedOrigins = std::move(value.AllowedOrigins);
    result.AllowedMethods = std::move(value.AllowedMethods);
    result.AllowedHeaders = std::move(value.AllowedHeaders);
    result.ExposedHeaders = std::move(value.ExposedHeaders);
    result.MaxAgeInSeconds = value.MaxAgeInSeconds;
    return result;
  }

  inline Models::QueueServiceProperties ToGenerated(PublicModels::QueueServiceProperties value)
  {
    Models::QueueServiceProperties result;
    Models::Logging logging;
    logging.Version = std::move(value.Logging.Version);
    logging.Delete = value.Logging.Delete;
    logging.Read = value.Logging.Read;
    logging.Write = value.Logging.Write;
    logging.RetentionPolicy = ToGenerated(std::move(value.Logging.RetentionPolicy));
    result.Logging = std::move(logging);
    result.HourMetrics = ToGenerated(std::move(value.HourMetrics));
    result.MinuteMetrics = ToGenerated(std::move(value.MinuteMetrics));
    std::vector<Models::CorsRule> cors;
    cors.reserve(value.Cors.size());
    for (auto& rule : value.Cors)
    {
      cors.emplace_back(ToGenerated(std::move(rule)));
    }
    result.Cors = std::move(cors);
    return result;
  }

  inline PublicModels::QueueServiceProperties ToPublic(Models::GetPropertiesServiceResult value)
  {
    PublicModels::QueueServiceProperties result;
    if (value.Logging.HasValue())
    {
      auto& logging = value.Logging.Value();
      result.Logging.Version = std::move(logging.Version);
      result.Logging.Delete = logging.Delete;
      result.Logging.Read = logging.Read;
      result.Logging.Write = logging.Write;
      result.Logging.RetentionPolicy = ToPublic(std::move(logging.RetentionPolicy));
    }
    if (value.HourMetrics.HasValue())
    {
      result.HourMetrics = ToPublic(std::move(value.HourMetrics.Value()));
    }
    if (value.MinuteMetrics.HasValue())
    {
      result.MinuteMetrics = ToPublic(std::move(value.MinuteMetrics.Value()));
    }
    if (value.Cors.HasValue())
    {
      result.Cors.reserve(value.Cors.Value().size());
      for (auto& rule : value.Cors.Value())
      {
        result.Cors.emplace_back(ToPublic(std::move(rule)));
      }
    }
    return result;
  }

  inline Models::SignedIdentifiers ToGenerated(PublicModels::QueueAccessPolicy const& value)
  {
    Models::SignedIdentifiers result;
    result.Items.reserve(value.SignedIdentifiers.size());
    for (auto const& identifier : value.SignedIdentifiers)
    {
      Models::SignedIdentifier item;
      item.Id = identifier.Id;
      item.AccessPolicy.Start = identifier.StartsOn;
      item.AccessPolicy.Expiry = identifier.ExpiresOn;
      item.AccessPolicy.Permission = identifier.Permissions;
      result.Items.emplace_back(std::move(item));
    }
    return result;
  }

  inline PublicModels::QueueAccessPolicy ToPublic(Models::GetAccessPolicyQueueResult value)
  {
    PublicModels::QueueAccessPolicy result;
    result.SignedIdentifiers.reserve(value.Items.size());
    for (auto& identifier : value.Items)
    {
      PublicModels::SignedIdentifier item;
      item.Id = std::move(identifier.Id);
      item.StartsOn = identifier.AccessPolicy.Start;
      item.ExpiresOn = identifier.AccessPolicy.Expiry;
      if (identifier.AccessPolicy.Permission.HasValue())
      {
        item.Permissions = std::move(identifier.AccessPolicy.Permission.Value());
      }
      result.SignedIdentifiers.emplace_back(std::move(item));
    }
    return result;
  }

  inline PublicModels::QueueMessage ToPublic(Models::ReceivedMessage value)
  {
    PublicModels::QueueMessage result;
    result.MessageId = std::move(value.MessageId);
    result.InsertedOn = value.InsertionTime;
    result.ExpiresOn = value.ExpirationTime;
    result.PopReceipt = std::move(value.PopReceipt);
    result.NextVisibleOn = value.TimeNextVisible;
    result.DequeueCount = value.DequeueCount;
    result.MessageText = std::move(value.MessageText);
    return result;
  }

  inline PublicModels::PeekedQueueMessage ToPublic(Models::PeekedMessage value)
  {
    PublicModels::PeekedQueueMessage result;
    result.MessageId = std::move(value.MessageId);
    result.InsertedOn = value.InsertionTime;
    result.ExpiresOn = value.ExpirationTime;
    result.DequeueCount = value.DequeueCount;
    result.MessageText = std::move(value.MessageText);
    return result;
  }

}}}} // namespace Azure::Storage::Queues::_detail
