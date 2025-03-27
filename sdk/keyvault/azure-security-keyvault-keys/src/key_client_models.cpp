// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "./generated/keys_models.hpp"
#include "./private/key_serializers.hpp"
#include <azure/keyvault/keys/key_client_models.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;

KeyVaultKey::KeyVaultKey(_detail::Models::KeyBundle const& response)
{
  if (response.Key.HasValue())
  {
    if (response.Key.Value().Crv.HasValue())
    {
      Key.CurveName = KeyCurveName(response.Key.Value().Crv.Value().ToString());
    }
    if (response.Key.Value().Kty.HasValue())
    {
      Key.KeyType = KeyVaultKeyType(response.Key.Value().Kty.Value().ToString());
    }
    if (response.Key.Value().D.HasValue())
    {
      Key.D = response.Key.Value().D.Value();
    }
    if (response.Key.Value().Dp.HasValue())
    {
      Key.DP = response.Key.Value().Dp.Value();
    }
    if (response.Key.Value().Dq.HasValue())
    {
      Key.DQ = response.Key.Value().Dq.Value();
    }
    if (response.Key.Value().E.HasValue())
    {
      Key.E = response.Key.Value().E.Value();
    }
    if (response.Key.Value().N.HasValue())
    {
      Key.N = response.Key.Value().N.Value();
    }
    if (response.Key.Value().P.HasValue())
    {
      Key.P = response.Key.Value().P.Value();
    }
    if (response.Key.Value().Q.HasValue())
    {
      Key.Q = response.Key.Value().Q.Value();
    }
    if (response.Key.Value().Qi.HasValue())
    {
      Key.QI = response.Key.Value().Qi.Value();
    }
    if (response.Key.Value().T.HasValue())
    {
      Key.T = response.Key.Value().T.Value();
    }
    if (response.Key.Value().K.HasValue())
    {
      Key.K = response.Key.Value().K.Value();
    }
    if (response.Key.Value().KeyOps.HasValue())
    {
      std::vector<KeyOperation> keyOperations;
      for (auto keyOp : response.Key.Value().KeyOps.Value())
      {
        keyOperations.emplace_back(KeyOperation(keyOp));
      }
      Key.SetKeyOperations(keyOperations);
    }
    if (response.Key.Value().Kid.HasValue())
    {
      Key.Id = response.Key.Value().Kid.Value();
      Properties.Id = Key.Id;
      KeyVaultKeySerializer::ParseKeyUrl(Properties, Key.Id);
    }
    if (response.Key.Value().X.HasValue())
    {
      Key.X = response.Key.Value().X.Value();
    }
    if (response.Key.Value().K.HasValue())
    {
      Key.Y = response.Key.Value().Y.Value();
    }
  }

  if (response.Attributes.HasValue())
  {
    Properties.ExpiresOn = response.Attributes.Value().Expires;
    Properties.CreatedOn = response.Attributes.Value().Created;
    Properties.Enabled = response.Attributes.Value().Enabled;
    Properties.NotBefore = response.Attributes.Value().NotBefore;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      Properties.RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    Properties.Exportable = response.Attributes.Value().Exportable;
    Properties.HsmPlatform = response.Attributes.Value().HsmPlatform;
    Properties.RecoverableDays = response.Attributes.Value().RecoverableDays;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      Properties.RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    Properties.UpdatedOn = response.Attributes.Value().Updated;
    Properties.HsmPlatform = response.Attributes.Value().HsmPlatform;
    if (response.Attributes.Value().Attestation.HasValue())
    {
      Azure::Security::KeyVault::Keys::KeyAttestation attestation;
      attestation.CertificatePemFile
          = response.Attributes.Value().Attestation.Value().CertificatePemFile;
      attestation.PrivateKeyAttestation
          = response.Attributes.Value().Attestation.Value().PrivateKeyAttestation;
      attestation.PublicKeyAttestation
          = response.Attributes.Value().Attestation.Value().PublicKeyAttestation;
      attestation.Version = response.Attributes.Value().Attestation.Value().Version;
      Properties.Attestation = attestation;
    }
  }
  
  if (response.Managed.HasValue())
  {
    Properties.Managed = (response.Managed.ValueOr(false) == true);
  }

  if (response.Tags.HasValue())
  {
    for (auto const& tag : response.Tags.Value())
    {
      Properties.Tags.emplace(tag.first, tag.second);
    }
  }
  
  if (response.ReleasePolicy.HasValue())
  {
    KeyReleasePolicy policy;
    if (response.ReleasePolicy.Value().ContentType.HasValue())
    {
      policy.ContentType = response.ReleasePolicy.Value().ContentType.Value();
    }
    if (response.ReleasePolicy.Value().Immutable.HasValue())
    {
      policy.Immutable = response.ReleasePolicy.Value().Immutable.Value();
    }
    if (response.ReleasePolicy.Value().EncodedPolicy.HasValue())
    {
      auto encodedPolicy = response.ReleasePolicy.Value().EncodedPolicy.Value();
      policy.EncodedPolicy = std::string{encodedPolicy.begin(), encodedPolicy.end()};
    }
    Properties.ReleasePolicy = policy;
  }

}

