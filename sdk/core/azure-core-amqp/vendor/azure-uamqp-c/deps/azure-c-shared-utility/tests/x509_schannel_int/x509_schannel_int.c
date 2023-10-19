// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Integration tests for x509_schannel, in particular that x509_verify_certificate_in_chain()
// correctly determines whether certificates chain up to trusted certs or not.  There are
// no networking calls or changes to local machine's certificate store, so it runs 
// without need for a server on other side of test or for special privileges on host machine.

#ifndef WIN32
#error x509_schannel_unittests can only be compiled and used in Windows
#else

#include "windows.h"

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif
#include "testrunnerswitcher.h"

#include <wincrypt.h>

#include "azure_c_shared_utility/x509_schannel.h"


#define TEST_MAX_SERVER_CERTIFICATE_ENCODE_SIZE 4096

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a root cert that in this test our simulated client certificates chain up to.
#define X509_TEST_CERTIFICATE_CHAIN1 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDojCCAoqgAwIBAgIQZ4iO7pTqsrtEub7360KODTANBgkqhkiG9w0BAQsFADA6""\n"  \
"MTgwNgYDVQQDDC9BenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n"  \
"dGlvbiB0ZXN0czAgFw0xODExMjgyMzA0MDVaGA8yMDY4MTExNTIzMTQwNVowOjE4""\n"  \
"MDYGA1UEAwwvQXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRp""\n"  \
"b24gdGVzdHMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDB8Qk1R0fU""\n"  \
"l6nQLnfS9IBIvPxWKIbsVc87JCvgjW8FvX9o6+NadFSd7v4bvmIxk5Z9W9agMABT""\n"  \
"iT/KLIYzmJ/Afn+7moi9M+BF/sE7Mu+Hm4LVKZTbbaW5OQ1ySOFVpneoj4GyXXn8""\n"  \
"yYRspav0aUBaLyOGWyG4UUKXgrPa0jiEDmcagmrtfCr3TyowIqzb4Xhvoe9Lqqt4""\n"  \
"s8l2KHHAf4sGqye+pcC2EVWmcx1XsS97u8ngOOSSMe7pA4KTJ8+dbWfYa2kPqhle""\n"  \
"cRIeRBm7PrZA6nINT+uRQmilE+ZuuLgHoCRpDXqgJGv/we5Frhe4g2rPPORSsyv7""\n"  \
"lbBSCOzbbHfNAgMBAAGjgaEwgZ4wDgYDVR0PAQH/BAQDAgIEMB0GA1UdJQQWMBQG""\n"  \
"CCsGAQUFBwMCBggrBgEFBQcDATA6BgNVHREEMzAxgi9BenVyZSBDLVV0aWxpdHkg""\n"  \
"eDUwOSBzY2hhbm5lbCBpbnRlZ3JhdGlvbiB0ZXN0czASBgNVHRMBAf8ECDAGAQH/""\n"  \
"AgEMMB0GA1UdDgQWBBQLDpN3UOpuCRNiQGscPM7a2egIGjANBgkqhkiG9w0BAQsF""\n"  \
"AAOCAQEAOn9hb0+2Z7zKHBUtAnSBcTRClOhHlyk6BYqyhbn56FQuZY+8WfNTJsWp""\n"  \
"gJCiEqUz5GbBzEVrlxdUSfu7qAISQjqhAN8lndJ7Ux7V6IPFSzVVfqv+vkHRJREV""\n"  \
"L8gQJfBhcjqcKgB1fNH3dVk+rnNhqZEfdTayamlgCCqiNpl26PbtFLM0wjFGwVjH""\n"  \
"1f2SJ4EIGPgiFZj0k7IIpRb3eDBJ37bA2cfRqD4bscR2rAPn8jZDrE3UCIuQXSYS""\n"  \
"ama9/loDlT4qyA/g/YHKMKlI8iTv98/k7gG6TL7Tep6N9BDEcy/NH8cZPjSxi94q""\n"  \
"4CJkC6ZhNoDZ8WsCIGsoJqa3cGktAg==""\n"  \
"-----END CERTIFICATE-----"


// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN2 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQGF/+e0JWT55NwcioWwYK4TANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjIpMCAXDTE4MTEyOTE2NDM1OVoYDzIwNjgxMTE2""\n" \
"MTY1MzU5WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjIpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEA31oJhWVlg2IlNv/WNHqu/pg4hqMvsWGLLE6rzrxnoCVi""\n" \
"Z0I3uAD+vHm5C3Eg5p3mq1zv1vpXxFDdV6fL83XPLXq+dgQp8xdj8MtnKE/RVSsJ""\n" \
"reH+u/xXnwd8fzu8Q7l4qvU3nTJk3EtkwkVxM5vP5Gg7MnHx2JHdxeW3y1PdFiOV""\n" \
"1uKi19iz+n5FhKBmLeuPd1G3pXA1r+MU/saVloGtLDmH8+YiMnDP4iHLLU6yWK7r""\n" \
"c4HW4sdR9209zddN4YlKT0MgtJlKkZaOWowThmyubomsuaAPTB5WCW7qTFcNS9Eu""\n" \
"N/GXYUbHdqWggeiJK5y2ELnwB0p+QOZwL/ir+SG9PQIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW4yKTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBT7""\n" \
"ekxNZopXaL2cfcEBXWiwEwHrGTANBgkqhkiG9w0BAQsFAAOCAQEAt8/jfQQ2GhPT""\n" \
"FQk+7IjYscNZLff6v67hXIUKdhk4kcEu9uErIRexMKv+XCFRXFyR/DZ620YSlyXQ""\n" \
"+99vmnHYYoNMAeD6v15p1yJ7/M6lUOXvJ4HHTpvngOFt6pNXwVzDJ94Bd/FodZQb""\n" \
"DuhYvA85B5aavUv98a+KzCIzaQ3gmiR1D+4Jj62n4VtyQC1ZamkclLk2DG/TCAKF""\n" \
"RLrpfu2PsFjnHS+RBgvTggSBksGucX5TmkgRAsYgocr6pnjqVfv7Rd1tC+TLygV8""\n" \
"MtlqCMFjiRs8LIu9DWaF+Wf6z4QGVGhIDW4W+utcE7ORc4FPuKuufxNs5quVhTR6""\n" \
"N/MOypAmlA==""\n" \
"-----END CERTIFICATE-----"

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN3 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQJSMG/xxtuYtKc/9ipIkzqDANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjMpMCAXDTE4MTEyOTE2NDUwOVoYDzIwNjgxMTE2""\n" \
"MTY1NTA5WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjMpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEAvwHMSrmTvqGHlkrImSW7Zla8b0aqaPjjzuBg993dzOQM""\n" \
"appCFotZBhvnapZ1gyYPRLfFmyRW2g5PPpIhvA02DT+WyrA0RNtxrn0dtDmlKk4E""\n" \
"uEU4pGXyAOw9O0TrrSuiyOw/BcRTSHgWSkeKiaDpP1oGV7fI+Gb77hHZzP9gWSUg""\n" \
"kVlLFQnz92X8UPGEqvnkZF06rRmtgWXJ8L+GvZBdw2DiKutdQdXja0eusbtCo16n""\n" \
"hn17+VjbXyJq4D0wPQd2BlsTxNarWGMOBdzigd36WLz5F6QI40k4dYzMHSV7lfNk""\n" \
"nn9jYON04N7GsVZiDEQW8/dte6dw/GoO14iBv4xInQIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW4zKTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBRO""\n" \
"+UANd6vHbVPXyMx/C3sBL7bsLjANBgkqhkiG9w0BAQsFAAOCAQEAp5UUFJB7hK7+""\n" \
"lpt4wDYJ+8ao8+a0WNtNwQf/IxLU6Mi6o39uMIZaJhXjlL8bf7cVIschOBEeWxSZ""\n" \
"4HvP8FZxvNo+WsmZtteFOhRwI4Aoa9xebmdcgU0HmF5IqsUu06xKgTeFwQYCfSjt""\n" \
"rqGldWEtfaiIX+GAp6AO5AIPi8ScGwq3WDCbLp2CSJ35i8EzFF86uF6nqt1I9cYE""\n" \
"P33C5qDlgPV9/pjxSsjeeuL1p2BHt17hFNMX40CnS2uuabOiejRKvNrORTr4FDxN""\n" \
"FdJEWbZ1DAvf/Vxx2u36LIuACGeCMhvWH0G3fdMchqcfQRhdNIDco62U42/98CRP""\n" \
"cFyISsIRMg==""\n" \
"-----END CERTIFICATE-----"

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN4 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQWetmG6MFzotE5ijvB0TaQzANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjQpMCAXDTE4MTEyOTE3Mjc0NFoYDzIwNjgxMTE2""\n" \
"MTczNzQ0WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjQpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEAvlVWHCBFYzk3V/mBMYi9MCaWdU+n2GgBTVh/00oFp1/o""\n" \
"R71qJsqVqfkOWMh7PwMi2IweETW/2w/b1qcUDWhLGfH2rQALizsIcIXXfTptIXHg""\n" \
"wSFhrGa842EoDHVXfejDoZToacLij1iu/YlNWqraW57iujdt9OL8o1atLR8413wM""\n" \
"wS8CC/oWI1/UfHJK8Mw8GcJJNfBC/oKWAcEcxdvX9npn/5R35/MgRqehtpged7/0""\n" \
"RMeONkQ8YWHxdjAqUIxnSzDEL+kq/oFHskN1oFpKBmIlWe/ZZAalFs7x4vC3Lonz""\n" \
"9knz6KZH5qEGWVKOr9rCwtmPvMO8t6Jh+Rr80cg44QIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW40KTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBQj""\n" \
"+JrZbDPGFbBAnJHA5SnhFljtZjANBgkqhkiG9w0BAQsFAAOCAQEAN6jtbDX1YW70""\n" \
"4drtz4xxbRCVsHrXy7RcdfEeqNipgvzX4C5x6twcCULXGlm/s5SjwaR0UfE6mZFA""\n" \
"2gb50R3q8XOdEqoAMJYgjerwuUr38SohzpSeMaWjTwWLFhVkxW5vt/pv6f2/HrTN""\n" \
"Z8qQectIljP0oeVBhouvmXGPfZpYWM7ckS4mx+FocAtX6gWGufXhiOulchsjT1HV""\n" \
"E9hYUiyCa+0u9AHBuhdipzGeZV2KnIBW9QMQR0OMIlwhuVyQz5GwXuHus2BnVsgg""\n" \
"7tO6HHkgddwJhk+3kyDy12MING6MPF45g2yvMoJYl2fviua5mlFVx6g1wrxeS0Cc""\n" \
"g+y5cZsf/Q==""\n" \
"-----END CERTIFICATE-----" \


// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// Simulated server certificate that is chained up to X509_TEST_CERTIFICATE_CHAIN1
const char* x509_test_server_certificate = 
"-----BEGIN CERTIFICATE-----""\n"
"MIIDeDCCAmCgAwIBAgIQTw2XxN6ct5VDfdcMxzzcBDANBgkqhkiG9w0BAQsFADA6""\n"
"MTgwNgYDVQQDDC9BenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n"
"dGlvbiB0ZXN0czAgFw0xODExMjgyMzA1MDNaGA8yMDY4MTExNTIzMTUwM1owFjEU""\n"
"MBIGA1UEAwwLdGVzdC14NTA5LTEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK""\n"
"AoIBAQCuNtGq7eU1rijE7kkjpQY763K09I54XScDM7N2XHsTtDYG6L1khqbovgZ0""\n"
"mJ0uhCRjYBUi4yJYaPRdvKQx5taDRzf3GZ4rvpYwBdWRcor/o4k9iDzZGj9qnnP1""\n"
"8cVwVaLbcnOwRWJYyEPznfTcCFG2LHOIF3iFdcohKdY8iALLacwU95dYywgDWWgx""\n"
"sv2Q8C+maKjik38s3en5QQI+lHKKEXfnTYkWtPqtejXGakrOHJWcnF8tjQi6hMMe""\n"
"sZb1/PL1oeYDEpeQguNe0VznqDG0mHUeBGbyNNLBnkd12MmgxBF32cEC+INVo+Wn""\n"
"S2Gp0h2RfK4Vt6ovAKxhQzzyVxuRAgMBAAGjgZswgZgwDgYDVR0PAQH/BAQDAgWg""\n"
"MBYGA1UdEQQPMA2CC3Rlc3QteDUwOS0xMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggr""\n"
"BgEFBQcDATAPBgNVHRMBAf8EBTADAgEAMB8GA1UdIwQYMBaAFAsOk3dQ6m4JE2JA""\n"
"axw8ztrZ6AgaMB0GA1UdDgQWBBS+QQFsY+Ob8IxaayQ9wl0uuJXsFDANBgkqhkiG""\n"
"9w0BAQsFAAOCAQEAgexxPxV/EXwnDaEAdnrKQfyE5/vtElitWaeCsQij0m8tbPCj""\n"
"jNHOUAM3XwrQNoffk5zW9o/njrdWcMFYcW6SyS9npBKuoCOSKDYV5p+fPluFzgie""\n"
"0BWlr8oZLpuqKNf5yQE7p6RdHWguqmSruY6HyfOjjwWtkUQSeXbA9y3qr5rypVaQ""\n"
"zKsPi9h7CkUsoTFnUFZA+UkIfoJZ+Y+7d2bfg/u6LTRsnURlr5E++Jn1zlUYUl0r""\n"
"/WwcRPj80Swd8ntifdBCx/XwJ53tWGXFx1xopmNamSPAsfwEvpFstWZFyxmCcuN5""\n"
"NqMEOiICezJDDG3q/THL/lQANjVo4QFMMdmDZA==""\n"
"-----END CERTIFICATE-----";

