// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Base64, Basic)
{
  int maxLength = 7;

  std::vector<uint8_t> data;
  for (uint8_t i = 0; i < maxLength; i++)
  {
    data.push_back(i + 1);
  }

  std::string result = Convert::Base64Encode(data);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQIDBAUGBw==");
  EXPECT_TRUE(std::equal(data.begin(), data.end(), Convert::Base64Decode(result).begin()));

  std::vector<uint8_t> subsection = std::vector<uint8_t>(data.begin(), data.begin() + 1);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQ==");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 2);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQI=");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 3);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQID");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 4);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQIDBA==");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 5);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQIDBAU=");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 6);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQIDBAUG");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));
}

static thread_local std::mt19937_64 random_generator(std::random_device{}());

static char RandomChar()
{
  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::uniform_int_distribution<std::size_t> distribution(0, sizeof(charset) - 2);
  return charset[distribution(random_generator)];
}

void RandomBuffer(char* buffer, std::size_t length)
{
  char* start_addr = buffer;
  char* end_addr = buffer + length;

  const std::size_t rand_int_size = sizeof(uint64_t);

  while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }

  std::uniform_int_distribution<uint64_t> distribution(0ULL, std::numeric_limits<uint64_t>::max());
  while (start_addr + rand_int_size <= end_addr)
  {
    *reinterpret_cast<uint64_t*>(start_addr) = distribution(random_generator);
    start_addr += rand_int_size;
  }
  while (start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }
}

inline void RandomBuffer(uint8_t* buffer, std::size_t length)
{
  RandomBuffer(reinterpret_cast<char*>(buffer), length);
}

TEST(Base64, Roundtrip)
{
  for (std::size_t len : {0, 10, 100, 1000, 10000})
  {
    std::vector<uint8_t> data;
    data.resize(len);
    RandomBuffer(data.data(), data.size());
    EXPECT_EQ(Convert::Base64Decode(Convert::Base64Encode(data)), data);
  }
}