DeletedKey::DeletedKey(_detail::Models::DeletedKeyBundle const& response)
{
  if (response.Key.HasValue())
  {
    if (response.Key.Value().Crv.HasValue())
    {
      Key.CurveName = KeyCurveName(response.Key.Value().Crv.Value().ToString());
    }
    if (response.Key.Value().Kty.HasValue())
    {
      Key.KeyType = KeyVaultKeyType(response.Key.Value().Kty.Value().ToString());
    }
    if (response.Key.Value().D.HasValue())
    {
      Key.D = response.Key.Value().D.Value();
    }
    if (response.Key.Value().Dp.HasValue())
    {
      Key.DP = response.Key.Value().Dp.Value();
    }
    if (response.Key.Value().Dq.HasValue())
    {
      Key.DQ = response.Key.Value().Dq.Value();
    }
    if (response.Key.Value().E.HasValue())
    {
      Key.E = response.Key.Value().E.Value();
    }
    if (response.Key.Value().N.HasValue())
    {
      Key.N = response.Key.Value().N.Value();
    }
    if (response.Key.Value().P.HasValue())
    {
      Key.P = response.Key.Value().P.Value();
    }
    if (response.Key.Value().Q.HasValue())
    {
      Key.Q = response.Key.Value().Q.Value();
    }
    if (response.Key.Value().Qi.HasValue())
    {
      Key.QI = response.Key.Value().Qi.Value();
    }
    if (response.Key.Value().T.HasValue())
    {
      Key.T = response.Key.Value().T.Value();
    }
    if (response.Key.Value().K.HasValue())
    {
      Key.K = response.Key.Value().K.Value();
    }
    if (response.Key.Value().KeyOps.HasValue())
    {
      std::vector<KeyOperation> keyOperations;
      for (auto keyOp : response.Key.Value().KeyOps.Value())
      {
        keyOperations.emplace_back(KeyOperation(keyOp));
      }
      Key.SetKeyOperations(keyOperations);
    }
    if (response.Key.Value().Kid.HasValue())
    {
      Key.Id = response.Key.Value().Kid.Value();
      Properties.Id = Key.Id;
      KeyVaultKeySerializer::ParseKeyUrl(Properties, Key.Id);
    }
    if (response.Key.Value().X.HasValue())
    {
      Key.X = response.Key.Value().X.Value();
    }
    if (response.Key.Value().K.HasValue())
    {
      Key.Y = response.Key.Value().Y.Value();
    }
  }

  if (response.Attributes.HasValue())
  {
    Properties.ExpiresOn = response.Attributes.Value().Expires;
    Properties.CreatedOn = response.Attributes.Value().Created;
    Properties.Enabled = response.Attributes.Value().Enabled;
    Properties.NotBefore = response.Attributes.Value().NotBefore;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      Properties.RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    Properties.Exportable = response.Attributes.Value().Exportable;
    Properties.HsmPlatform = response.Attributes.Value().HsmPlatform;
    Properties.RecoverableDays = response.Attributes.Value().RecoverableDays;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      Properties.RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    Properties.UpdatedOn = response.Attributes.Value().Updated;
    if (response.Attributes.Value().Attestation.HasValue())
    {
      Azure::Security::KeyVault::Keys::KeyAttestation attestation;
      attestation.CertificatePemFile
          = response.Attributes.Value().Attestation.Value().CertificatePemFile;
      attestation.PrivateKeyAttestation
          = response.Attributes.Value().Attestation.Value().PrivateKeyAttestation;
      attestation.PublicKeyAttestation
          = response.Attributes.Value().Attestation.Value().PublicKeyAttestation;
      attestation.Version = response.Attributes.Value().Attestation.Value().Version;
      Properties.Attestation = attestation;
    }
  }

  if (response.Managed.HasValue())
  {
    Properties.Managed = (response.Managed.ValueOr(false) == true);
  }

  if (response.Tags.HasValue())
  {
    for (auto const& tag : response.Tags.Value())
    {
      Properties.Tags.emplace(tag.first, tag.second);
    }
  }

  if (response.ReleasePolicy.HasValue())
  {
    KeyReleasePolicy policy;
    if (response.ReleasePolicy.Value().ContentType.HasValue())
    {
      policy.ContentType = response.ReleasePolicy.Value().ContentType.Value();
    }
    if (response.ReleasePolicy.Value().Immutable.HasValue())
    {
      policy.Immutable = response.ReleasePolicy.Value().Immutable.Value();
    }
    if (response.ReleasePolicy.Value().EncodedPolicy.HasValue())
    {
      auto encodedPolicy = response.ReleasePolicy.Value().EncodedPolicy.Value();
      policy.EncodedPolicy = std::string{encodedPolicy.begin(), encodedPolicy.end()};
    }
    Properties.ReleasePolicy = policy;
  }
  if (response.RecoveryId.HasValue())
  {
    RecoveryId = response.RecoveryId.Value();
  }
  if (response.DeletedDate.HasValue())
  {
    DeletedDate = response.DeletedDate.Value();
  }
  if (response.ScheduledPurgeDate.HasValue())
  {
    ScheduledPurgeDate = response.ScheduledPurgeDate.Value();
  }
}

