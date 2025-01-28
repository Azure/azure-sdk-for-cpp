// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_admin_client.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  EventHubsManagement::EventHubsManagement(
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential)
      : m_resourceGroup{Azure::Core::_internal::Environment::GetVariable(
          "EVENTHUBS_RESOURCE_GROUP")},
        m_location{Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_LOCATION")},
        m_subscriptionId{
            Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_SUBSCRIPTION_ID")}
  {

    Azure::Core::_internal::ClientOptions options;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;

    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes = {"https://management.azure.com/.default"};
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies;
    options.Telemetry.ApplicationId = "eventhubs.test";

    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        "eventhubs.test",
        "1.0.0",
        std::move(perRetryPolicies),
        std::move(perCallPolicies));
  }

  EventHubsManagement::EventHubsCreateOrUpdateOperation EventHubsManagement::CreateNamespace(
      std::string const& namespaceName,
      EventHubsPricingTier pricingTier,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + namespaceName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Json::_internal::json jsonBody;
    switch (pricingTier)
    {
      case EventHubsPricingTier::Basic:
        jsonBody["sku"] = Azure::Core::Json::_internal::json::object();
        jsonBody["sku"]["name"] = "Basic";
        jsonBody["sku"]["tier"] = "Basic";
        break;
      case EventHubsPricingTier::Standard:
        jsonBody["sku"] = Azure::Core::Json::_internal::json::object();
        jsonBody["sku"]["name"] = "Standard";
        jsonBody["sku"]["tier"] = "Standard";
        break;
      case EventHubsPricingTier::Premium:
        jsonBody["sku"] = Azure::Core::Json::_internal::json::object();
        jsonBody["sku"]["name"] = "Premium";
        jsonBody["sku"]["tier"] = "Premium";
        break;
    }
    jsonBody["properties"] = Azure::Core::Json::_internal::json::object();
    jsonBody["location"] = m_location;
    std::string bodyText = jsonBody.dump();

    Azure::Core::IO::MemoryBodyStream requestBody{
        reinterpret_cast<const uint8_t*>(bodyText.data()), bodyText.size()};
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Put, requestUrl, &requestBody);

    request.SetHeader("Accept", "application/json");
    request.SetHeader("Content-Type", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Created
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Accepted)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);
    if (jsonOutput.is_null())
    {
      throw std::runtime_error("JSON output is null!");
    }
    if (!jsonOutput.is_object())
    {
      throw std::runtime_error("JSON output is not an object!");
    }
    return EventHubsCreateOrUpdateOperation{m_pipeline, jsonOutput};
  }

  EventHubsManagement::EventHubsCreateOrUpdateOperation::EventHubsCreateOrUpdateOperation(
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
      Azure::Core::Json::_internal::json const& json)
      : m_namespaceInfo{EventHubsNamespace::Deserialize(json)}, m_pipeline{pipeline}
  {
    UpdateStatus();
  }

  void EventHubsManagement::EventHubsCreateOrUpdateOperation::UpdateStatus()
  {
    if (m_namespaceInfo.Properties.ProvisioningState == "Succeeded")
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Canceled")
    {
      m_status = Azure::Core::OperationStatus::Cancelled;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Creating")
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Created")
    {
      // The "Created" provisioning status does not mean that the namespace is ready. We need to
      // wait until the ProvisioningState is "Succeeded".
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Updating")
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Deleting")
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (m_namespaceInfo.Properties.ProvisioningState == "Failed")
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else
    {
      throw std::runtime_error(
          "Unknown provisioning state: " + m_namespaceInfo.Properties.ProvisioningState);
    }
  }

  std::unique_ptr<Azure::Core::Http::RawResponse>
  EventHubsManagement::EventHubsCreateOrUpdateOperation::PollInternal(
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl("https://management.azure.com/" + m_namespaceInfo.Id);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, requestUrl);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }
    //    auto& val = result->GetBody();
    //    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};

    m_namespaceInfo = EventHubsNamespace::Deserialize(
        Azure::Core::Json::_internal::json::parse(result->GetBody()));
    UpdateStatus();
    return result;
  }

  Response<EventHubsManagement::EventHubsNamespace>
  EventHubsManagement::EventHubsCreateOrUpdateOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      Poll(context);
      if (IsDone())
      {
        break;
      }
      std::this_thread::sleep_for(period);
    }

    return Azure::Response<EventHubsManagement::EventHubsNamespace>(
        Value(), std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
  }

  EventHubsManagement::EventHubsNamespace
  EventHubsManagement::EventHubsCreateOrUpdateOperation::Value() const
  {
    return m_namespaceInfo;
  }

  std::string EventHubsManagement::EventHubsCreateOrUpdateOperation::GetResumeToken() const
  {
    throw std::runtime_error("Not implemented!");
  }

  EventHubsManagement::EventHubsDeleteOperation EventHubsManagement::DeleteNamespace(
      std::string const& namespaceName,
      bool force,
      Azure::Core::Context const& context)
  {
    (void)force;
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + namespaceName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, requestUrl);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Created
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Accepted)
    {
      throw Azure::Core::RequestFailedException(result);
    }
    return EventHubsDeleteOperation{m_pipeline, result->GetHeaders().at("location")};
  }

  EventHubsManagement::EventHubsDeleteOperation::EventHubsDeleteOperation(
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
      std::string const& pollingUrl)
      : m_pollingUrl{pollingUrl}, m_pipeline{pipeline}
  {
  }

  std::unique_ptr<Azure::Core::Http::RawResponse>
  EventHubsManagement::EventHubsDeleteOperation::PollInternal(Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(m_pollingUrl);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, requestUrl);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::NoContent
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Accepted)
    {
      throw Azure::Core::RequestFailedException(result);
    }
    // Ok and No Content are terminal states for a delete operation.
    if (result->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok
        || result->GetStatusCode() == Azure::Core::Http::HttpStatusCode::NoContent)
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    return result;
  }

  Response<bool> EventHubsManagement::EventHubsDeleteOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      Poll(context);
      if (IsDone())
      {
        break;
      }
      std::this_thread::sleep_for(period);
    }

    return Azure::Response<bool>(
        Value(), std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
  }
  bool EventHubsManagement::EventHubsDeleteOperation::Value() const { return true; }
  std::string Test::EventHubsManagement::EventHubsDeleteOperation::GetResumeToken() const
  {
    throw std::runtime_error("Not implemented!");
  }

  std::vector<std::string> EventHubsManagement::ListNamespaces(Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/providers/Microsoft.EventHub/namespaces");
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, requestUrl);
    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);
    if (jsonOutput.is_null())
    {
      throw std::runtime_error("JSON output is null!");
    }
    if (!jsonOutput.is_object())
    {
      throw std::runtime_error("JSON output is not an object!");
    }
    if (!jsonOutput.contains("value"))
    {
      throw std::runtime_error("JSON output is missing required 'value'!");
    }
    jsonOutput = jsonOutput["value"];

    if (!jsonOutput.is_array())
    {
      throw std::runtime_error("JSON output is not an array!");
    }
    std::vector<std::string> returnValue;
    for (auto const& it : jsonOutput)
    {
      if (!it.is_object())
      {
        throw std::runtime_error("Item is not an object: " + it.dump());
      }
      returnValue.push_back(it["name"].get<std::string>());
    }
    return returnValue;
  }
  bool EventHubsManagement::DoesNamespaceExist(
      std::string const& namespaceName,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/providers/Microsoft.EventHub/checkNameAvailability");
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    std::string bodyText = "{\"name\":\"" + namespaceName + "\"}";
    Azure::Core::IO::MemoryBodyStream requestBody{
        reinterpret_cast<const uint8_t*>(bodyText.data()), bodyText.size()};
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, requestUrl, &requestBody);
    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);
    if (jsonOutput.is_null())
    {
      throw std::runtime_error("JSON output is null!");
    }
    if (!jsonOutput.is_object())
    {
      throw std::runtime_error("JSON output is not an object!");
    }

    return !jsonOutput["nameAvailable"].get<bool>();
  }

  EventHubsManagement::Namespace EventHubsManagement::GetNamespace(
      std::string const& namespaceName,
      Azure::Core::Context const& context)
  {
    if (namespaceName.empty())
    {
      throw std::runtime_error("Namespace name cannot be empty!");
    }
    if (DoesNamespaceExist(namespaceName, context))
    {
      return Namespace(m_pipeline, namespaceName, m_resourceGroup, m_subscriptionId, context);
    }
    else
    {
      throw std::runtime_error("Namespace does not exist!");
    }
  }

  EventHubsManagement::Namespace::Namespace(
      std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline,
      std::string const& name,
      std::string const& resourceGroup,
      std::string const& subscriptionId,
      Azure::Core::Context const& context)
      : m_name(name), m_resourceGroup(resourceGroup),
        m_subscriptionId(subscriptionId), m_pipeline{pipeline}
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + name);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, requestUrl);
    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);
    if (jsonOutput.is_null())
    {
      throw std::runtime_error("JSON output is null!");
    }
    if (!jsonOutput.is_object())
    {
      throw std::runtime_error("JSON output is not an object!");
    }
  }

  std::vector<std::string> EventHubsManagement::Namespace::ListEventHubs(
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + m_name + "/eventhubs");
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, requestUrl);
    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);
    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);
    if (jsonOutput.is_null())
    {
      throw std::runtime_error("JSON output is null!");
    }
    if (!jsonOutput.is_object())
    {
      throw std::runtime_error("JSON output is not an object!");
    }
    if (!jsonOutput.contains("value"))
    {
      throw std::runtime_error("JSON output is missing required 'value'!");
    }
    jsonOutput = jsonOutput["value"];
    if (!jsonOutput.is_array())
    {
      throw std::runtime_error("JSON output is not an array!");
    }
    std::vector<std::string> returnValue;
    for (auto const& it : jsonOutput)
    {
      if (!it.is_object())
      {
        throw std::runtime_error("Item is not an object: " + it.dump());
      }
      returnValue.push_back(it["name"].get<std::string>());
    }
    return returnValue;
  }

  EventHubsManagement::EventHub EventHubsManagement::Namespace::CreateEventHub(
      std::string const& eventHubName,
      EventHubsManagement::CreateEventHubOptions const& eventHubsOptions,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + m_name + "/eventhubs/" + eventHubName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Json::_internal::json jsonBody;
    jsonBody["properties"] = Azure::Core::Json::_internal::json::object();
    (void)eventHubsOptions;

    std::string bodyText = jsonBody.dump();

    Azure::Core::IO::MemoryBodyStream requestBody{
        reinterpret_cast<const uint8_t*>(bodyText.data()), bodyText.size()};
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Put, requestUrl, &requestBody);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);

    return EventHub(
        EventHubCreationOptions{m_name, eventHubName, m_resourceGroup, m_subscriptionId},
        m_pipeline);
  }

  bool EventHubsManagement::Namespace::DeleteEventHub(
      std::string const& eventHubName,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + m_name + "/eventhubs/" + eventHubName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, requestUrl);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok
        && result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::NoContent)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    // There is no expected body on a delete eventhub response.

    return true;
  }

  bool EventHubsManagement::EventHub::CreateConsumerGroup(
      std::string const& consumerGroupName,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + m_namespace + "/eventhubs/" + m_name
        + "/consumergroups/" + consumerGroupName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Json::_internal::json jsonBody;
    jsonBody["properties"] = Azure::Core::Json::_internal::json::object();

    std::string bodyText = jsonBody.dump();

    Azure::Core::IO::MemoryBodyStream requestBody{
        reinterpret_cast<const uint8_t*>(bodyText.data()), bodyText.size()};
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Put, requestUrl, &requestBody);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    Azure::Core::Json::_internal::json jsonOutput
        = Azure::Core::Json::_internal::json::parse(bodyAsString);

    return true;
  }

  bool EventHubsManagement::EventHub::DeleteConsumerGroup(
      std::string const& consumerGroupName,
      Azure::Core::Context const& context)
  {
    Azure::Core::Url requestUrl(
        "https://management.azure.com/subscriptions/" + Azure::Core::Url::Encode(m_subscriptionId)
        + "/resourceGroups/" + Azure::Core::Url::Encode(m_resourceGroup)
        + "/providers/Microsoft.EventHub/namespaces/" + m_namespace + "/eventhubs/" + m_name
        + "/consumergroups/" + consumerGroupName);
    requestUrl.AppendQueryParameter("api-version", "2022-10-01-preview");

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Delete, requestUrl);

    request.SetHeader("Accept", "application/json");
    auto result = m_pipeline->Send(request, context);

    auto& val = result->GetBody();
    std::string bodyAsString{reinterpret_cast<const char*>(val.data()), val.size()};
    if (result->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Ok)
    {
      throw Azure::Core::RequestFailedException(result);
    }

    return true;
  }

  EventHubsManagement::NamespaceProperties EventHubsManagement::NamespaceProperties::Deserialize(
      Azure::Core::Json::_internal::json const& json)
  {
    NamespaceProperties rv;
    rv.MinimumTlsVersion = json["minimumTlsVersion"];
    rv.ProvisioningState = json["provisioningState"];
    rv.Status = json["status"];
    rv.CreatedAt = json["createdAt"];
    rv.UpdatedAt = json["updatedAt"];
    rv.ServiceBusEndpoint = json["serviceBusEndpoint"];
    if (json.contains("clusterArmId"))
    {
      rv.ClusterArmId = json["clusterArmId"];
    }
    rv.MetricId = json["metricId"];
    rv.IsAutoInflateEnabled = json["isAutoInflateEnabled"];
    rv.PublicNetworkAccess = json["publicNetworkAccess"];
    rv.MaximumThroughputUnits = json["maximumThroughputUnits"];
    rv.KafkaEnabled = json["kafkaEnabled"];
    rv.ZoneRedundant = json["zoneRedundant"];
    rv.DisableLocalAuth = json["disableLocalAuth"];
    if (json.contains("alternateName"))
    {
      rv.AlternateName = json["alternateName"];
    }
    // Skipped: encryption
    return rv;
  }
  EventHubsManagement::NamespaceSku EventHubsManagement::NamespaceSku::Deserialize(
      Azure::Core::Json::_internal::json const& json)
  {
    NamespaceSku rv;
    rv.Name = json["name"];
    rv.PricingTier = json["tier"];
    rv.Capacity = json["capacity"];
    return rv;
  }

  EventHubsManagement::EventHubsNamespace EventHubsManagement::EventHubsNamespace::Deserialize(
      Azure::Core::Json::_internal::json const& json)
  {
    EventHubsNamespace rv;
    rv.Name = json["name"];
    rv.Sku = NamespaceSku::Deserialize(json["sku"]);
    rv.Location = json["location"];
    rv.Properties = NamespaceProperties::Deserialize(json["properties"]);
    for (const auto& tag : json["tags"])
    {
      (void)tag;
    }
    rv.Id = json["id"];
    rv.Type = json["type"];
    return rv;
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
