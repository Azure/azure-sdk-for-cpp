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
      if (response.Key.Value().Crv.Value() == Models::JsonWebKeyCurveName::PFiveHundredTwentyOne)
      {
        Key.CurveName = KeyCurveName::P256;
      }
      else if (
          response.Key.Value().Crv.Value() == Models::JsonWebKeyCurveName::PThreeHundredEightyFour)
      {
        Key.CurveName = KeyCurveName::P384;
      }
      else if (response.Key.Value().Crv.Value() == Models::JsonWebKeyCurveName::PTwoHundredFiftySix)
      {
        Key.CurveName = KeyCurveName::P256;
      }
      else if (response.Key.Value().Crv.Value() == Models::JsonWebKeyCurveName::P256k)
      {
        Key.CurveName = KeyCurveName::P256K;
      }
      else
      {
        throw std::runtime_error("Unsupported curve name");
      }
    }
    if (response.Key.Value().Kty.HasValue())
    {
      if (response.Key.Value().Kty.Value() == Models::JsonWebKeyType::Rsa)
      {
        Key.KeyType = KeyVaultKeyType::Rsa;
      }
      else if (response.Key.Value().Kty.Value() == Models::JsonWebKeyType::RsaHsm)
      {
        Key.KeyType = KeyVaultKeyType::RsaHsm;
      }
      else if (response.Key.Value().Kty.Value() == Models::JsonWebKeyType::EC)
      {
        Key.KeyType = KeyVaultKeyType::Ec;
      }
      else if (response.Key.Value().Kty.Value() == Models::JsonWebKeyType::ECHsm)
      {
        Key.KeyType = KeyVaultKeyType::EcHsm;
      }
      else
      {
        throw std::runtime_error("Unsupported key type");
      }
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
      for (auto keyOp : response.Key.Value().KeyOps.Value())
      {
        if (keyOp == Models::JsonWebKeyOperation::Encrypt.ToString())
        {
          Key.SetKeyOperations({KeyOperation::Encrypt});
        }
        else if (keyOp == Models::JsonWebKeyOperation::Decrypt.ToString())
        {
          Key.SetKeyOperations({KeyOperation::Decrypt});
        }
        else if (keyOp == Models::JsonWebKeyOperation::Sign.ToString())
        {
          Key.SetKeyOperations({KeyOperation::Sign});
        }
        else if (keyOp == Models::JsonWebKeyOperation::Verify.ToString())
        {
          Key.SetKeyOperations({KeyOperation::Verify});
        }
        else if (keyOp == Models::JsonWebKeyOperation::WrapKey.ToString())
        {
          Key.SetKeyOperations({KeyOperation::WrapKey});
        }
        else if (keyOp == Models::JsonWebKeyOperation::UnwrapKey.ToString())
        {
          Key.SetKeyOperations({KeyOperation::UnwrapKey});
        }
      }
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