_detail::Models::KeyUpdateParameters KeyProperties::ToKeyUpdateParameters(
    Azure::Nullable<std::vector<KeyOperation>> const& keyOperations) const
{
  _detail::Models::KeyUpdateParameters kUP;
  if (Tags.size() > 0)
  {
    std::map<std::string, std::string> tags;
    for (auto const& tag : Tags)
    {
      tags.insert({tag.first, tag.second});
    }
    kUP.Tags = tags;
  }
  if (ReleasePolicy.HasValue())
  {
    _detail::Models::KeyReleasePolicy releasePolicy;
    if (ReleasePolicy.Value().ContentType.HasValue())
    {
      releasePolicy.ContentType = ReleasePolicy.Value().ContentType.Value();
    }
    if (ReleasePolicy.Value().EncodedPolicy.size() > 0)
    {
      releasePolicy.EncodedPolicy = std::vector<uint8_t>(
          ReleasePolicy.Value().EncodedPolicy.begin(), ReleasePolicy.Value().EncodedPolicy.end());
    }
    releasePolicy.Immutable = ReleasePolicy.Value().Immutable;
    kUP.ReleasePolicy = releasePolicy;
  }
  if (keyOperations.HasValue())
  {
    kUP.KeyOps = std::vector<_detail::Models::JsonWebKeyOperation>();
    for (auto const& operation : keyOperations.Value())
    {
    kUP.KeyOps.Value().push_back(_detail::Models::JsonWebKeyOperation(operation.ToString()));
    }
  }

  kUP.KeyAttributes = _detail::Models::KeyAttributes();
  if (Attestation.HasValue())
  {
    kUP.KeyAttributes.Value().Attestation = _detail::Models::KeyAttestation();
    kUP.KeyAttributes.Value().Attestation.Value().CertificatePemFile
        = Attestation.Value().CertificatePemFile;
    kUP.KeyAttributes.Value().Attestation.Value().PrivateKeyAttestation
        = Attestation.Value().PrivateKeyAttestation;
    kUP.KeyAttributes.Value().Attestation.Value().PublicKeyAttestation
        = Attestation.Value().PublicKeyAttestation;
    kUP.KeyAttributes.Value().Attestation.Value().Version = Attestation.Value().Version;
  }
  kUP.KeyAttributes.Value().Created = CreatedOn;
  kUP.KeyAttributes.Value().Enabled = Enabled;
  kUP.KeyAttributes.Value().Expires = ExpiresOn;
  kUP.KeyAttributes.Value().Exportable = Exportable;
  kUP.KeyAttributes.Value().HsmPlatform = HsmPlatform;
  kUP.KeyAttributes.Value().NotBefore = NotBefore;
  kUP.KeyAttributes.Value().RecoverableDays = RecoverableDays;
  if (RecoveryLevel.size() > 0)
  {
    kUP.KeyAttributes.Value().RecoveryLevel = _detail::Models::DeletionRecoveryLevel(RecoveryLevel);
  }
  kUP.KeyAttributes.Value().Updated = UpdatedOn;
  return kUP;
}

