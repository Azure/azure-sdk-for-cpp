// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "crypto_test_collateral.hpp"

namespace Azure { namespace Security { namespace Attestation { namespace Test {
  // Test ephemeral ECDS public and private keys for test purposes.

  constexpr char const* testEcdsPrivateKey(R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQg6g5VraUfx16neNxT
UodoJBPFB3WzspMM5icOPLnd9h6hRANCAATWmDp5WrcaZZQQZPhI2asPDYJFbnY5
MTfWq57zhkm3+wrn3ch6yUg6JZT+OwKhTf3i0FX+IWcPB1iFDQ2Vy3zh
-----END PRIVATE KEY-----
)");
  constexpr char const* testEcdsPublicKey(R"(-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1pg6eVq3GmWUEGT4SNmrDw2CRW52
OTE31que84ZJt/sK593IeslIOiWU/jsCoU394tBV/iFnDwdYhQ0Nlct84Q==
-----END PUBLIC KEY-----
)");

  constexpr char const* testRsaPrivateKey(R"(-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDOqtQaML1iiA2j
YmrePlsDjPOH429ZAXMQNPCWWqEkNY1T5HckA9dW81P43w8/k81tPuC2eLVBD/2o
zaRIZpWikbwsvN4uoQhrJDpuQyJ2V681fRWj8Dinqcd2Ccar8igwvE9UJMGDkX+q
Wti+BwA4dJ56LzxbQHYc39mVcmosfpu4ihifMj2U4bNA+e0s8ft/VEm2UG4hWgzK
NMrGvqkusAGvWs0/UPthb7HJEVZaHX5ebX2LgedUgSl88oe4P5tNR8XSbTvvp3oP
x4EkM63S4wp4A2JsfpNgYPT1oNRB+/VHjW+OZjKaYjoTqs++M7KnHaUn+qUwCuMY
meVI2cEZAgMBAAECggEALf1l5c4i1uJf5pPoJDzMFCYxq3O5O51O9bRSNaNFaMFi
CeE1ghY4vWi4rxE0W3mQpVbwuqpx62CbmgzpGhN6CQtVTL9a0hWTwgP13MOjz6ID
o2uKfUjf0q+9a08RnwHsX6wIGzlytsySFF7TDLaSHf4VpisMy2G05wgJa3BioDPG
IjFjJNR9jB+Ql9IYpnJh30m3sLAsmj2I2UvHbBtTmjE9/He0W4M4HvJZ9+Mm0BLZ
53apdBAne2axgxoESWy6kPZB/Fcvwn3zIKJQ2rwdN3mhZb/tVw7t5DV9ASmTOr6L
62qedFGSrAQoAjrB9oxFd+DLACVmJebdIWsQzcDeYQKBgQDo1Ii37UKOCtBp7SPr
Sa0d9ngqX7+w/AK95yylKOU/mDQA9v/vDVD4XmQI9ZOYzA/xM09SWm7HTmbJDK7U
EqLrtdx6j2itjf4M3X+zYCxbxsT93D7Sq/zQUnnNcUGuo98CjayMEVWDRDF+CZTG
X0W3OZUAU9D1SvJ8hNhulK2WzwKBgQDjO8f9Ta1crRWyhMxPQQWG3H3pyvuC92I1
EZKYoQfmMN64cKMIjE12062Oht48udZUHTSe/zHRCn0C4c8EVSEKEUvJUgXjc4lP
c1ftRfpC9WNnwczu1uaAk6vfXF/plEB/9knBYhpAcORpWxB8f64oSrz5EanJiQbv
NWJ+1ZCjlwKBgQDiJraZuKph390qVn4CJ7EwnluABTrjxRVAshAqaHusdsFkgoZ8
Azo31S9jiG2SB/wgM8+DVXWuv9eUx231bhizzRTYMv3hPj+a7XcBm5PanUpwroKT
DR1mmAXZaH39DQ0rpMMJ1jhyZUWRf+rzeEz2OMci50bbS64XBs5XMrEd/wKBgQC4
tu3ZMP2N6n1Kwry6aCav/Ci2lfRh/+rrLL+4Jp6fNna2A4nj9vk5cNUSmPuq7X4W
ni8aWGQMg7QfVaPM586Vun2ax3xV6qNh3GdLT6kiKQuHWnjWZga12lTKmvK0k3jj
DDfkZXTlkV97bTU3nyrZQfffl8YnN6ZVaVYJuF19PQKBgAXShPkDmWPV3EwinvlK
Aak2GBskgGpbpsYb0pUDn/PRpk/5Y4TC+Pi8mDXllUeAK8T52+BtgOwqRv4rMc6o
cpjbImp0NonsrJjYgBlqvSmGCVFD8Fs/X3bGU/eMTR223jQt68ZZgf0uEFwJiApW
TBZ3pOmBQcldj1vo3JmpQvcT
-----END PRIVATE KEY-----
)");

  constexpr char const* testRsaPublicKey(R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzqrUGjC9YogNo2Jq3j5b
A4zzh+NvWQFzEDTwllqhJDWNU+R3JAPXVvNT+N8PP5PNbT7gtni1QQ/9qM2kSGaV
opG8LLzeLqEIayQ6bkMidlevNX0Vo/A4p6nHdgnGq/IoMLxPVCTBg5F/qlrYvgcA
OHSeei88W0B2HN/ZlXJqLH6buIoYnzI9lOGzQPntLPH7f1RJtlBuIVoMyjTKxr6p
LrABr1rNP1D7YW+xyRFWWh1+Xm19i4HnVIEpfPKHuD+bTUfF0m0776d6D8eBJDOt
0uMKeANibH6TYGD09aDUQfv1R41vjmYymmI6E6rPvjOypx2lJ/qlMArjGJnlSNnB
GQIDAQAB
-----END PUBLIC KEY-----
)");

  std::string CryptoTestCollateral::TestEcdsPrivateKey() { return testEcdsPrivateKey; }
  std::string CryptoTestCollateral::TestEcdsPublicKey() { return testEcdsPublicKey; }
  std::string CryptoTestCollateral::TestRsaPrivateKey() { return testRsaPrivateKey; }
  std::string CryptoTestCollateral::TestRsaPublicKey() { return testRsaPublicKey; }
}}}} // namespace Azure::Security::Attestation::Test
