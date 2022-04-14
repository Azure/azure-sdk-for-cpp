// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;

KeyRotationPolicy _detail::KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto body = rawResponse.GetBody();
  return KeyRotationPolicyDeserialize(body);
}

KeyRotationPolicy _detail::KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(
    std::vector<uint8_t> const& body)
{
  auto jsonParser = Azure::Core::Json::_internal::json::parse(body);
  KeyRotationPolicy policy;

  std::string goqu(body.begin(), body.end());

  policy.Id = jsonParser[_detail::IdValue].get<std::string>();

  if (!jsonParser[_detail::AttributesPropertyName].is_null())
  {
    auto jsonFragment = jsonParser[_detail::AttributesPropertyName];
    policy.Attributes.ExpiryTime = jsonFragment[_detail::ExpiryTimeValue].get<std::string>();

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        policy.Attributes.Created,
        jsonFragment,
        _detail::CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        policy.Attributes.Updated,
        jsonFragment,
        _detail::UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
  }

  if (!jsonParser[_detail::LifeTimeActionsValue].is_null())
  {
    auto lifeTimeActions = jsonParser[_detail::LifeTimeActionsValue];

    for (auto action : lifeTimeActions)
    {
      LifetimeActions currentAction;

      JsonOptional::SetIfExists(
          currentAction.Trigger.TimeAfterCreate,
          action[_detail::TriggerActionsValue],
          _detail::TACActionsValue);

      JsonOptional::SetIfExists(
          currentAction.Trigger.TimeBeforeExpiry,
          action[_detail::TriggerActionsValue],
          _detail::TBEActionsValue);

      auto actionType = action[_detail::ActionActionsValue][TypeActionsValue].get<std::string>();
      std::transform(actionType.begin(), actionType.end(), actionType.begin(), [](unsigned char c) {
        return std::tolower(c);
      });

      if (actionType == _detail::RotateActionsValue)
      {
        currentAction.Action = LifetimeActionType::Rotate;
      }
      else if (actionType == _detail::NotifyActionsValue)
      {
        currentAction.Action = LifetimeActionType::Notify;
      }

      policy.LifetimeActions.emplace_back(currentAction);
    }
  }

  return policy;
}

std::string _detail::KeyRotationPolicySerializer::KeyRotationPolicySerialize(
    KeyRotationPolicy const& rotationPolicy)
{
  json payload;

  JsonOptional::SetFromNullable(
      rotationPolicy.Attributes.ExpiryTime,
      payload[_detail::AttributesPropertyName],
      _detail::ExpiryTimeValue);
  payload[_detail::LifeTimeActionsValue].array();
  for (auto lifetimeAction : rotationPolicy.LifetimeActions)
  {
    json oneAction;

    JsonOptional::SetFromNullable(
        lifetimeAction.Trigger.TimeAfterCreate,
        oneAction[_detail::TriggerActionsValue],
        _detail::TACActionsValue);

    JsonOptional::SetFromNullable(
        lifetimeAction.Trigger.TimeBeforeExpiry,
        oneAction[_detail::TriggerActionsValue],
        _detail::TBEActionsValue);

    if (lifetimeAction.Action == LifetimeActionType::Notify)
    {
      oneAction[_detail::ActionActionsValue][_detail::TypeActionsValue]
          = _detail::NotifyActionsValue;
    }
    else
    {
      oneAction[_detail::ActionActionsValue][_detail::TypeActionsValue]
          = _detail::RotateActionsValue;
    }

    payload[_detail::LifeTimeActionsValue].emplace_back(oneAction);
  }

  return payload.dump();
};