TEST(Base64, keyVaultBackUpKey)
{
  std::string backupKey(
      "JkF6dXJlS2V5VmF1bHRLZXlCYWNrdXBWMS5taWNyb3NvZnQuY29tZXlKcmFXUWlPaUl5WVdabU5tRmhNUzAzTm1Ka0xU"
      "UTBZVGN0WVRjek5DMDJaalZoWkRCaU5XRTRPVGdpTENKaGJHY2lPaUpTVTBFdFQwRkZVQzB5TlRZaUxDSmxibU1pT2lK"
      "Qk1qVTJRMEpETFVoVE5URXlJbjAuSWo0aWp4TE01NzRjbHF5RU1DR0d5MHJDczJsczVJc0dPSk9IdC04a1hkakFPZnUy"
      "YkRxSENMOG5ycVhjUHVYcmxCRlh6WW1UQU1PNXA5VDFHdk9FSE52YVg1WTIwNmdBU3BsN0UwWVQzWjljQTVYa3dqMWty"
      "TTAyNV8telVsaU1hSzhNbkJwQTNOSk5Cby04b0I3S3Rpd3QtVFB0OXBHQUZEeGxWRTdhZmFxUzZnOWlUMmZ6ckYxVE5I"
      "ZFFkbVZ4aTZtT0x1X2NJV1lmUHFtZHRrVnlFaTMza09VTVhFLTM2NVBmaEpJSjVWZ2w5RkxzVGZfbHBuTkhzclYzVWVp"
      "SEVCcktHWXR3OEdfcGNsakhmeXJWTmFXRVlfR2ZWY0l5VUJEY2wzYXU2RF9jYVRkUXROa0l3MDI1MDgtWDVwUHdONk5P"
      "dWZqc20zZGlhaHpxTUlKSVlnLjJKYW1zVjBmV1Y1OTU4WlJPZnZzY3cuYkt0b3NGMHF0bXNodTFsYUN2cVpYSnpPMnVV"
      "VHROTWZQbm9RT2IwUTV1a0VrcGdqT0IyTUlOemp6RjltTHJVQW9BMnpjSTdaLWlrWEFIM3AtbFV4VUJ1V0w1MkRWMVh1"
      "ZkRtXzlsZ28yX0wyeHVpa3NDUGtGbEdJejdVMmEtZk5ONU02QXl2YWhmcHAycHdRR1BvbU41aUN4clM2SlV6VHBHTVhu"
      "QXd3T1E2dkpCLWFKcG9HRkJxZ2IzM3NIcHhidHJDS2JHZzdrQ1ZSZ1BwSzN5MVp6R2N3dHdKUlhTdm9QYUZqNnJFV0ZR"
      "YUJhSnFZd0lCaXVWQnVwbnFQSVVQVV9DSnE2blB2M0l6S1ZjZXZZeE85SE1JSUZYR1RNTW84aTFqc1dlQ08zTHpGQXJ6"
      "MDR0SXA3R0Iya3V0SmxMUEY5RThZdTNaLW8zaWtkMkNTX3JhU0o3djE2N0xheUVCTHNIMy1ucWw4QUFYUGoyN3EtZWV4"
      "ZEZCUWVia0pETW1Yek5oTzU0c19ZZ1VJQ0VWbUNzNUpyaFE2eGlLOTdIU3ZidVVqTzc3elJLaklxa1VYMkRGdFppTVlW"
      "anQwNXgxRVV0SjBialBvUW91Wk91SjBzVjgtS1kwQ3BvSU5oMXBvb3JEUExidXhsRXlMMWJqMk9LQmFsTVZPbjhwWFE0"
      "cVdSM3ppbGdjWXctR1BFRFRuLU9sdnVBMGRaWWpVNHlCVjczYWlSZ3N4RW9Gdl9qRU40YVR4MXJtN3I5VTZCNlpuXy1F"
      "dlJtQWN6aWdGSGpPUW04b0NpYlVBTlYxOEJrRG5JbDRHTXNMTnRqQm9QVTg1TVVaTlBtUFlEcTAxY25jNWZSakFXeXpM"
      "azlDOFU5b3MyS1lhTnBXTUdtT0h4d19nY190OHJBZkxJaUVFeFlRbkd4aVB1SUtNckZsVXNrVmJsRS1iUGt5WG5vQ04y"
      "RjdKN0hVTUZJZFcyQ1F3cDhZTzdEbUl4RDBGQklKcEk0SlAwa19hQndQdXZqVzFPX2FnU3pILTQxMU5WM2hRWlh5clZw"
      "RlhoUDU2dklMb2JjOS1YNFRPSjFjMkVOUHRwTFVxS0V5SDVIZk1PRjRvTEdMcnAtcFlEeC14c0dWUEFmaVRfQ0pIcGFr"
      "UXNyRXB1MlRyTFlBSkVFUGFELTFTTXBBSlFrd3hBQXRReVV5MFBTRzJJMUtYT0ZRVDRjUS1GQ2ltc1kwOXJ4T0kwajlJ"
      "SzJZbzRUWXBoT1J2U0NjNW51S0cwLTdyYXZIR2Iyd0pDMzZHb0lkbDVLeVZuVVNEbGNwV1BvS3hvRHNFNndWcnU5ZXBE"
      "VTN6Q0JwT2RBcmVVRUFJeFJ3UEUwc0tjNldtbUVTUkh0c0hBLTl2REtfOUczS0NnVnROeWVsWG9aaElac3dSdTBMRTVi"
      "U2FrOFpIT2JKa2w4Ym9NQnB1ZWZoUWJ1ZFZaZ0VNTWhBMW9iRi1uR0xpT05CZ29kOXBtU19sWFhpSjFPRGphb1BOblA1"
      "S3BNQUNaTXhLNGJwdDcydHBjUFlaR0dROWpBZmRlZ0owakM2S1FHSHFyNWRjMG5rOW9aQ3ZObk1vY0xjTk42R0drSDFs"
      "bE5qcDhabkY4aVVoYV96dVktUkxiQkJXLWI0M3dzY2VBWWJuNE0xdnVKRzF2aE96X1pydHFFRGdQam8tUnBKUi1CWE9w"
      "R3FwcVg0R2R2MEpsZGpZbUZHTXRTMjUxLTNnaVl0WVJ0bkhIeFJYNWtfVkFYaVhEMGd3Z21PY2hsSzhqenZUWmhRc0ly"
      "Q1JkTVdTdWVtVHp6MzZ6Rm02ZGE1eWc3TU5HcWZ3dnZpdm9OZV9UTWoySTgwSnIyZkxiZmxpNzhPWFp6YUZ4QXpEanMx"
      "OUg4MzJpNVFQZ1JOTnFROWprTTM4MzBlVlNtc1JUdnRPcWRrN3VjQkxGR1ZEV2JQZXp4cnZsTjFfYk8tbFRMNGp1eW9T"
      "ZFJUZHlobVpFd2VlZ3RaUWE2RC0yWTFyczFoWUVWaG5EOGZrTEVYMldBd25YUWZZUWJIUnVQMEJZMnFwVGFRVnp5aGxi"
      "OG1HdWtqUF9kN25rMVA3NF9WeE41eFB2bjNoYnBSc0tTcGpCVmc1RUpuMzVCaHJ5YlhYek5iTkFWcHdIcGZDdXhrcEpq"
      "WVFNZHY5M1V4NGwwcS1hZUpROE9OTTFibkc2akVEMzQwUkhiZzdzY2Qxbk9ob1hoQ2stcWI5cVlabm5wckhSbWFfMzdE"
      "V1NUV2xpWVNLcGdtMDZJQ2ZvdkIxV3FRejNZNFIzOTBkUTMyYkxzYjVhUXUzVGF3ZGMyUVRNR0czVENPc2hDUW9ONlhI"
      "OGhGeDRibWt2cm1MQ2cwNkc3QWdscVhxbXJOZlROOWxDVWlKYVJSb0x0Rm81SHVXS1dROC1aV29KYXhyZDB2S2hOc1Nn"
      "N09BZUlRWl9zVDN5Y3JnamdYN2VVaExvMkdjWERLTUpsVzlRYVV3LVdTTkxWNENXbG4ya2FrTzd2bmxCUms5R0xONVBD"
      "NERkY1QtVEpQdHo3TEsyQUFIdUdpbzJKSzJZaHVZVzBDS3hCSWVhM3cxbVBZem5JRF81eFk3RzJzVDJ6MHJBVVEyWEhP"
      "el9tZnhzeXR6YmZDQmY2ZlZNTXFNSThTOGRraV9IVWxiOGxSV2RaRnFQTTVqMlI0Yldyd045d084LWhFeDIydXJTUXp2"
      "d1AwenZ4d1RBa09MajRKS1hmMFJoeWtwTTgwamFONkFUMkNGSFIyV2ZDVzUzMFQzMk41U1FBS1VVUksyZE5KNkNKOHh1"
      "WnBia3loTkxieW1YZFVpOF93UVhNLUFQc0QtWFppcl9IeEhvV2NjMW1VeDRBdTU3M19Ra0Exd3E5eFVXT2ZycUdGcHRa"
      "ZldpZTdLYXFhTUoxRzcyeTZhaGhSWVl1bHpXUFdBTVlfZjhubWNnWEtXVUJGZXhseE5URzM2UFZoVEN0VnlZQmhCREJL"
      "aGYwN095Y1k4dGJLMDlIaW5LdEVlNEFyZFE1a3JiMTB0ZU9JeXpkY2RUSVdTT2VHS2txVDBkclJBMDdYUm54X2d6V1BK"
      "UXRCclVOcjlIdEx0Q0RIeUFoY01rSThkYzJqSjFjX09aeXliODlWaWZCODFyR2RjVkVsbXplV2tNMHRKVUNuQl8xdmxD"
      "N29sLXdRVFVvXzdLWnk0N2lRem1uQUo2NXhidFJ0SG9LS25qekRtWGp1bmpVWV9pS0pQNzh6R1QyajBWLU41Y2F3T2F2"
      "V0VGSHBwbUMzZU9JRzZaR1VNdjVLcndrWDRGcDVwVFJhdXVJWmdPRGEzcXI1QTQxNy1fb1lhaTJkX2ZYb1VsVWtpUTZy"
      "S0ozaDExaW5GbTkwSzlQVW44MWpOYjFSYUJnWHRBU0pVZ1plU1BscVl4UUgzMlpUSG0zRGpJUmZhdGV3dl9iTTZDWXVh"
      "WDdHU3lRTzV5ak5PM0g0Y0tkNWVDMFFNZ3lIajFVd2Z4LVlLWXFoRjNEbTJ5LTM2bnkzZHlVV3NrX1UzRHN4WENPSlNN"
      "N2tCMFREWDNwMFBtX3lJNk9VMTlsTmtRR25SNUZNSGVRVEMtUEhkR0JxSHpPM0VNTGRsVUROU3FFWkkxVExhb1JFd2hC"
      "NHFCY21samp2ZTYtOGNYUDJHRHAxd1ZWOVJwS084SXJzQUNRYkpWNG43MFVoUS1lSFN5ekZXUGxHeDZ1cjNpWUlEVGow"
      "QUJvY1NKUFZsM0VKM1F6cUl5eXVITGlWZUdOLTV3NnF5b2dMY3NqZHdiTnZiS0pxTFhFam51VXJ2Sy1ORjVPcGdFb2NL"
      "MUFtV0wtbmlqZDZ5RjhWUjlZRGR5Z095QzNJT0Nzalc3alJKeFVibE5uQmxxWUtudzNnZmJuZ3BUZmFfMFFZLTVQNExC"
      "WkVpWW4tQkZLTlEwd2xFN1pWbk12bXhOcFVrOTdZZFpPMzJJQlRTQ2hXd28wMmF6UUtqWG5Nc0FhY0o1UWtHRHVpb18t"
      "NHYwTG5BLTd2ajR4QjVtamlqRnJKU01rM09SeURIT1lYNDViODM3VkdYSXA4SW1YTVdEOTRmaThVMlBkeEJrU3lOTTho"
      "THhZclZSUGdUODUwWEg1UDFGZERNVlFJR1V5MHlEcHhGaEduZWNxbTRhSnBmelJqXzZpZFBGOVAxOURYSjRzeEpvU2pR"
      "UEJoNUxWeWJrRjlXLUJHazZ6LXJMMDhXU1RYUnJCU2FGRG0zQTFhdi01b0RqQ2hUZmRYSUFhVWFBXzJrR0pCeTRmSm1a"
      "M0hqY2ViNnRIUnFfaUhWMTBMQVBXcVY3LWhLUjZfVjJuWHdEQ3ZPRkN2RUVtRmlYa2NDRjVuaXRXM3BZSzNlV2FXb0p0"
      "UkNiYTJzNC1mUEtuRk8yLUcxeTVYUW9NSVM0azQyOEpUc2J4bVowZnplczh2MVRWMFJrakFCSnhRUmxySG8zMHdyT0tK"
      "Q05wRGxsWlBsZ2JtTGVrbWtLYm1MVXpOSDNudXpYRDllOXpiVUUzckhFR1N3OE9KdEtFRDQyUkFIamdXV29uMVRwZFlH"
      "dll0aV9nd1ZSQ1RlcVgzUjRWSHR4Q3NOZUFCMjZaQnVVcFlVSFJRZGJQaDdEMGdGTF9GenNtcmIxb0NDV2tiOVdWbFBE"
      "QkhmTDNwOXE4QzZ0UmVXZGYwVTloRmt2QzZRaTh2eFZ6b0JXQzkyTkdpZUxYc0cxNXRZR3F2cHdDbTE4TlpTcEdHcU5y"
      "N0YzWTU2VXFoNnllOTVMUjBkZE93dnJnb1JYZ1QyQjUxR3VOVjFCV1VhcW81RVc3OXEzNUE5c3VELXlqX1FwVWw3SXVh"
      "TEFDcnprMVBpZEhOb2lyX1p1NF9pVVFaZ0c5Z2hPTHRzLUQtaFpnMncxMDBOTGdIVDNWOVg0ZnVqMk5uT2xxNWZPSUJB"
      "RHZqVWNUWmlhcWM5aE94c3dFbzZLNUZlcXlndzNTUUdmN3A5ZHhsaW8tT2J0dGh6alppVW50WEd2Y19DM2taYk9QTHVk"
      "TjhBSjRSc0dweTE0VGRuWTM3Tko5WnZNQzZhZGJudEhXd3BuLXRzZjl3MnJnX0V6cDY1NVJIZjh6WW1nZnZNVnUwMDE5"
      "X2d0OG9USHQzNHRBN2J3MGlrdUttZVRBc2s2aGJkQzZZQzRQaHZtWUhwT2kzb1p6eG5vR3dXMmluTWNUY0l2ZjIzNDBv"
      "dXJIeHZvc0ZNcmItN1JQRWtyT0VhNHppOHl2UUViWmxMOElSblc2S1g2ZnlYX2pmakw5a3ljUTJZMDM0RkttWVZSUk9I"
      "WmdydkQ5bDVVX0l1aVpKX0Zwcjh4T3RWYW1Oa2FHSFZndzhHOVg3WkNqaVJKLTZwTWZFTDI2SG5sMEZINWM4THZ5NjBu"
      "MFJjSUdoLXVtMll4aVh1SzVQRl9qRWJXUmowQXlscjFhd1V4SkFBanlDWE1oUXNNRzU1MmdOeUJfaHFqUFVfLThQZmg0"
      "N3VXZmJQOFVFSkFDWlFXOGxwUFR6aGMyZzlUYWdlZGF2WlF4Q1Z1cVl3WlVENEtjaFZuVEVITjRVYVB3UHJvQ2YtZGlJ"
      "UDJZYTYxR2NQYzd5TjdXSy1xWEJoR01KaXZNUm1JLTEzVGZqUTBtM2tDMEhwWEJSbTNuZklyS192NGpPenNhNnctdmNK"
      "N2xtbU1kRUxlRE5USXJJZDBzNG1VZW56bXBEbHkwSnN3LV9BSGE0LVRVRjg3T05LUlh5eGNWN284b0JfMHJUbjEyNzJt"
      "OTZHeUlvb2lEanVGUmZMZ0lmcGd0dnY3M0pWY2t0VHlJakh3TUROMEVOcDZCZy1TZDUyeVhGQjk3cXFEYlBjZHk4Qmpy"
      "SEkwOXZoeDFWbTJ4cmw5cXl3Wkh2UFZORjZqVHlBZjMtSVlINmp2RmhwaTB0WjVaM0pXbkZiYUVXS1Y4cDUxSm52M3Nx"
      "ZkhNdm81Y25SejN0TjJaZ0VlRWcwbk1fWHU0by01eWFXbVZsV3NYcWJuYUhBdTR4aGsyV2ptaHV2anhUbUlmRS1DTk0z"
      "TURXenhpM2c3NTJTRGFvbVpiM0IyV0FDeVBZNEIwUWFwR0d4VlF6Mk9ZZjZFSEl3WVhES2tRTnRLd2JXNXcwWUpSd2p3"
      "M2FYM1ROODVnWFk0SWNKcmRnS0s1Sk9JNU5qeTN0SVgtX1ZaTEZWTjBmWEJBSFlFTjAxeEIzSU1sRUxwdUN1aDFIbGpM"
      "TUVhWUlMR1d6bzhyYkp5TzdRajJGUUJGbHAyTzRvTjFzSDVIRWhROFk2UlU3VkpENGFwbE5GTlJ2a2toRUxEdDZSMHFW"
      "LV9yR01tcUVQdzMwQ1E0c2VWQXpqM2JHa3otTDVjdUVuUkJOVXpsekFRWDkzZi02UEZzaS1iUlpyNUd5bm1VeUtKMW56"
      "ZG1uNnUtdUUzekh3dElQVm01dC03RGppR09aRktYY0dvVGdIQ3pMVURkaDc2eFU1b3RQQVZWaGlnWDZUdURSWTFXOGM2"
      "X3hWZ1daLXdoR2NmUlJ3ZGlqX1lQZ0lsR185MTdVcmJUX0lDUnltWGVxQ3BhWHBtUk5KWjZXZ2lsNDdhVW5EeDQ2Y0tV"
      "bkx5LUNEdk5tRS1LSTVEZDkwVDZVZWNaS2NzbGJyVGF0OWRQRHFRTElMMG5ieU9CWHN3Q2k2c2N6ajRpU3d0ZGUySEUx"
      "NjdGT1RnMWRPcnZRRGtBcDdJLXdUQUZqdURzalY3WGxvYmVjMF9mUXBZb0NlR0VHWEhlSUhYeUw2YmVHR2VaS3B1UUxk"
      "amhzQ3ZweWVQOTk1UHJIb3dUUkJtTlltcVBRUTlwZzQ4VThOMWpFX25ldm5XSUlzbGFYV0hMbkY5NU51WFhyRkQ1dGZB"
      "bHVuUlY5NmVVU0lTeE52dE10U2lhdEpJYXVHbjhjLlplemVDZW51ODduV051WVZfZ3gzd2FEX09ELUh6MTJna0hYa2RR"
      "TDl2QUE");

  auto decodeValue = Convert::Base64Decode(backupKey);
  auto encodeValue = Convert::Base64Encode(decodeValue);
  EXPECT_EQ(backupKey, encodeValue);
}
