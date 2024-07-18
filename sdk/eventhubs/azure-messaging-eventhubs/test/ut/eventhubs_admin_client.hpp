// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <azure/core/context.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/test/test_base.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  // Derived from Azure::Core::Test::TestBase to get access to GetTestCredential, which has logic to
  // add the pipeline credential when run in CI.
  class EventHubsManagement : Azure::Core::Test::TestBase {
  public:
    enum class EventHubsPricingTier
    {
      Premium,
      Standard,
      Basic
    };
    EventHubsManagement();

    ~EventHubsManagement() = default;

    struct NamespaceIdentity
    {
    };
    enum class SystemDataByType
    {
      User,
      Application,
      ManagedIdentity,
      Key,
    };
    struct NamespaceSystemData
    {
      std::string CreatedBy;
      SystemDataByType CreatedByType;
      std::string CreatedAt;
      std::string LastModifiedBy;
      SystemDataByType LastModifiedByType;
      std::string LastModifiedAt;
      static NamespaceSystemData Deserialize(Azure::Core::Json::_internal::json const& json);
    };
    struct NamespaceProperties
    {
      std::string MinimumTlsVersion;
      std::string ProvisioningState;
      std::string Status;
      std::string CreatedAt;
      std::string UpdatedAt;
      std::string ServiceBusEndpoint;
      std::string ClusterArmId;
      std::string MetricId;
      bool IsAutoInflateEnabled;
      std::string PublicNetworkAccess;
      std::int32_t MaximumThroughputUnits;
      bool KafkaEnabled;
      bool ZoneRedundant;
      // NamespaceEncryption Encryption; - SKIPPED.
      // PrivateEndpointConnections - SKIPPED.
      bool DisableLocalAuth;
      std::string AlternateName;
      static NamespaceProperties Deserialize(Azure::Core::Json::_internal::json const& json);
    };
    struct NamespaceSku
    {
      std::string PricingTier;
      std::string Name;
      std::int32_t Capacity;
      static NamespaceSku Deserialize(Azure::Core::Json::_internal::json const& json);
    };
    struct EventHubsNamespace
    {
      NamespaceSku Sku;
      NamespaceIdentity Identity;
      NamespaceSystemData SystemData;
      NamespaceProperties Properties;
      std::int32_t Capacity;
      std::string Name;
      std::string Location;
      std::map<std::string, std::string> Tags;
      std::string Id;
      std::string Type;
      static EventHubsNamespace Deserialize(Azure::Core::Json::_internal::json const& json);
    };

    class EventHubsCreateOrUpdateOperation : public Azure::Core::Operation<EventHubsNamespace> {
    public:
      EventHubsCreateOrUpdateOperation(
          std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
          Azure::Core::Json::_internal::json const& jsonCreateResult);

    private:
      void UpdateStatus();
      // Inherited via Operation
      std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
          Azure::Core::Context const& context) override;
      Response<EventHubsNamespace> PollUntilDoneInternal(
          std::chrono::milliseconds period,
          Azure::Core::Context& context) override;
      EventHubsNamespace Value() const override;
      std::string GetResumeToken() const override;

      EventHubsNamespace m_namespaceInfo;
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    };

    class EventHubsDeleteOperation : public Azure::Core::Operation<bool> {
    public:
      EventHubsDeleteOperation(
          std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
          std::string const& pollingLocation);

    private:
      // Inherited via Operation
      std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
          Azure::Core::Context const& context) override;
      Response<bool> PollUntilDoneInternal(
          std::chrono::milliseconds period,
          Azure::Core::Context& context) override;
      bool Value() const override;
      std::string GetResumeToken() const override;

      std::string m_pollingUrl;
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    };

    class Namespace;

    struct EventHubCreationOptions
    {
      std::string Namespace;
      std::string Name;
      std::string ResourceGroup;
      std::string SubscriptionId;
    };

    class EventHub {
      EventHub(
          EventHubCreationOptions const& options,
          std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
          : m_namespace{options.Namespace}, m_name(options.Name),
            m_resourceGroup(options.ResourceGroup), m_subscriptionId{options.SubscriptionId},
            m_pipeline{pipeline}
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
      std::string m_namespace;
      std::string m_name;
      std::string m_resourceGroup;
      std::string m_subscriptionId;
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
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

    struct ConsumerGroup
    {
      Azure::DateTime CreatedAt;
      Azure::DateTime UpdatedAt;
      std::string UserMetadata;
    };

    class Namespace {
      Namespace(
          std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
          std::string const& name,
          std::string const& resourceGroup,
          std::string const& subscriptionId,
          Azure::Core::Context const& context);

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

      ConsumerGroup CreateConsumerGroup(
          std::string const& eventHubName,
          std::string const& consumerGroupName,
          Azure::Core::Context const& context = {});

    private:
      std::string m_name;
      std::string m_resourceGroup;
      std::string m_subscriptionId;
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
      friend class EventHubsManagement;
    };

    EventHubsCreateOrUpdateOperation CreateNamespace(
        std::string const& namespaceName,
        EventHubsPricingTier pricingTier = EventHubsPricingTier::Standard,
        Azure::Core::Context const& context = {});

    EventHubsDeleteOperation DeleteNamespace(
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
    // Dummy test body to satisfy Azure::Core::Test::TestBase.
    void TestBody() override {}

    void SetUp() override { SetUpTestBase(AZURE_TEST_RECORDING_DIR); };
    std::string m_resourceGroup;
    std::string m_location;
    std::string m_subscriptionId;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
  };
}}}} // namespace Azure::Messaging::EventHubs::Test