KeyRotationPolicy::KeyRotationPolicy(_detail::Models::KeyRotationPolicy const& krp) {
  if (krp.Id.HasValue())
  {
    Id = krp.Id.Value();
  }

  if (krp.LifetimeActions.HasValue())
  {
    for (auto const& action : krp.LifetimeActions.Value())
    {
      LifetimeActionsType la;
      if (action.Action.Value().Type.Value() == _detail::Models::KeyRotationPolicyAction::Rotate)
      {
        la.Action = LifetimeActionType::Rotate;
      }
      else if (
          action.Action.Value().Type.Value() == _detail::Models::KeyRotationPolicyAction::Notify)
      {
        la.Action = LifetimeActionType::Notify;
      }
      if (action.Trigger.HasValue())
      {
        la.Trigger.TimeAfterCreate = action.Trigger.Value().TimeAfterCreate;
        la.Trigger.TimeBeforeExpiry = action.Trigger.Value().TimeBeforeExpiry;
      }
      LifetimeActions.emplace_back(la);
    }
  }
  if (krp.Attributes.HasValue())
  {
    Attributes.ExpiryTime = krp.Attributes.Value().ExpiryTime;
    Attributes.Created = krp.Attributes.Value().Created;
    Attributes.Updated = krp.Attributes.Value().Updated;
  }
}

_detail::Models::KeyRotationPolicy KeyRotationPolicy::ToKeyRotationPolicy() const
{
  _detail::Models::KeyRotationPolicy krp;
  if (Id.size() > 0)
  {
    krp.Id = Id;
  }
  if (LifetimeActions.size() > 0)
  {
    krp.LifetimeActions = std::vector<_detail::Models::LifetimeActions>();
    for (auto const& action : LifetimeActions)
    {
      _detail::Models::LifetimeActions la;
      _detail::Models::LifetimeActionsType laType;
      if (action.Action == LifetimeActionType::Rotate)
      {
        laType.Type = _detail::Models::KeyRotationPolicyAction::Rotate;
      }
      else if (action.Action == LifetimeActionType::Notify)
      {
        laType.Type = _detail::Models::KeyRotationPolicyAction::Notify;
      }
      la.Action = laType;
      la.Trigger = _detail::Models::LifetimeActionsTrigger();
      if (action.Trigger.TimeAfterCreate.HasValue())
      {
        la.Trigger.Value().TimeAfterCreate = action.Trigger.TimeAfterCreate.Value();
      }
      if (action.Trigger.TimeBeforeExpiry.HasValue())
      {
        la.Trigger.Value().TimeBeforeExpiry = action.Trigger.TimeBeforeExpiry.Value();
      }
      krp.LifetimeActions.Value().emplace_back(la);
    }
  }

  krp.Attributes = _detail::Models::KeyRotationPolicyAttributes();
  krp.Attributes.Value().Created = Attributes.Created;
  krp.Attributes.Value().Updated = Attributes.Updated;
  krp.Attributes.Value().ExpiryTime = Attributes.ExpiryTime;
  return krp;
}

KeyProperties::KeyProperties(_detail::Models::KeyItem const& response) {
  if (response.Attributes.HasValue())
  {
    ExpiresOn = response.Attributes.Value().Expires;
    CreatedOn = response.Attributes.Value().Created;
    Enabled = response.Attributes.Value().Enabled;
    NotBefore = response.Attributes.Value().NotBefore;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    Exportable = response.Attributes.Value().Exportable;
    HsmPlatform = response.Attributes.Value().HsmPlatform;
    RecoverableDays = response.Attributes.Value().RecoverableDays;
    if (response.Attributes.Value().RecoveryLevel.HasValue())
    {
      RecoveryLevel = response.Attributes.Value().RecoveryLevel.Value().ToString();
    }
    UpdatedOn = response.Attributes.Value().Updated;
    HsmPlatform = response.Attributes.Value().HsmPlatform;
    if (response.Attributes.Value().Attestation.HasValue())
    {
      Azure::Security::KeyVault::Keys::KeyAttestation attestation;
      attestation.CertificatePemFile
          = response.Attributes.Value().Attestation.Value().CertificatePemFile;
      attestation.PrivateKeyAttestation
          = response.Attributes.Value().Attestation.Value().PrivateKeyAttestation;
      attestation.PublicKeyAttestation
          = response.Attributes.Value().Attestation.Value().PublicKeyAttestation;
      attestation.Version = response.Attributes.Value().Attestation.Value().Version;
      Attestation = attestation;
    }
  }

  if (response.Managed.HasValue())
  {
    Managed = (response.Managed.ValueOr(false) == true);
  }

  if (response.Tags.HasValue())
  {
    for (auto const& tag : response.Tags.Value())
    {
      Tags.emplace(tag.first, tag.second);
    }
  }
  if (response.Kid.HasValue())
  {
    Id = response.Kid.Value();
    KeyVaultKeySerializer::ParseKeyUrl(*this, Id);
  }
}