// Tests whether serverCertificate chains up to trustedCertificate or not
static void test_VerifyCertificateChain(const char* trustedCertificate, const char* serverCertificate, bool expectSuccess)
{
    char serverCertificateEncoded[TEST_MAX_SERVER_CERTIFICATE_ENCODE_SIZE];
    DWORD serverCertificateEncodedLen = sizeof(serverCertificateEncoded);

    ASSERT_IS_TRUE(CryptStringToBinaryA(serverCertificate, 0, CRYPT_STRING_ANY, (BYTE*)serverCertificateEncoded, &serverCertificateEncodedLen, NULL, NULL)==TRUE, "CryptStringToBinaryA fails, GetLastError=0x%08x", GetLastError());

    PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, (const BYTE*)serverCertificateEncoded, serverCertificateEncodedLen);
    ASSERT_IS_NOT_NULL(pCertContext, "CertCreateCertificateContext fails, GetLastError=0x%08x", GetLastError());

    int result = x509_verify_certificate_in_chain(trustedCertificate, pCertContext);

    if (expectSuccess)
    {
        ASSERT_ARE_EQUAL(int, result, 0, "x509_verify_certificate_in_chain failed but should've succeeded");
    }
    else
    {
        ASSERT_ARE_NOT_EQUAL(int, result, 0, "x509_verify_certificate_in_chain succeeded but should've failed");

    }
}

BEGIN_TEST_SUITE(x509_schannel_int)

TEST_SUITE_INITIALIZE(suite_init)
{
    ;
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    ;
}

TEST_FUNCTION_INITIALIZE(function_init)
{
    ;
}

TEST_FUNCTION_CLEANUP(function_cleanup)
{
    ;
}


// Test server certificate chains directly to the trusted root
TEST_FUNCTION(x509_verify_certificate_chains_one_trusted_cert)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1, x509_test_server_certificate, true);
}

// Test that server certificate that does not chain is rejected
TEST_FUNCTION(x509_verify_certificate_does_not_chain_one_trusted_cert)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2, x509_test_server_certificate, false);
}

// Test server certificate chains directly to the trusted root and 2 certs are passed
TEST_FUNCTION(x509_verify_certificate_chains_two_trusted_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN2, x509_test_server_certificate, true);
}

// Test that when 3 trusted certs are passed in, and the root we chain to comes first, that we succeed.
TEST_FUNCTION(x509_verify_certificate_chains_three_certs_root_first)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, true);
}

// Test that when 3 trusted certs are passed in, and the root we chain to comes in the middle, that we succeed.
TEST_FUNCTION(x509_verify_certificate_chains_three_certs_root_middle)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, true);
}

// Test that when 2 trusted certs are passed in but we don't chain to any of them, that we fail.
TEST_FUNCTION(x509_verify_certificate_no_chains_two_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, false);
}

// Test that when 3 trusted certs are passed in but we don't chain to any of them, that we fail.
TEST_FUNCTION(x509_verify_certificate_no_chains_three_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3 X509_TEST_CERTIFICATE_CHAIN4, x509_test_server_certificate, false);
}

END_TEST_SUITE(x509_schannel_int)

#endif /*WIN32*/

