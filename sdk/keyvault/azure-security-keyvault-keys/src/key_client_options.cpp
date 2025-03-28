// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "./generated/keys_models.hpp"

#include <azure/keyvault/keys/key_client_options.hpp>
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
_detail::Models::KeyCreateParameters CreateKeyOptions::ToKeyCreateParameters() const
{
  Models::KeyCreateParameters keyCreateParameters;
  Models::KeyAttributes attributes;
  std::vector<Models::JsonWebKeyOperation> operations;
  for (auto operation : KeyOperations)
  {
    operations.push_back(Models::JsonWebKeyOperation(operation.ToString()));
  }
  keyCreateParameters.KeyOps = operations;
  std::map<std::string, std::string> tags;
  for (auto const& tag : Tags)
  {
    tags.insert({tag.first, tag.second});
  }
  keyCreateParameters.Tags = tags;
  attributes.NotBefore = NotBefore;
  attributes.Expires = ExpiresOn;
  attributes.Enabled = Enabled;
  attributes.Exportable = Exportable;
  if (ReleasePolicy.HasValue())
  {
    Models::KeyReleasePolicy releasePolicy;
    if (ReleasePolicy.Value().ContentType.HasValue())
    {
      releasePolicy.ContentType = ReleasePolicy.Value().ContentType.Value();
    }

    releasePolicy.EncodedPolicy = std::vector<uint8_t>(
        ReleasePolicy.Value().EncodedPolicy.begin(), ReleasePolicy.Value().EncodedPolicy.end());
    releasePolicy.Immutable = ReleasePolicy.Value().Immutable;
    keyCreateParameters.ReleasePolicy = releasePolicy;
  }
  keyCreateParameters.KeyAttributes = attributes;
  return keyCreateParameters;
}

_detail::Models::KeyImportParameters ImportKeyOptions::ToKeyImportParameters() const
{
  _detail::Models::KeyImportParameters kIP;
  kIP.Hsm = HardwareProtected;
  kIP.Key = _detail::Models::JsonWebKey();
  if (Key.CurveName.HasValue())
  {
    kIP.Key.Crv = _detail::Models::JsonWebKeyCurveName(Key.CurveName.Value().ToString());
  }
  kIP.Key.D = Key.D;
  kIP.Key.Dp = Key.DP;
  kIP.Key.Dq = Key.DQ;
  kIP.Key.E = Key.E;
  kIP.Key.K = Key.K;
  auto operations = Key.KeyOperations();
  if (operations.size() > 0)
  {
    kIP.Key.KeyOps = std::vector<std::string>();
    for (auto op : Key.KeyOperations())
    {
      kIP.Key.KeyOps.Value().push_back(op.ToString());
    }
  }
  kIP.Key.Kid = Key.Id;
  kIP.Key.Kty = _detail::Models::JsonWebKeyType(Key.KeyType.ToString());
  kIP.Key.N = Key.N;
  kIP.Key.P = Key.P;
  kIP.Key.Q = Key.Q;
  kIP.Key.Qi = Key.QI;
  kIP.Key.T = Key.T;
  kIP.Key.X = Key.X;
  kIP.Key.Y = Key.Y;
  auto attributes = _detail::Models::KeyAttributes();

  if (Properties.Attestation.HasValue())
  {
    attributes.Attestation.Value().CertificatePemFile
        = Properties.Attestation.Value().CertificatePemFile;
    attributes.Attestation.Value().PrivateKeyAttestation
        = Properties.Attestation.Value().PrivateKeyAttestation;
    attributes.Attestation.Value().PublicKeyAttestation
        = Properties.Attestation.Value().PublicKeyAttestation;
    attributes.Attestation.Value().Version = Properties.Attestation.Value().Version;
  };
  attributes.Expires = Properties.ExpiresOn;
  attributes.Enabled = Properties.Enabled;
  attributes.Created = Properties.CreatedOn;
  attributes.Exportable = Properties.Exportable;
  attributes.HsmPlatform = Properties.HsmPlatform;
  attributes.NotBefore = Properties.NotBefore;
  attributes.RecoverableDays = Properties.RecoverableDays;
  attributes.Updated = Properties.UpdatedOn;
  attributes.RecoveryLevel = _detail::Models::DeletionRecoveryLevel(Properties.RecoveryLevel);
  kIP.KeyAttributes = attributes;

  return kIP;
}

_detail::Models::KeyReleaseParameters KeyReleaseOptions::ToKeyReleaseParameters() const
{
  _detail::Models::KeyReleaseParameters krp = _detail::Models::KeyReleaseParameters();
  if (Encryption.HasValue())
  {
    if (Encryption.Value() == KeyEncryptionAlgorithm::CkmRsaAesKeyWrap)
    {
      krp.Enc = _detail::Models::KeyEncryptionAlgorithm::KeyEncryptionAlgorithm::CkmRsaAesKeyWrap;
    }
    else if (Encryption.Value() == KeyEncryptionAlgorithm::RsaAesKeyWrap256)
    {
      krp.Enc = _detail::Models::KeyEncryptionAlgorithm::KeyEncryptionAlgorithm::
          RsaAesKeyWrapTwoHundredFiftySix;
    }
    if (Encryption.Value() == KeyEncryptionAlgorithm::RsaAesKeyWrap384)
    {
      krp.Enc = _detail::Models::KeyEncryptionAlgorithm::KeyEncryptionAlgorithm::
          RsaAesKeyWrapThreeHundredEightyFour;
    }
  }

  krp.Nonce = Nonce;
  krp.TargetAttestationToken = Target;
  return krp;
}
