// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <azure/core/context.hpp>

#include <string>
#include <vector>

// Create an event hub:
//	az eventhubs eventhub create --resource-group $EVENTHUBS_RESOURCE_GROUP --namespace-name
//$EVENTHUBS_NAMESPACE --name $EVENTHUBS_NAME
// Create a consumer group:
//	az eventhubs eventhub consumer-group create --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --eventhub-name $EVENTHUBS_NAME --name
//$EVENTHUBS_CONSUMER_GROUP
// Get the connection string:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv
// Get the connection string with the event hub name:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv | sed "s/EntityPath=.*/EntityPath=$EVENTHUBS_NAME/"
// Get the connection string with the event hub name and consumer group:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv | sed
//"s/EntityPath=.*/EntityPath=$EVENTHUBS_NAME/;s/Endpoint=.*/Endpoint=$EVENTHUBS_NAMESPACE.servicebus.windows.net/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_CONSUMER_GROUP/"
// Get the connection string with the event hub name and consumer group and SAS key:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv | sed
//"s/EntityPath=.*/EntityPath=$EVENTHUBS_NAME/;s/Endpoint=.*/Endpoint=$EVENTHUBS_NAMESPACE.servicebus.windows.net/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_CONSUMER_GROUP/;s/SharedAccessKey=.*/SharedAccessKey=$EVENTHUBS_SAS_KEY/"
// Get the connection string with the event hub name and consumer group and SAS key and SAS key
// name:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv | sed
//"s/EntityPath=.*/EntityPath=$EVENTHUBS_NAME/;s/Endpoint=.*/Endpoint=$EVENTHUBS_NAMESPACE.servicebus.windows.net/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_CONSUMER_GROUP/;s/SharedAccessKey=.*/SharedAccessKey=$EVENTHUBS_SAS_KEY/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_SAS_KEY_NAME/"
// Get the connection string with the event hub name and consumer group and SAS key and SAS key name
// and endpoint:
//	az eventhubs namespace authorization-rule keys list --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --name RootManageSharedAccessKey --query
// primaryConnectionString --output tsv | sed
//"s/EntityPath=.*/EntityPath=$EVENTHUBS_NAME/;s/Endpoint=.*/Endpoint=$EVENTHUBS_NAMESPACE.servicebus.windows.net/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_CONSUMER_GROUP/;s/SharedAccessKey=.*/SharedAccessKey=$EVENTHUBS_SAS_KEY/;s/SharedAccessKeyName=.*/SharedAccessKeyName=$EVENTHUBS_SAS_KEY_NAME/;s/Endpoint=.*/Endpoint=$EVENTHUBS_ENDPOINT/"
// Delete a consumer group:
//	az eventhubs eventhub consumer-group delete --resource-group $EVENTHUBS_RESOURCE_GROUP
//--namespace-name $EVENTHUBS_NAMESPACE --eventhub-name $EVENTHUBS_NAME --name
//$EVENTHUBS_CONSUMER_GROUP
// Delete an event hub:
//	az eventhubs eventhub delete --resource-group $EVENTHUBS_RESOURCE_GROUP --namespace-name
//$EVENTHUBS_NAMESPACE --name $EVENTHUBS_NAME
// Delete a namespace:
//	az eventhubs namespace delete --resource-group $EVENTHUBS_RESOURCE_GROUP --name
//$EVENTHUBS_NAMESPACE
// Delete a namespace (force):
//	az eventhubs namespace delete --resource-group $EVENTHUBS_RESOURCE_GROUP --name
//$EVENTHUBS_NAMESPACE --force
// Delete a namespace (force) (yes):
//	az eventhubs namespace delete --resource-group $EVENTHUBS_RESOURCE_GROUP --name
//$EVENTHUBS_NAMESPACE --force --yes

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class EventHubsManagement {
  public:
    enum class EventHubsPricingTier
    {
      Premium,
      Standard,
      Basic
    };
    EventHubsManagement();

    ~EventHubsManagement() = default;

    class Namespace;
    class EventHub {
      EventHub(
          std::string const& name,
          std::string const& resourceGroup,
          std::string const& subscriptionId)
          : m_name(name), m_resourceGroup(resourceGroup), m_subscriptionId{subscriptionId}
      {
      }

    public:
      std::string const& Name() const { return m_name; }
      std::string const& ResourceGroup() const { return m_resourceGroup; }

      bool CreateConsumerGroup(
          std::string const& consumerGroupName,
          Azure::Core::Context const& context = {});
      bool DeleteConsumerGroup(
          std::string const& consumerGroupName,
          Azure::Core::Context const& context = {});
      bool DoesConsumerGroupExist(
          std::string const& consumerGroupName,
          Azure::Core::Context const& context = {});

    private:
      std::string m_name;
      std::string m_resourceGroup;
      std::string m_subscriptionId;
      friend class EventHubsManagement;
      friend class EventHubsManagement::Namespace;
    };

    struct CreateEventHubOptions
    {
      /// <summary>
      /// Blob naming convention for archive, e.g.
      /// {Namespace}/{EventHub}/{PartitionId}/{Year}/{Month}/{Day}/{Hour}/{Minute}/{Second}. Here
      /// all the parameters (Namespace,EventHub .. etc) are mandatory irrespective of order.
      /// </summary>
      std::string ArchiveNameFormat;
      std::string BlobContainerName;
      std::chrono::seconds CaptureInterval;
      std::uint32_t CaptureSizeLimit;
      std::string DestinationName; // Should be EventHubArchive.AzureBlockBlob.
      bool EnableCapture;
      bool EnableSystemAssignedIdentity;
      std::vector<std::string> UserAssignedIdentityIds;
      std::int8_t PartitionCount;
      std::int32_t RetentionPeriodInHours;
      bool SkipEmptyArchives;
      std::string Status; // One of Active, Disabled, SendDisabled.
      std::string StorageAccount;
      std::int32_t TombstoneRetentionTimeInHours;
    };

    class Namespace {
      Namespace(
          std::string const& name,
          std::string const& resourceGroup,
          std::string const& subscriptionId)
          : m_name(name), m_resourceGroup(resourceGroup), m_subscriptionId(subscriptionId)
      {
      }

    public:
      std::string const& Name() const { return m_name; }
      std::string const& ResourceGroup() const { return m_resourceGroup; }

      std::vector<std::string> ListEventHubs(Azure::Core::Context const& context = {});
      EventHub CreateEventHub(
          std::string const& eventHubName,
          CreateEventHubOptions const& options = {},
          Azure::Core::Context const& context = {});
      bool DeleteEventHub(
          std::string const& eventHubName,
          Azure::Core::Context const& context = {});
      bool DoesEventHubExist(
          std::string const& eventHubName,
          Azure::Core::Context const& context = {});

    private:
      std::string m_name;
      std::string m_resourceGroup;
      std::string m_subscriptionId;
      friend class EventHubsManagement;
    };

    Namespace CreateNamespace(
        std::string const& namespaceName,
        EventHubsPricingTier pricingTier = EventHubsPricingTier::Standard,
        Azure::Core::Context const& context = {});

    void DeleteNamespace(
        std::string const& namespaceName,
        bool force = false,
        Azure::Core::Context const& context = {});

    Namespace GetNamespace(
        std::string const& namespaceName,
        Azure::Core::Context const& context = {});

    std::vector<std::string> ListNamespaces(Azure::Core::Context const& context = {});

    bool DoesNamespaceExist(
        std::string const& namespaceName,
        Azure::Core::Context const& context = {});

  private:
    std::string m_resourceGroup;
    std::string m_location;
    std::string m_subscriptionId;
  };
}}}} // namespace Azure::Messaging::EventHubs::Test
