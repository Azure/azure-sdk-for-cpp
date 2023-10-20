// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstring>
#else
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/buffer_.h"

static const struct
{
    size_t inputLength;
    const unsigned char* inputData;
    const char* expectedOutput;
} testVector_BINARY_with_equal_signs[] =
    {
        {1,(const unsigned char*)"\x00", "AA==" },
        {1,(const unsigned char*)"\x01", "AQ==" },
        {1,(const unsigned char*)"\x02", "Ag==" },
        {1,(const unsigned char*)"\x03", "Aw==" },
        {1,(const unsigned char*)"\x04", "BA==" },
        {1,(const unsigned char*)"\x05", "BQ==" },
        {1,(const unsigned char*)"\x06", "Bg==" },
        {1,(const unsigned char*)"\x07", "Bw==" },
        {1,(const unsigned char*)"\x08", "CA==" },
        {1,(const unsigned char*)"\x09", "CQ==" },
        {1,(const unsigned char*)"\x0a", "Cg==" },
        {1,(const unsigned char*)"\x0b", "Cw==" },
        {1,(const unsigned char*)"\x0c", "DA==" },
        {1,(const unsigned char*)"\x0d", "DQ==" },
        {1,(const unsigned char*)"\x0e", "Dg==" },
        {1,(const unsigned char*)"\x0f", "Dw==" },
        {1,(const unsigned char*)"\x10", "EA==" },
        {1,(const unsigned char*)"\x11", "EQ==" },
        {1,(const unsigned char*)"\x12", "Eg==" },
        {1,(const unsigned char*)"\x13", "Ew==" },
        {1,(const unsigned char*)"\x14", "FA==" },
        {1,(const unsigned char*)"\x15", "FQ==" },
        {1,(const unsigned char*)"\x16", "Fg==" },
        {1,(const unsigned char*)"\x17", "Fw==" },
        {1,(const unsigned char*)"\x18", "GA==" },
        {1,(const unsigned char*)"\x19", "GQ==" },
        {1,(const unsigned char*)"\x1a", "Gg==" },
        {1,(const unsigned char*)"\x1b", "Gw==" },
        {1,(const unsigned char*)"\x1c", "HA==" },
        {1,(const unsigned char*)"\x1d", "HQ==" },
        {1,(const unsigned char*)"\x1e", "Hg==" },
        {1,(const unsigned char*)"\x1f", "Hw==" },
        {1,(const unsigned char*)"\x20", "IA==" },
        {1,(const unsigned char*)"\x21", "IQ==" },
        {1,(const unsigned char*)"\x22", "Ig==" },
        {1,(const unsigned char*)"\x23", "Iw==" },
        {1,(const unsigned char*)"\x24", "JA==" },
        {1,(const unsigned char*)"\x25", "JQ==" },
        {1,(const unsigned char*)"\x26", "Jg==" },
        {1,(const unsigned char*)"\x27", "Jw==" },
        {1,(const unsigned char*)"\x28", "KA==" },
        {1,(const unsigned char*)"\x29", "KQ==" },
        {1,(const unsigned char*)"\x2a", "Kg==" },
        {1,(const unsigned char*)"\x2b", "Kw==" },
        {1,(const unsigned char*)"\x2c", "LA==" },
        {1,(const unsigned char*)"\x2d", "LQ==" },
        {1,(const unsigned char*)"\x2e", "Lg==" },
        {1,(const unsigned char*)"\x2f", "Lw==" },
        {1,(const unsigned char*)"\x30", "MA==" },
        {1,(const unsigned char*)"\x31", "MQ==" },
        {1,(const unsigned char*)"\x32", "Mg==" },
        {1,(const unsigned char*)"\x33", "Mw==" },
        {1,(const unsigned char*)"\x34", "NA==" },
        {1,(const unsigned char*)"\x35", "NQ==" },
        {1,(const unsigned char*)"\x36", "Ng==" },
        {1,(const unsigned char*)"\x37", "Nw==" },
        {1,(const unsigned char*)"\x38", "OA==" },
        {1,(const unsigned char*)"\x39", "OQ==" },
        {1,(const unsigned char*)"\x3a", "Og==" },
        {1,(const unsigned char*)"\x3b", "Ow==" },
        {1,(const unsigned char*)"\x3c", "PA==" },
        {1,(const unsigned char*)"\x3d", "PQ==" },
        {1,(const unsigned char*)"\x3e", "Pg==" },
        {1,(const unsigned char*)"\x3f", "Pw==" },
        {1,(const unsigned char*)"\x40", "QA==" },
        {1,(const unsigned char*)"\x41", "QQ==" },
        {1,(const unsigned char*)"\x42", "Qg==" },
        {1,(const unsigned char*)"\x43", "Qw==" },
        {1,(const unsigned char*)"\x44", "RA==" },
        {1,(const unsigned char*)"\x45", "RQ==" },
        {1,(const unsigned char*)"\x46", "Rg==" },
        {1,(const unsigned char*)"\x47", "Rw==" },
        {1,(const unsigned char*)"\x48", "SA==" },
        {1,(const unsigned char*)"\x49", "SQ==" },
        {1,(const unsigned char*)"\x4a", "Sg==" },
        {1,(const unsigned char*)"\x4b", "Sw==" },
        {1,(const unsigned char*)"\x4c", "TA==" },
        {1,(const unsigned char*)"\x4d", "TQ==" },
        {1,(const unsigned char*)"\x4e", "Tg==" },
        {1,(const unsigned char*)"\x4f", "Tw==" },
        {1,(const unsigned char*)"\x50", "UA==" },
        {1,(const unsigned char*)"\x51", "UQ==" },
        {1,(const unsigned char*)"\x52", "Ug==" },
        {1,(const unsigned char*)"\x53", "Uw==" },
        {1,(const unsigned char*)"\x54", "VA==" },
        {1,(const unsigned char*)"\x55", "VQ==" },
        {1,(const unsigned char*)"\x56", "Vg==" },
        {1,(const unsigned char*)"\x57", "Vw==" },
        {1,(const unsigned char*)"\x58", "WA==" },
        {1,(const unsigned char*)"\x59", "WQ==" },
        {1,(const unsigned char*)"\x5a", "Wg==" },
        {1,(const unsigned char*)"\x5b", "Ww==" },
        {1,(const unsigned char*)"\x5c", "XA==" },
        {1,(const unsigned char*)"\x5d", "XQ==" },
        {1,(const unsigned char*)"\x5e", "Xg==" },
        {1,(const unsigned char*)"\x5f", "Xw==" },
        {1,(const unsigned char*)"\x60", "YA==" },
        {1,(const unsigned char*)"\x61", "YQ==" },
        {1,(const unsigned char*)"\x62", "Yg==" },
        {1,(const unsigned char*)"\x63", "Yw==" },
        {1,(const unsigned char*)"\x64", "ZA==" },
        {1,(const unsigned char*)"\x65", "ZQ==" },
        {1,(const unsigned char*)"\x66", "Zg==" },
        {1,(const unsigned char*)"\x67", "Zw==" },
        {1,(const unsigned char*)"\x68", "aA==" },
        {1,(const unsigned char*)"\x69", "aQ==" },
        {1,(const unsigned char*)"\x6a", "ag==" },
        {1,(const unsigned char*)"\x6b", "aw==" },
        {1,(const unsigned char*)"\x6c", "bA==" },
        {1,(const unsigned char*)"\x6d", "bQ==" },
        {1,(const unsigned char*)"\x6e", "bg==" },
        {1,(const unsigned char*)"\x6f", "bw==" },
        {1,(const unsigned char*)"\x70", "cA==" },
        {1,(const unsigned char*)"\x71", "cQ==" },
        {1,(const unsigned char*)"\x72", "cg==" },
        {1,(const unsigned char*)"\x73", "cw==" },
        {1,(const unsigned char*)"\x74", "dA==" },
        {1,(const unsigned char*)"\x75", "dQ==" },
        {1,(const unsigned char*)"\x76", "dg==" },
        {1,(const unsigned char*)"\x77", "dw==" },
        {1,(const unsigned char*)"\x78", "eA==" },
        {1,(const unsigned char*)"\x79", "eQ==" },
        {1,(const unsigned char*)"\x7a", "eg==" },
        {1,(const unsigned char*)"\x7b", "ew==" },
        {1,(const unsigned char*)"\x7c", "fA==" },
        {1,(const unsigned char*)"\x7d", "fQ==" },
        {1,(const unsigned char*)"\x7e", "fg==" },
        {1,(const unsigned char*)"\x7f", "fw==" },
        {1,(const unsigned char*)"\x80", "gA==" },
        {1,(const unsigned char*)"\x81", "gQ==" },
        {1,(const unsigned char*)"\x82", "gg==" },
        {1,(const unsigned char*)"\x83", "gw==" },
        {1,(const unsigned char*)"\x84", "hA==" },
        {1,(const unsigned char*)"\x85", "hQ==" },
        {1,(const unsigned char*)"\x86", "hg==" },
        {1,(const unsigned char*)"\x87", "hw==" },
        {1,(const unsigned char*)"\x88", "iA==" },
        {1,(const unsigned char*)"\x89", "iQ==" },
        {1,(const unsigned char*)"\x8a", "ig==" },
        {1,(const unsigned char*)"\x8b", "iw==" },
        {1,(const unsigned char*)"\x8c", "jA==" },
        {1,(const unsigned char*)"\x8d", "jQ==" },
        {1,(const unsigned char*)"\x8e", "jg==" },
        {1,(const unsigned char*)"\x8f", "jw==" },
        {1,(const unsigned char*)"\x90", "kA==" },
        {1,(const unsigned char*)"\x91", "kQ==" },
        {1,(const unsigned char*)"\x92", "kg==" },
        {1,(const unsigned char*)"\x93", "kw==" },
        {1,(const unsigned char*)"\x94", "lA==" },
        {1,(const unsigned char*)"\x95", "lQ==" },
        {1,(const unsigned char*)"\x96", "lg==" },
        {1,(const unsigned char*)"\x97", "lw==" },
        {1,(const unsigned char*)"\x98", "mA==" },
        {1,(const unsigned char*)"\x99", "mQ==" },
        {1,(const unsigned char*)"\x9a", "mg==" },
        {1,(const unsigned char*)"\x9b", "mw==" },
        {1,(const unsigned char*)"\x9c", "nA==" },
        {1,(const unsigned char*)"\x9d", "nQ==" },
        {1,(const unsigned char*)"\x9e", "ng==" },
        {1,(const unsigned char*)"\x9f", "nw==" },
        {1,(const unsigned char*)"\xa0", "oA==" },
        {1,(const unsigned char*)"\xa1", "oQ==" },
        {1,(const unsigned char*)"\xa2", "og==" },
        {1,(const unsigned char*)"\xa3", "ow==" },
        {1,(const unsigned char*)"\xa4", "pA==" },
        {1,(const unsigned char*)"\xa5", "pQ==" },
        {1,(const unsigned char*)"\xa6", "pg==" },
        {1,(const unsigned char*)"\xa7", "pw==" },
        {1,(const unsigned char*)"\xa8", "qA==" },
        {1,(const unsigned char*)"\xa9", "qQ==" },
        {1,(const unsigned char*)"\xaa", "qg==" },
        {1,(const unsigned char*)"\xab", "qw==" },
        {1,(const unsigned char*)"\xac", "rA==" },
        {1,(const unsigned char*)"\xad", "rQ==" },
        {1,(const unsigned char*)"\xae", "rg==" },
        {1,(const unsigned char*)"\xaf", "rw==" },
        {1,(const unsigned char*)"\xb0", "sA==" },
        {1,(const unsigned char*)"\xb1", "sQ==" },
        {1,(const unsigned char*)"\xb2", "sg==" },
        {1,(const unsigned char*)"\xb3", "sw==" },
        {1,(const unsigned char*)"\xb4", "tA==" },
        {1,(const unsigned char*)"\xb5", "tQ==" },
        {1,(const unsigned char*)"\xb6", "tg==" },
        {1,(const unsigned char*)"\xb7", "tw==" },
        {1,(const unsigned char*)"\xb8", "uA==" },
        {1,(const unsigned char*)"\xb9", "uQ==" },
        {1,(const unsigned char*)"\xba", "ug==" },
        {1,(const unsigned char*)"\xbb", "uw==" },
        {1,(const unsigned char*)"\xbc", "vA==" },
        {1,(const unsigned char*)"\xbd", "vQ==" },
        {1,(const unsigned char*)"\xbe", "vg==" },
        {1,(const unsigned char*)"\xbf", "vw==" },
        {1,(const unsigned char*)"\xc0", "wA==" },
        {1,(const unsigned char*)"\xc1", "wQ==" },
        {1,(const unsigned char*)"\xc2", "wg==" },
        {1,(const unsigned char*)"\xc3", "ww==" },
        {1,(const unsigned char*)"\xc4", "xA==" },
        {1,(const unsigned char*)"\xc5", "xQ==" },
        {1,(const unsigned char*)"\xc6", "xg==" },
        {1,(const unsigned char*)"\xc7", "xw==" },
        {1,(const unsigned char*)"\xc8", "yA==" },
        {1,(const unsigned char*)"\xc9", "yQ==" },
        {1,(const unsigned char*)"\xca", "yg==" },
        {1,(const unsigned char*)"\xcb", "yw==" },
        {1,(const unsigned char*)"\xcc", "zA==" },
        {1,(const unsigned char*)"\xcd", "zQ==" },
        {1,(const unsigned char*)"\xce", "zg==" },
        {1,(const unsigned char*)"\xcf", "zw==" },
        {1,(const unsigned char*)"\xd0", "0A==" },
        {1,(const unsigned char*)"\xd1", "0Q==" },
        {1,(const unsigned char*)"\xd2", "0g==" },
        {1,(const unsigned char*)"\xd3", "0w==" },
        {1,(const unsigned char*)"\xd4", "1A==" },
        {1,(const unsigned char*)"\xd5", "1Q==" },
        {1,(const unsigned char*)"\xd6", "1g==" },
        {1,(const unsigned char*)"\xd7", "1w==" },
        {1,(const unsigned char*)"\xd8", "2A==" },
        {1,(const unsigned char*)"\xd9", "2Q==" },
        {1,(const unsigned char*)"\xda", "2g==" },
        {1,(const unsigned char*)"\xdb", "2w==" },
        {1,(const unsigned char*)"\xdc", "3A==" },
        {1,(const unsigned char*)"\xdd", "3Q==" },
        {1,(const unsigned char*)"\xde", "3g==" },
        {1,(const unsigned char*)"\xdf", "3w==" },
        {1,(const unsigned char*)"\xe0", "4A==" },
        {1,(const unsigned char*)"\xe1", "4Q==" },
        {1,(const unsigned char*)"\xe2", "4g==" },
        {1,(const unsigned char*)"\xe3", "4w==" },
        {1,(const unsigned char*)"\xe4", "5A==" },
        {1,(const unsigned char*)"\xe5", "5Q==" },
        {1,(const unsigned char*)"\xe6", "5g==" },
        {1,(const unsigned char*)"\xe7", "5w==" },
        {1,(const unsigned char*)"\xe8", "6A==" },
        {1,(const unsigned char*)"\xe9", "6Q==" },
        {1,(const unsigned char*)"\xea", "6g==" },
        {1,(const unsigned char*)"\xeb", "6w==" },
        {1,(const unsigned char*)"\xec", "7A==" },
        {1,(const unsigned char*)"\xed", "7Q==" },
        {1,(const unsigned char*)"\xee", "7g==" },
        {1,(const unsigned char*)"\xef", "7w==" },
        {1,(const unsigned char*)"\xf0", "8A==" },
        {1,(const unsigned char*)"\xf1", "8Q==" },
        {1,(const unsigned char*)"\xf2", "8g==" },
        {1,(const unsigned char*)"\xf3", "8w==" },
        {1,(const unsigned char*)"\xf4", "9A==" },
        {1,(const unsigned char*)"\xf5", "9Q==" },
        {1,(const unsigned char*)"\xf6", "9g==" },
        {1,(const unsigned char*)"\xf7", "9w==" },
        {1,(const unsigned char*)"\xf8", "+A==" },
        {1,(const unsigned char*)"\xf9", "+Q==" },
        {1,(const unsigned char*)"\xfa", "+g==" },
        {1,(const unsigned char*)"\xfb", "+w==" },
        {1,(const unsigned char*)"\xfc", "/A==" },
        {1,(const unsigned char*)"\xfd", "/Q==" },
        {1,(const unsigned char*)"\xfe", "/g==" },
        {1,(const unsigned char*)"\xff", "/w==" },
        {2,(const unsigned char*)"\x00\x00", "AAA=" },
        {2,(const unsigned char*)"\x00\x11", "ABE=" },
        {2,(const unsigned char*)"\x00\x22", "ACI=" },
        {2,(const unsigned char*)"\x00\x33", "ADM=" },
        {2,(const unsigned char*)"\x00\x44", "AEQ=" },
        {2,(const unsigned char*)"\x00\x55", "AFU=" },
        {2,(const unsigned char*)"\x00\x66", "AGY=" },
        {2,(const unsigned char*)"\x00\x77", "AHc=" },
        {2,(const unsigned char*)"\x00\x88", "AIg=" },
        {2,(const unsigned char*)"\x00\x99", "AJk=" },
        {2,(const unsigned char*)"\x00\xaa", "AKo=" },
        {2,(const unsigned char*)"\x00\xbb", "ALs=" },
        {2,(const unsigned char*)"\x00\xcc", "AMw=" },
        {2,(const unsigned char*)"\x00\xdd", "AN0=" },
        {2,(const unsigned char*)"\x00\xee", "AO4=" },
        {2,(const unsigned char*)"\x00\xff", "AP8=" },
        {2,(const unsigned char*)"\x11\x00", "EQA=" },
        {2,(const unsigned char*)"\x11\x11", "ERE=" },
        {2,(const unsigned char*)"\x11\x22", "ESI=" },
        {2,(const unsigned char*)"\x11\x33", "ETM=" },
        {2,(const unsigned char*)"\x11\x44", "EUQ=" },
        {2,(const unsigned char*)"\x11\x55", "EVU=" },
        {2,(const unsigned char*)"\x11\x66", "EWY=" },
        {2,(const unsigned char*)"\x11\x77", "EXc=" },
        {2,(const unsigned char*)"\x11\x88", "EYg=" },
        {2,(const unsigned char*)"\x11\x99", "EZk=" },
        {2,(const unsigned char*)"\x11\xaa", "Eao=" },
        {2,(const unsigned char*)"\x11\xbb", "Ebs=" },
        {2,(const unsigned char*)"\x11\xcc", "Ecw=" },
        {2,(const unsigned char*)"\x11\xdd", "Ed0=" },
        {2,(const unsigned char*)"\x11\xee", "Ee4=" },
        {2,(const unsigned char*)"\x11\xff", "Ef8=" },
        {2,(const unsigned char*)"\x22\x00", "IgA=" },
        {2,(const unsigned char*)"\x22\x11", "IhE=" },
        {2,(const unsigned char*)"\x22\x22", "IiI=" },
        {2,(const unsigned char*)"\x22\x33", "IjM=" },
        {2,(const unsigned char*)"\x22\x44", "IkQ=" },
        {2,(const unsigned char*)"\x22\x55", "IlU=" },
        {2,(const unsigned char*)"\x22\x66", "ImY=" },
        {2,(const unsigned char*)"\x22\x77", "Inc=" },
        {2,(const unsigned char*)"\x22\x88", "Iog=" },
        {2,(const unsigned char*)"\x22\x99", "Ipk=" },
        {2,(const unsigned char*)"\x22\xaa", "Iqo=" },
        {2,(const unsigned char*)"\x22\xbb", "Irs=" },
        {2,(const unsigned char*)"\x22\xcc", "Isw=" },
        {2,(const unsigned char*)"\x22\xdd", "It0=" },
        {2,(const unsigned char*)"\x22\xee", "Iu4=" },
        {2,(const unsigned char*)"\x22\xff", "Iv8=" },
        {2,(const unsigned char*)"\x33\x00", "MwA=" },
        {2,(const unsigned char*)"\x33\x11", "MxE=" },
        {2,(const unsigned char*)"\x33\x22", "MyI=" },
        {2,(const unsigned char*)"\x33\x33", "MzM=" },
        {2,(const unsigned char*)"\x33\x44", "M0Q=" },
        {2,(const unsigned char*)"\x33\x55", "M1U=" },
        {2,(const unsigned char*)"\x33\x66", "M2Y=" },
        {2,(const unsigned char*)"\x33\x77", "M3c=" },
        {2,(const unsigned char*)"\x33\x88", "M4g=" },
        {2,(const unsigned char*)"\x33\x99", "M5k=" },
        {2,(const unsigned char*)"\x33\xaa", "M6o=" },
        {2,(const unsigned char*)"\x33\xbb", "M7s=" },
        {2,(const unsigned char*)"\x33\xcc", "M8w=" },
        {2,(const unsigned char*)"\x33\xdd", "M90=" },
        {2,(const unsigned char*)"\x33\xee", "M+4=" },
        {2,(const unsigned char*)"\x33\xff", "M/8=" },
        {2,(const unsigned char*)"\x44\x00", "RAA=" },
        {2,(const unsigned char*)"\x44\x11", "RBE=" },
        {2,(const unsigned char*)"\x44\x22", "RCI=" },
        {2,(const unsigned char*)"\x44\x33", "RDM=" },
        {2,(const unsigned char*)"\x44\x44", "REQ=" },
        {2,(const unsigned char*)"\x44\x55", "RFU=" },
        {2,(const unsigned char*)"\x44\x66", "RGY=" },
        {2,(const unsigned char*)"\x44\x77", "RHc=" },
        {2,(const unsigned char*)"\x44\x88", "RIg=" },
        {2,(const unsigned char*)"\x44\x99", "RJk=" },
        {2,(const unsigned char*)"\x44\xaa", "RKo=" },
        {2,(const unsigned char*)"\x44\xbb", "RLs=" },
        {2,(const unsigned char*)"\x44\xcc", "RMw=" },
        {2,(const unsigned char*)"\x44\xdd", "RN0=" },
        {2,(const unsigned char*)"\x44\xee", "RO4=" },
        {2,(const unsigned char*)"\x44\xff", "RP8=" },
        {2,(const unsigned char*)"\x55\x00", "VQA=" },
        {2,(const unsigned char*)"\x55\x11", "VRE=" },
        {2,(const unsigned char*)"\x55\x22", "VSI=" },
        {2,(const unsigned char*)"\x55\x33", "VTM=" },
        {2,(const unsigned char*)"\x55\x44", "VUQ=" },
        {2,(const unsigned char*)"\x55\x55", "VVU=" },
        {2,(const unsigned char*)"\x55\x66", "VWY=" },
        {2,(const unsigned char*)"\x55\x77", "VXc=" },
        {2,(const unsigned char*)"\x55\x88", "VYg=" },
        {2,(const unsigned char*)"\x55\x99", "VZk=" },
        {2,(const unsigned char*)"\x55\xaa", "Vao=" },
        {2,(const unsigned char*)"\x55\xbb", "Vbs=" },
        {2,(const unsigned char*)"\x55\xcc", "Vcw=" },
        {2,(const unsigned char*)"\x55\xdd", "Vd0=" },
        {2,(const unsigned char*)"\x55\xee", "Ve4=" },
        {2,(const unsigned char*)"\x55\xff", "Vf8=" },
        {2,(const unsigned char*)"\x66\x00", "ZgA=" },
        {2,(const unsigned char*)"\x66\x11", "ZhE=" },
        {2,(const unsigned char*)"\x66\x22", "ZiI=" },
        {2,(const unsigned char*)"\x66\x33", "ZjM=" },
        {2,(const unsigned char*)"\x66\x44", "ZkQ=" },
        {2,(const unsigned char*)"\x66\x55", "ZlU=" },
        {2,(const unsigned char*)"\x66\x66", "ZmY=" },
        {2,(const unsigned char*)"\x66\x77", "Znc=" },
        {2,(const unsigned char*)"\x66\x88", "Zog=" },
        {2,(const unsigned char*)"\x66\x99", "Zpk=" },
        {2,(const unsigned char*)"\x66\xaa", "Zqo=" },
        {2,(const unsigned char*)"\x66\xbb", "Zrs=" },
        {2,(const unsigned char*)"\x66\xcc", "Zsw=" },
        {2,(const unsigned char*)"\x66\xdd", "Zt0=" },
        {2,(const unsigned char*)"\x66\xee", "Zu4=" },
        {2,(const unsigned char*)"\x66\xff", "Zv8=" },
        {2,(const unsigned char*)"\x77\x00", "dwA=" },
        {2,(const unsigned char*)"\x77\x11", "dxE=" },
        {2,(const unsigned char*)"\x77\x22", "dyI=" },
        {2,(const unsigned char*)"\x77\x33", "dzM=" },
        {2,(const unsigned char*)"\x77\x44", "d0Q=" },
        {2,(const unsigned char*)"\x77\x55", "d1U=" },
        {2,(const unsigned char*)"\x77\x66", "d2Y=" },
        {2,(const unsigned char*)"\x77\x77", "d3c=" },
        {2,(const unsigned char*)"\x77\x88", "d4g=" },
        {2,(const unsigned char*)"\x77\x99", "d5k=" },
        {2,(const unsigned char*)"\x77\xaa", "d6o=" },
        {2,(const unsigned char*)"\x77\xbb", "d7s=" },
        {2,(const unsigned char*)"\x77\xcc", "d8w=" },
        {2,(const unsigned char*)"\x77\xdd", "d90=" },
        {2,(const unsigned char*)"\x77\xee", "d+4=" },
        {2,(const unsigned char*)"\x77\xff", "d/8=" },
        {2,(const unsigned char*)"\x88\x00", "iAA=" },
        {2,(const unsigned char*)"\x88\x11", "iBE=" },
        {2,(const unsigned char*)"\x88\x22", "iCI=" },
        {2,(const unsigned char*)"\x88\x33", "iDM=" },
        {2,(const unsigned char*)"\x88\x44", "iEQ=" },
        {2,(const unsigned char*)"\x88\x55", "iFU=" },
        {2,(const unsigned char*)"\x88\x66", "iGY=" },
        {2,(const unsigned char*)"\x88\x77", "iHc=" },
        {2,(const unsigned char*)"\x88\x88", "iIg=" },
        {2,(const unsigned char*)"\x88\x99", "iJk=" },
        {2,(const unsigned char*)"\x88\xaa", "iKo=" },
        {2,(const unsigned char*)"\x88\xbb", "iLs=" },
        {2,(const unsigned char*)"\x88\xcc", "iMw=" },
        {2,(const unsigned char*)"\x88\xdd", "iN0=" },
        {2,(const unsigned char*)"\x88\xee", "iO4=" },
        {2,(const unsigned char*)"\x88\xff", "iP8=" },
        {2,(const unsigned char*)"\x99\x00", "mQA=" },
        {2,(const unsigned char*)"\x99\x11", "mRE=" },
        {2,(const unsigned char*)"\x99\x22", "mSI=" },
        {2,(const unsigned char*)"\x99\x33", "mTM=" },
        {2,(const unsigned char*)"\x99\x44", "mUQ=" },
        {2,(const unsigned char*)"\x99\x55", "mVU=" },
        {2,(const unsigned char*)"\x99\x66", "mWY=" },
        {2,(const unsigned char*)"\x99\x77", "mXc=" },
        {2,(const unsigned char*)"\x99\x88", "mYg=" },
        {2,(const unsigned char*)"\x99\x99", "mZk=" },
        {2,(const unsigned char*)"\x99\xaa", "mao=" },
        {2,(const unsigned char*)"\x99\xbb", "mbs=" },
        {2,(const unsigned char*)"\x99\xcc", "mcw=" },
        {2,(const unsigned char*)"\x99\xdd", "md0=" },
        {2,(const unsigned char*)"\x99\xee", "me4=" },
        {2,(const unsigned char*)"\x99\xff", "mf8=" },
        {2,(const unsigned char*)"\xaa\x00", "qgA=" },
        {2,(const unsigned char*)"\xaa\x11", "qhE=" },
        {2,(const unsigned char*)"\xaa\x22", "qiI=" },
        {2,(const unsigned char*)"\xaa\x33", "qjM=" },
        {2,(const unsigned char*)"\xaa\x44", "qkQ=" },
        {2,(const unsigned char*)"\xaa\x55", "qlU=" },
        {2,(const unsigned char*)"\xaa\x66", "qmY=" },
        {2,(const unsigned char*)"\xaa\x77", "qnc=" },
        {2,(const unsigned char*)"\xaa\x88", "qog=" },
        {2,(const unsigned char*)"\xaa\x99", "qpk=" },
        {2,(const unsigned char*)"\xaa\xaa", "qqo=" },
        {2,(const unsigned char*)"\xaa\xbb", "qrs=" },
        {2,(const unsigned char*)"\xaa\xcc", "qsw=" },
        {2,(const unsigned char*)"\xaa\xdd", "qt0=" },
        {2,(const unsigned char*)"\xaa\xee", "qu4=" },
        {2,(const unsigned char*)"\xaa\xff", "qv8=" },
        {2,(const unsigned char*)"\xbb\x00", "uwA=" },
        {2,(const unsigned char*)"\xbb\x11", "uxE=" },
        {2,(const unsigned char*)"\xbb\x22", "uyI=" },
        {2,(const unsigned char*)"\xbb\x33", "uzM=" },
        {2,(const unsigned char*)"\xbb\x44", "u0Q=" },
        {2,(const unsigned char*)"\xbb\x55", "u1U=" },
        {2,(const unsigned char*)"\xbb\x66", "u2Y=" },
        {2,(const unsigned char*)"\xbb\x77", "u3c=" },
        {2,(const unsigned char*)"\xbb\x88", "u4g=" },
        {2,(const unsigned char*)"\xbb\x99", "u5k=" },
        {2,(const unsigned char*)"\xbb\xaa", "u6o=" },
        {2,(const unsigned char*)"\xbb\xbb", "u7s=" },
        {2,(const unsigned char*)"\xbb\xcc", "u8w=" },
        {2,(const unsigned char*)"\xbb\xdd", "u90=" },
        {2,(const unsigned char*)"\xbb\xee", "u+4=" },
        {2,(const unsigned char*)"\xbb\xff", "u/8=" },
        {2,(const unsigned char*)"\xcc\x00", "zAA=" },
        {2,(const unsigned char*)"\xcc\x11", "zBE=" },
        {2,(const unsigned char*)"\xcc\x22", "zCI=" },
        {2,(const unsigned char*)"\xcc\x33", "zDM=" },
        {2,(const unsigned char*)"\xcc\x44", "zEQ=" },
        {2,(const unsigned char*)"\xcc\x55", "zFU=" },
        {2,(const unsigned char*)"\xcc\x66", "zGY=" },
        {2,(const unsigned char*)"\xcc\x77", "zHc=" },
        {2,(const unsigned char*)"\xcc\x88", "zIg=" },
        {2,(const unsigned char*)"\xcc\x99", "zJk=" },
        {2,(const unsigned char*)"\xcc\xaa", "zKo=" },
        {2,(const unsigned char*)"\xcc\xbb", "zLs=" },
        {2,(const unsigned char*)"\xcc\xcc", "zMw=" },
        {2,(const unsigned char*)"\xcc\xdd", "zN0=" },
        {2,(const unsigned char*)"\xcc\xee", "zO4=" },
        {2,(const unsigned char*)"\xcc\xff", "zP8=" },
        {2,(const unsigned char*)"\xdd\x00", "3QA=" },
        {2,(const unsigned char*)"\xdd\x11", "3RE=" },
        {2,(const unsigned char*)"\xdd\x22", "3SI=" },
        {2,(const unsigned char*)"\xdd\x33", "3TM=" },
        {2,(const unsigned char*)"\xdd\x44", "3UQ=" },
        {2,(const unsigned char*)"\xdd\x55", "3VU=" },
        {2,(const unsigned char*)"\xdd\x66", "3WY=" },
        {2,(const unsigned char*)"\xdd\x77", "3Xc=" },
        {2,(const unsigned char*)"\xdd\x88", "3Yg=" },
        {2,(const unsigned char*)"\xdd\x99", "3Zk=" },
        {2,(const unsigned char*)"\xdd\xaa", "3ao=" },
        {2,(const unsigned char*)"\xdd\xbb", "3bs=" },
        {2,(const unsigned char*)"\xdd\xcc", "3cw=" },
        {2,(const unsigned char*)"\xdd\xdd", "3d0=" },
        {2,(const unsigned char*)"\xdd\xee", "3e4=" },
        {2,(const unsigned char*)"\xdd\xff", "3f8=" },
        {2,(const unsigned char*)"\xee\x00", "7gA=" },
        {2,(const unsigned char*)"\xee\x11", "7hE=" },
        {2,(const unsigned char*)"\xee\x22", "7iI=" },
        {2,(const unsigned char*)"\xee\x33", "7jM=" },
        {2,(const unsigned char*)"\xee\x44", "7kQ=" },
        {2,(const unsigned char*)"\xee\x55", "7lU=" },
        {2,(const unsigned char*)"\xee\x66", "7mY=" },
        {2,(const unsigned char*)"\xee\x77", "7nc=" },
        {2,(const unsigned char*)"\xee\x88", "7og=" },
        {2,(const unsigned char*)"\xee\x99", "7pk=" },
        {2,(const unsigned char*)"\xee\xaa", "7qo=" },
        {2,(const unsigned char*)"\xee\xbb", "7rs=" },
        {2,(const unsigned char*)"\xee\xcc", "7sw=" },
        {2,(const unsigned char*)"\xee\xdd", "7t0=" },
        {2,(const unsigned char*)"\xee\xee", "7u4=" },
        {2,(const unsigned char*)"\xee\xff", "7v8=" },
        {2,(const unsigned char*)"\xff\x00", "/wA=" },
        {2,(const unsigned char*)"\xff\x11", "/xE=" },
        {2,(const unsigned char*)"\xff\x22", "/yI=" },
        {2,(const unsigned char*)"\xff\x33", "/zM=" },
        {2,(const unsigned char*)"\xff\x44", "/0Q=" },
        {2,(const unsigned char*)"\xff\x55", "/1U=" },
        {2,(const unsigned char*)"\xff\x66", "/2Y=" },
        {2,(const unsigned char*)"\xff\x77", "/3c=" },
        {2,(const unsigned char*)"\xff\x88", "/4g=" },
        {2,(const unsigned char*)"\xff\x99", "/5k=" },
        {2,(const unsigned char*)"\xff\xaa", "/6o=" },
        {2,(const unsigned char*)"\xff\xbb", "/7s=" },
        {2,(const unsigned char*)"\xff\xcc", "/8w=" },
        {2,(const unsigned char*)"\xff\xdd", "/90=" },
        {2,(const unsigned char*)"\xff\xee", "/+4=" },
        {2,(const unsigned char*)"\xff\xff", "//8=" },
        {3,(const unsigned char*)"\x00\x00\x00", "AAAA" },
        {3,(const unsigned char*)"\x00\x00\x2f", "AAAv" },
        {3,(const unsigned char*)"\x00\x00\x5e", "AABe" },
        {3,(const unsigned char*)"\x00\x00\x8d", "AACN" },
        {3,(const unsigned char*)"\x00\x00\xbc", "AAC8" },
        {3,(const unsigned char*)"\x00\x00\xeb", "AADr" },
        {3,(const unsigned char*)"\x00\x2f\x00", "AC8A" },
        {3,(const unsigned char*)"\x00\x2f\x2f", "AC8v" },
        {3,(const unsigned char*)"\x00\x2f\x5e", "AC9e" },
        {3,(const unsigned char*)"\x00\x2f\x8d", "AC+N" },
        {3,(const unsigned char*)"\x00\x2f\xbc", "AC+8" },
        {3,(const unsigned char*)"\x00\x2f\xeb", "AC/r" },
        {3,(const unsigned char*)"\x00\x5e\x00", "AF4A" },
        {3,(const unsigned char*)"\x00\x5e\x2f", "AF4v" },
        {3,(const unsigned char*)"\x00\x5e\x5e", "AF5e" },
        {3,(const unsigned char*)"\x00\x5e\x8d", "AF6N" },
        {3,(const unsigned char*)"\x00\x5e\xbc", "AF68" },
        {3,(const unsigned char*)"\x00\x5e\xeb", "AF7r" },
        {3,(const unsigned char*)"\x00\x8d\x00", "AI0A" },
        {3,(const unsigned char*)"\x00\x8d\x2f", "AI0v" },
        {3,(const unsigned char*)"\x00\x8d\x5e", "AI1e" },
        {3,(const unsigned char*)"\x00\x8d\x8d", "AI2N" },
        {3,(const unsigned char*)"\x00\x8d\xbc", "AI28" },
        {3,(const unsigned char*)"\x00\x8d\xeb", "AI3r" },
        {3,(const unsigned char*)"\x00\xbc\x00", "ALwA" },
        {3,(const unsigned char*)"\x00\xbc\x2f", "ALwv" },
        {3,(const unsigned char*)"\x00\xbc\x5e", "ALxe" },
        {3,(const unsigned char*)"\x00\xbc\x8d", "ALyN" },
        {3,(const unsigned char*)"\x00\xbc\xbc", "ALy8" },
        {3,(const unsigned char*)"\x00\xbc\xeb", "ALzr" },
        {3,(const unsigned char*)"\x00\xeb\x00", "AOsA" },
        {3,(const unsigned char*)"\x00\xeb\x2f", "AOsv" },
        {3,(const unsigned char*)"\x00\xeb\x5e", "AOte" },
        {3,(const unsigned char*)"\x00\xeb\x8d", "AOuN" },
        {3,(const unsigned char*)"\x00\xeb\xbc", "AOu8" },
        {3,(const unsigned char*)"\x00\xeb\xeb", "AOvr" },
        {3,(const unsigned char*)"\x2f\x00\x00", "LwAA" },
        {3,(const unsigned char*)"\x2f\x00\x2f", "LwAv" },
        {3,(const unsigned char*)"\x2f\x00\x5e", "LwBe" },
        {3,(const unsigned char*)"\x2f\x00\x8d", "LwCN" },
        {3,(const unsigned char*)"\x2f\x00\xbc", "LwC8" },
        {3,(const unsigned char*)"\x2f\x00\xeb", "LwDr" },
        {3,(const unsigned char*)"\x2f\x2f\x00", "Ly8A" },
        {3,(const unsigned char*)"\x2f\x2f\x2f", "Ly8v" },
        {3,(const unsigned char*)"\x2f\x2f\x5e", "Ly9e" },
        {3,(const unsigned char*)"\x2f\x2f\x8d", "Ly+N" },
        {3,(const unsigned char*)"\x2f\x2f\xbc", "Ly+8" },
        {3,(const unsigned char*)"\x2f\x2f\xeb", "Ly/r" },
        {3,(const unsigned char*)"\x2f\x5e\x00", "L14A" },
        {3,(const unsigned char*)"\x2f\x5e\x2f", "L14v" },
        {3,(const unsigned char*)"\x2f\x5e\x5e", "L15e" },
        {3,(const unsigned char*)"\x2f\x5e\x8d", "L16N" },
        {3,(const unsigned char*)"\x2f\x5e\xbc", "L168" },
        {3,(const unsigned char*)"\x2f\x5e\xeb", "L17r" },
        {3,(const unsigned char*)"\x2f\x8d\x00", "L40A" },
        {3,(const unsigned char*)"\x2f\x8d\x2f", "L40v" },
        {3,(const unsigned char*)"\x2f\x8d\x5e", "L41e" },
        {3,(const unsigned char*)"\x2f\x8d\x8d", "L42N" },
        {3,(const unsigned char*)"\x2f\x8d\xbc", "L428" },
        {3,(const unsigned char*)"\x2f\x8d\xeb", "L43r" },
        {3,(const unsigned char*)"\x2f\xbc\x00", "L7wA" },
        {3,(const unsigned char*)"\x2f\xbc\x2f", "L7wv" },
        {3,(const unsigned char*)"\x2f\xbc\x5e", "L7xe" },
        {3,(const unsigned char*)"\x2f\xbc\x8d", "L7yN" },
        {3,(const unsigned char*)"\x2f\xbc\xbc", "L7y8" },
        {3,(const unsigned char*)"\x2f\xbc\xeb", "L7zr" },
        {3,(const unsigned char*)"\x2f\xeb\x00", "L+sA" },
        {3,(const unsigned char*)"\x2f\xeb\x2f", "L+sv" },
        {3,(const unsigned char*)"\x2f\xeb\x5e", "L+te" },
        {3,(const unsigned char*)"\x2f\xeb\x8d", "L+uN" },
        {3,(const unsigned char*)"\x2f\xeb\xbc", "L+u8" },
        {3,(const unsigned char*)"\x2f\xeb\xeb", "L+vr" },
        {3,(const unsigned char*)"\x5e\x00\x00", "XgAA" },
        {3,(const unsigned char*)"\x5e\x00\x2f", "XgAv" },
        {3,(const unsigned char*)"\x5e\x00\x5e", "XgBe" },
        {3,(const unsigned char*)"\x5e\x00\x8d", "XgCN" },
        {3,(const unsigned char*)"\x5e\x00\xbc", "XgC8" },
        {3,(const unsigned char*)"\x5e\x00\xeb", "XgDr" },
        {3,(const unsigned char*)"\x5e\x2f\x00", "Xi8A" },
        {3,(const unsigned char*)"\x5e\x2f\x2f", "Xi8v" },
        {3,(const unsigned char*)"\x5e\x2f\x5e", "Xi9e" },
        {3,(const unsigned char*)"\x5e\x2f\x8d", "Xi+N" },
        {3,(const unsigned char*)"\x5e\x2f\xbc", "Xi+8" },
        {3,(const unsigned char*)"\x5e\x2f\xeb", "Xi/r" },
        {3,(const unsigned char*)"\x5e\x5e\x00", "Xl4A" },
        {3,(const unsigned char*)"\x5e\x5e\x2f", "Xl4v" },
        {3,(const unsigned char*)"\x5e\x5e\x5e", "Xl5e" },
        {3,(const unsigned char*)"\x5e\x5e\x8d", "Xl6N" },
        {3,(const unsigned char*)"\x5e\x5e\xbc", "Xl68" },
        {3,(const unsigned char*)"\x5e\x5e\xeb", "Xl7r" },
        {3,(const unsigned char*)"\x5e\x8d\x00", "Xo0A" },
        {3,(const unsigned char*)"\x5e\x8d\x2f", "Xo0v" },
        {3,(const unsigned char*)"\x5e\x8d\x5e", "Xo1e" },
        {3,(const unsigned char*)"\x5e\x8d\x8d", "Xo2N" },
        {3,(const unsigned char*)"\x5e\x8d\xbc", "Xo28" },
        {3,(const unsigned char*)"\x5e\x8d\xeb", "Xo3r" },
        {3,(const unsigned char*)"\x5e\xbc\x00", "XrwA" },
        {3,(const unsigned char*)"\x5e\xbc\x2f", "Xrwv" },
        {3,(const unsigned char*)"\x5e\xbc\x5e", "Xrxe" },
        {3,(const unsigned char*)"\x5e\xbc\x8d", "XryN" },
        {3,(const unsigned char*)"\x5e\xbc\xbc", "Xry8" },
        {3,(const unsigned char*)"\x5e\xbc\xeb", "Xrzr" },
        {3,(const unsigned char*)"\x5e\xeb\x00", "XusA" },
        {3,(const unsigned char*)"\x5e\xeb\x2f", "Xusv" },
        {3,(const unsigned char*)"\x5e\xeb\x5e", "Xute" },
        {3,(const unsigned char*)"\x5e\xeb\x8d", "XuuN" },
        {3,(const unsigned char*)"\x5e\xeb\xbc", "Xuu8" },
        {3,(const unsigned char*)"\x5e\xeb\xeb", "Xuvr" },
        {3,(const unsigned char*)"\x8d\x00\x00", "jQAA" },
        {3,(const unsigned char*)"\x8d\x00\x2f", "jQAv" },
        {3,(const unsigned char*)"\x8d\x00\x5e", "jQBe" },
        {3,(const unsigned char*)"\x8d\x00\x8d", "jQCN" },
        {3,(const unsigned char*)"\x8d\x00\xbc", "jQC8" },
        {3,(const unsigned char*)"\x8d\x00\xeb", "jQDr" },
        {3,(const unsigned char*)"\x8d\x2f\x00", "jS8A" },
        {3,(const unsigned char*)"\x8d\x2f\x2f", "jS8v" },
        {3,(const unsigned char*)"\x8d\x2f\x5e", "jS9e" },
        {3,(const unsigned char*)"\x8d\x2f\x8d", "jS+N" },
        {3,(const unsigned char*)"\x8d\x2f\xbc", "jS+8" },
        {3,(const unsigned char*)"\x8d\x2f\xeb", "jS/r" },
        {3,(const unsigned char*)"\x8d\x5e\x00", "jV4A" },
        {3,(const unsigned char*)"\x8d\x5e\x2f", "jV4v" },
        {3,(const unsigned char*)"\x8d\x5e\x5e", "jV5e" },
        {3,(const unsigned char*)"\x8d\x5e\x8d", "jV6N" },
        {3,(const unsigned char*)"\x8d\x5e\xbc", "jV68" },
        {3,(const unsigned char*)"\x8d\x5e\xeb", "jV7r" },
        {3,(const unsigned char*)"\x8d\x8d\x00", "jY0A" },
        {3,(const unsigned char*)"\x8d\x8d\x2f", "jY0v" },
        {3,(const unsigned char*)"\x8d\x8d\x5e", "jY1e" },
        {3,(const unsigned char*)"\x8d\x8d\x8d", "jY2N" },
        {3,(const unsigned char*)"\x8d\x8d\xbc", "jY28" },
        {3,(const unsigned char*)"\x8d\x8d\xeb", "jY3r" },
        {3,(const unsigned char*)"\x8d\xbc\x00", "jbwA" },
        {3,(const unsigned char*)"\x8d\xbc\x2f", "jbwv" },
        {3,(const unsigned char*)"\x8d\xbc\x5e", "jbxe" },
        {3,(const unsigned char*)"\x8d\xbc\x8d", "jbyN" },
        {3,(const unsigned char*)"\x8d\xbc\xbc", "jby8" },
        {3,(const unsigned char*)"\x8d\xbc\xeb", "jbzr" },
        {3,(const unsigned char*)"\x8d\xeb\x00", "jesA" },
        {3,(const unsigned char*)"\x8d\xeb\x2f", "jesv" },
        {3,(const unsigned char*)"\x8d\xeb\x5e", "jete" },
        {3,(const unsigned char*)"\x8d\xeb\x8d", "jeuN" },
        {3,(const unsigned char*)"\x8d\xeb\xbc", "jeu8" },
        {3,(const unsigned char*)"\x8d\xeb\xeb", "jevr" },
        {3,(const unsigned char*)"\xbc\x00\x00", "vAAA" },
        {3,(const unsigned char*)"\xbc\x00\x2f", "vAAv" },
        {3,(const unsigned char*)"\xbc\x00\x5e", "vABe" },
        {3,(const unsigned char*)"\xbc\x00\x8d", "vACN" },
        {3,(const unsigned char*)"\xbc\x00\xbc", "vAC8" },
        {3,(const unsigned char*)"\xbc\x00\xeb", "vADr" },
        {3,(const unsigned char*)"\xbc\x2f\x00", "vC8A" },
        {3,(const unsigned char*)"\xbc\x2f\x2f", "vC8v" },
        {3,(const unsigned char*)"\xbc\x2f\x5e", "vC9e" },
        {3,(const unsigned char*)"\xbc\x2f\x8d", "vC+N" },
        {3,(const unsigned char*)"\xbc\x2f\xbc", "vC+8" },
        {3,(const unsigned char*)"\xbc\x2f\xeb", "vC/r" },
        {3,(const unsigned char*)"\xbc\x5e\x00", "vF4A" },
        {3,(const unsigned char*)"\xbc\x5e\x2f", "vF4v" },
        {3,(const unsigned char*)"\xbc\x5e\x5e", "vF5e" },
        {3,(const unsigned char*)"\xbc\x5e\x8d", "vF6N" },
        {3,(const unsigned char*)"\xbc\x5e\xbc", "vF68" },
        {3,(const unsigned char*)"\xbc\x5e\xeb", "vF7r" },
        {3,(const unsigned char*)"\xbc\x8d\x00", "vI0A" },
        {3,(const unsigned char*)"\xbc\x8d\x2f", "vI0v" },
        {3,(const unsigned char*)"\xbc\x8d\x5e", "vI1e" },
        {3,(const unsigned char*)"\xbc\x8d\x8d", "vI2N" },
        {3,(const unsigned char*)"\xbc\x8d\xbc", "vI28" },
        {3,(const unsigned char*)"\xbc\x8d\xeb", "vI3r" },
        {3,(const unsigned char*)"\xbc\xbc\x00", "vLwA" },
        {3,(const unsigned char*)"\xbc\xbc\x2f", "vLwv" },
        {3,(const unsigned char*)"\xbc\xbc\x5e", "vLxe" },
        {3,(const unsigned char*)"\xbc\xbc\x8d", "vLyN" },
        {3,(const unsigned char*)"\xbc\xbc\xbc", "vLy8" },
        {3,(const unsigned char*)"\xbc\xbc\xeb", "vLzr" },
        {3,(const unsigned char*)"\xbc\xeb\x00", "vOsA" },
        {3,(const unsigned char*)"\xbc\xeb\x2f", "vOsv" },
        {3,(const unsigned char*)"\xbc\xeb\x5e", "vOte" },
        {3,(const unsigned char*)"\xbc\xeb\x8d", "vOuN" },
        {3,(const unsigned char*)"\xbc\xeb\xbc", "vOu8" },
        {3,(const unsigned char*)"\xbc\xeb\xeb", "vOvr" },
        {3,(const unsigned char*)"\xeb\x00\x00", "6wAA" },
        {3,(const unsigned char*)"\xeb\x00\x2f", "6wAv" },
        {3,(const unsigned char*)"\xeb\x00\x5e", "6wBe" },
        {3,(const unsigned char*)"\xeb\x00\x8d", "6wCN" },
        {3,(const unsigned char*)"\xeb\x00\xbc", "6wC8" },
        {3,(const unsigned char*)"\xeb\x00\xeb", "6wDr" },
        {3,(const unsigned char*)"\xeb\x2f\x00", "6y8A" },
        {3,(const unsigned char*)"\xeb\x2f\x2f", "6y8v" },
        {3,(const unsigned char*)"\xeb\x2f\x5e", "6y9e" },
        {3,(const unsigned char*)"\xeb\x2f\x8d", "6y+N" },
        {3,(const unsigned char*)"\xeb\x2f\xbc", "6y+8" },
        {3,(const unsigned char*)"\xeb\x2f\xeb", "6y/r" },
        {3,(const unsigned char*)"\xeb\x5e\x00", "614A" },
        {3,(const unsigned char*)"\xeb\x5e\x2f", "614v" },
        {3,(const unsigned char*)"\xeb\x5e\x5e", "615e" },
        {3,(const unsigned char*)"\xeb\x5e\x8d", "616N" },
        {3,(const unsigned char*)"\xeb\x5e\xbc", "6168" },
        {3,(const unsigned char*)"\xeb\x5e\xeb", "617r" },
        {3,(const unsigned char*)"\xeb\x8d\x00", "640A" },
        {3,(const unsigned char*)"\xeb\x8d\x2f", "640v" },
        {3,(const unsigned char*)"\xeb\x8d\x5e", "641e" },
        {3,(const unsigned char*)"\xeb\x8d\x8d", "642N" },
        {3,(const unsigned char*)"\xeb\x8d\xbc", "6428" },
        {3,(const unsigned char*)"\xeb\x8d\xeb", "643r" },
        {3,(const unsigned char*)"\xeb\xbc\x00", "67wA" },
        {3,(const unsigned char*)"\xeb\xbc\x2f", "67wv" },
        {3,(const unsigned char*)"\xeb\xbc\x5e", "67xe" },
        {3,(const unsigned char*)"\xeb\xbc\x8d", "67yN" },
        {3,(const unsigned char*)"\xeb\xbc\xbc", "67y8" },
        {3,(const unsigned char*)"\xeb\xbc\xeb", "67zr" },
        {3,(const unsigned char*)"\xeb\xeb\x00", "6+sA" },
        {3,(const unsigned char*)"\xeb\xeb\x2f", "6+sv" },
        {3,(const unsigned char*)"\xeb\xeb\x5e", "6+te" },
        {3,(const unsigned char*)"\xeb\xeb\x8d", "6+uN" },
        {3,(const unsigned char*)"\xeb\xeb\xbc", "6+u8" },
        {3,(const unsigned char*)"\xeb\xeb\xeb", "6+vr" },
        {4,(const unsigned char*)"\x00\x00\x00\x00", "AAAAAA==" },
        {4,(const unsigned char*)"\x00\x00\x00\x55", "AAAAVQ==" },
        {4,(const unsigned char*)"\x00\x00\x00\xaa", "AAAAqg==" },
        {4,(const unsigned char*)"\x00\x00\x00\xff", "AAAA/w==" },
        {4,(const unsigned char*)"\x00\x00\x55\x00", "AABVAA==" },
        {4,(const unsigned char*)"\x00\x00\x55\x55", "AABVVQ==" },
        {4,(const unsigned char*)"\x00\x00\x55\xaa", "AABVqg==" },
        {4,(const unsigned char*)"\x00\x00\x55\xff", "AABV/w==" },
        {4,(const unsigned char*)"\x00\x00\xaa\x00", "AACqAA==" },
        {4,(const unsigned char*)"\x00\x00\xaa\x55", "AACqVQ==" },
        {4,(const unsigned char*)"\x00\x00\xaa\xaa", "AACqqg==" },
        {4,(const unsigned char*)"\x00\x00\xaa\xff", "AACq/w==" },
        {4,(const unsigned char*)"\x00\x00\xff\x00", "AAD/AA==" },
        {4,(const unsigned char*)"\x00\x00\xff\x55", "AAD/VQ==" },
        {4,(const unsigned char*)"\x00\x00\xff\xaa", "AAD/qg==" },
        {4,(const unsigned char*)"\x00\x00\xff\xff", "AAD//w==" },
        {4,(const unsigned char*)"\x00\x55\x00\x00", "AFUAAA==" },
        {4,(const unsigned char*)"\x00\x55\x00\x55", "AFUAVQ==" },
        {4,(const unsigned char*)"\x00\x55\x00\xaa", "AFUAqg==" },
        {4,(const unsigned char*)"\x00\x55\x00\xff", "AFUA/w==" },
        {4,(const unsigned char*)"\x00\x55\x55\x00", "AFVVAA==" },
        {4,(const unsigned char*)"\x00\x55\x55\x55", "AFVVVQ==" },
        {4,(const unsigned char*)"\x00\x55\x55\xaa", "AFVVqg==" },
        {4,(const unsigned char*)"\x00\x55\x55\xff", "AFVV/w==" },
        {4,(const unsigned char*)"\x00\x55\xaa\x00", "AFWqAA==" },
        {4,(const unsigned char*)"\x00\x55\xaa\x55", "AFWqVQ==" },
        {4,(const unsigned char*)"\x00\x55\xaa\xaa", "AFWqqg==" },
        {4,(const unsigned char*)"\x00\x55\xaa\xff", "AFWq/w==" },
        {4,(const unsigned char*)"\x00\x55\xff\x00", "AFX/AA==" },
        {4,(const unsigned char*)"\x00\x55\xff\x55", "AFX/VQ==" },
        {4,(const unsigned char*)"\x00\x55\xff\xaa", "AFX/qg==" },
        {4,(const unsigned char*)"\x00\x55\xff\xff", "AFX//w==" },
        {4,(const unsigned char*)"\x00\xaa\x00\x00", "AKoAAA==" },
        {4,(const unsigned char*)"\x00\xaa\x00\x55", "AKoAVQ==" },
        {4,(const unsigned char*)"\x00\xaa\x00\xaa", "AKoAqg==" },
        {4,(const unsigned char*)"\x00\xaa\x00\xff", "AKoA/w==" },
        {4,(const unsigned char*)"\x00\xaa\x55\x00", "AKpVAA==" },
        {4,(const unsigned char*)"\x00\xaa\x55\x55", "AKpVVQ==" },
        {4,(const unsigned char*)"\x00\xaa\x55\xaa", "AKpVqg==" },
        {4,(const unsigned char*)"\x00\xaa\x55\xff", "AKpV/w==" },
        {4,(const unsigned char*)"\x00\xaa\xaa\x00", "AKqqAA==" },
        {4,(const unsigned char*)"\x00\xaa\xaa\x55", "AKqqVQ==" },
        {4,(const unsigned char*)"\x00\xaa\xaa\xaa", "AKqqqg==" },
        {4,(const unsigned char*)"\x00\xaa\xaa\xff", "AKqq/w==" },
        {4,(const unsigned char*)"\x00\xaa\xff\x00", "AKr/AA==" },
        {4,(const unsigned char*)"\x00\xaa\xff\x55", "AKr/VQ==" },
        {4,(const unsigned char*)"\x00\xaa\xff\xaa", "AKr/qg==" },
        {4,(const unsigned char*)"\x00\xaa\xff\xff", "AKr//w==" },
        {4,(const unsigned char*)"\x00\xff\x00\x00", "AP8AAA==" },
        {4,(const unsigned char*)"\x00\xff\x00\x55", "AP8AVQ==" },
        {4,(const unsigned char*)"\x00\xff\x00\xaa", "AP8Aqg==" },
        {4,(const unsigned char*)"\x00\xff\x00\xff", "AP8A/w==" },
        {4,(const unsigned char*)"\x00\xff\x55\x00", "AP9VAA==" },
        {4,(const unsigned char*)"\x00\xff\x55\x55", "AP9VVQ==" },
        {4,(const unsigned char*)"\x00\xff\x55\xaa", "AP9Vqg==" },
        {4,(const unsigned char*)"\x00\xff\x55\xff", "AP9V/w==" },
        {4,(const unsigned char*)"\x00\xff\xaa\x00", "AP+qAA==" },
        {4,(const unsigned char*)"\x00\xff\xaa\x55", "AP+qVQ==" },
        {4,(const unsigned char*)"\x00\xff\xaa\xaa", "AP+qqg==" },
        {4,(const unsigned char*)"\x00\xff\xaa\xff", "AP+q/w==" },
        {4,(const unsigned char*)"\x00\xff\xff\x00", "AP//AA==" },
        {4,(const unsigned char*)"\x00\xff\xff\x55", "AP//VQ==" },
        {4,(const unsigned char*)"\x00\xff\xff\xaa", "AP//qg==" },
        {4,(const unsigned char*)"\x00\xff\xff\xff", "AP///w==" },
        {4,(const unsigned char*)"\x55\x00\x00\x00", "VQAAAA==" },
        {4,(const unsigned char*)"\x55\x00\x00\x55", "VQAAVQ==" },
        {4,(const unsigned char*)"\x55\x00\x00\xaa", "VQAAqg==" },
        {4,(const unsigned char*)"\x55\x00\x00\xff", "VQAA/w==" },
        {4,(const unsigned char*)"\x55\x00\x55\x00", "VQBVAA==" },
        {4,(const unsigned char*)"\x55\x00\x55\x55", "VQBVVQ==" },
        {4,(const unsigned char*)"\x55\x00\x55\xaa", "VQBVqg==" },
        {4,(const unsigned char*)"\x55\x00\x55\xff", "VQBV/w==" },
        {4,(const unsigned char*)"\x55\x00\xaa\x00", "VQCqAA==" },
        {4,(const unsigned char*)"\x55\x00\xaa\x55", "VQCqVQ==" },
        {4,(const unsigned char*)"\x55\x00\xaa\xaa", "VQCqqg==" },
        {4,(const unsigned char*)"\x55\x00\xaa\xff", "VQCq/w==" },
        {4,(const unsigned char*)"\x55\x00\xff\x00", "VQD/AA==" },
        {4,(const unsigned char*)"\x55\x00\xff\x55", "VQD/VQ==" },
        {4,(const unsigned char*)"\x55\x00\xff\xaa", "VQD/qg==" },
        {4,(const unsigned char*)"\x55\x00\xff\xff", "VQD//w==" },
        {4,(const unsigned char*)"\x55\x55\x00\x00", "VVUAAA==" },
        {4,(const unsigned char*)"\x55\x55\x00\x55", "VVUAVQ==" },
        {4,(const unsigned char*)"\x55\x55\x00\xaa", "VVUAqg==" },
        {4,(const unsigned char*)"\x55\x55\x00\xff", "VVUA/w==" },
        {4,(const unsigned char*)"\x55\x55\x55\x00", "VVVVAA==" },
        {4,(const unsigned char*)"\x55\x55\x55\x55", "VVVVVQ==" },
        {4,(const unsigned char*)"\x55\x55\x55\xaa", "VVVVqg==" },
        {4,(const unsigned char*)"\x55\x55\x55\xff", "VVVV/w==" },
        {4,(const unsigned char*)"\x55\x55\xaa\x00", "VVWqAA==" },
        {4,(const unsigned char*)"\x55\x55\xaa\x55", "VVWqVQ==" },
        {4,(const unsigned char*)"\x55\x55\xaa\xaa", "VVWqqg==" },
        {4,(const unsigned char*)"\x55\x55\xaa\xff", "VVWq/w==" },
        {4,(const unsigned char*)"\x55\x55\xff\x00", "VVX/AA==" },
        {4,(const unsigned char*)"\x55\x55\xff\x55", "VVX/VQ==" },
        {4,(const unsigned char*)"\x55\x55\xff\xaa", "VVX/qg==" },
        {4,(const unsigned char*)"\x55\x55\xff\xff", "VVX//w==" },
        {4,(const unsigned char*)"\x55\xaa\x00\x00", "VaoAAA==" },
        {4,(const unsigned char*)"\x55\xaa\x00\x55", "VaoAVQ==" },
        {4,(const unsigned char*)"\x55\xaa\x00\xaa", "VaoAqg==" },
        {4,(const unsigned char*)"\x55\xaa\x00\xff", "VaoA/w==" },
        {4,(const unsigned char*)"\x55\xaa\x55\x00", "VapVAA==" },
        {4,(const unsigned char*)"\x55\xaa\x55\x55", "VapVVQ==" },
        {4,(const unsigned char*)"\x55\xaa\x55\xaa", "VapVqg==" },
        {4,(const unsigned char*)"\x55\xaa\x55\xff", "VapV/w==" },
        {4,(const unsigned char*)"\x55\xaa\xaa\x00", "VaqqAA==" },
        {4,(const unsigned char*)"\x55\xaa\xaa\x55", "VaqqVQ==" },
        {4,(const unsigned char*)"\x55\xaa\xaa\xaa", "Vaqqqg==" },
        {4,(const unsigned char*)"\x55\xaa\xaa\xff", "Vaqq/w==" },
        {4,(const unsigned char*)"\x55\xaa\xff\x00", "Var/AA==" },
        {4,(const unsigned char*)"\x55\xaa\xff\x55", "Var/VQ==" },
        {4,(const unsigned char*)"\x55\xaa\xff\xaa", "Var/qg==" },
        {4,(const unsigned char*)"\x55\xaa\xff\xff", "Var//w==" },
        {4,(const unsigned char*)"\x55\xff\x00\x00", "Vf8AAA==" },
        {4,(const unsigned char*)"\x55\xff\x00\x55", "Vf8AVQ==" },
        {4,(const unsigned char*)"\x55\xff\x00\xaa", "Vf8Aqg==" },
        {4,(const unsigned char*)"\x55\xff\x00\xff", "Vf8A/w==" },
        {4,(const unsigned char*)"\x55\xff\x55\x00", "Vf9VAA==" },
        {4,(const unsigned char*)"\x55\xff\x55\x55", "Vf9VVQ==" },
        {4,(const unsigned char*)"\x55\xff\x55\xaa", "Vf9Vqg==" },
        {4,(const unsigned char*)"\x55\xff\x55\xff", "Vf9V/w==" },
        {4,(const unsigned char*)"\x55\xff\xaa\x00", "Vf+qAA==" },
        {4,(const unsigned char*)"\x55\xff\xaa\x55", "Vf+qVQ==" },
        {4,(const unsigned char*)"\x55\xff\xaa\xaa", "Vf+qqg==" },
        {4,(const unsigned char*)"\x55\xff\xaa\xff", "Vf+q/w==" },
        {4,(const unsigned char*)"\x55\xff\xff\x00", "Vf//AA==" },
        {4,(const unsigned char*)"\x55\xff\xff\x55", "Vf//VQ==" },
        {4,(const unsigned char*)"\x55\xff\xff\xaa", "Vf//qg==" },
        {4,(const unsigned char*)"\x55\xff\xff\xff", "Vf///w==" },
        {4,(const unsigned char*)"\xaa\x00\x00\x00", "qgAAAA==" },
        {4,(const unsigned char*)"\xaa\x00\x00\x55", "qgAAVQ==" },
        {4,(const unsigned char*)"\xaa\x00\x00\xaa", "qgAAqg==" },
        {4,(const unsigned char*)"\xaa\x00\x00\xff", "qgAA/w==" },
        {4,(const unsigned char*)"\xaa\x00\x55\x00", "qgBVAA==" },
        {4,(const unsigned char*)"\xaa\x00\x55\x55", "qgBVVQ==" },
        {4,(const unsigned char*)"\xaa\x00\x55\xaa", "qgBVqg==" },
        {4,(const unsigned char*)"\xaa\x00\x55\xff", "qgBV/w==" },
        {4,(const unsigned char*)"\xaa\x00\xaa\x00", "qgCqAA==" },
        {4,(const unsigned char*)"\xaa\x00\xaa\x55", "qgCqVQ==" },
        {4,(const unsigned char*)"\xaa\x00\xaa\xaa", "qgCqqg==" },
        {4,(const unsigned char*)"\xaa\x00\xaa\xff", "qgCq/w==" },
        {4,(const unsigned char*)"\xaa\x00\xff\x00", "qgD/AA==" },
        {4,(const unsigned char*)"\xaa\x00\xff\x55", "qgD/VQ==" },
        {4,(const unsigned char*)"\xaa\x00\xff\xaa", "qgD/qg==" },
        {4,(const unsigned char*)"\xaa\x00\xff\xff", "qgD//w==" },
        {4,(const unsigned char*)"\xaa\x55\x00\x00", "qlUAAA==" },
        {4,(const unsigned char*)"\xaa\x55\x00\x55", "qlUAVQ==" },
        {4,(const unsigned char*)"\xaa\x55\x00\xaa", "qlUAqg==" },
        {4,(const unsigned char*)"\xaa\x55\x00\xff", "qlUA/w==" },
        {4,(const unsigned char*)"\xaa\x55\x55\x00", "qlVVAA==" },
        {4,(const unsigned char*)"\xaa\x55\x55\x55", "qlVVVQ==" },
        {4,(const unsigned char*)"\xaa\x55\x55\xaa", "qlVVqg==" },
        {4,(const unsigned char*)"\xaa\x55\x55\xff", "qlVV/w==" },
        {4,(const unsigned char*)"\xaa\x55\xaa\x00", "qlWqAA==" },
        {4,(const unsigned char*)"\xaa\x55\xaa\x55", "qlWqVQ==" },
        {4,(const unsigned char*)"\xaa\x55\xaa\xaa", "qlWqqg==" },
        {4,(const unsigned char*)"\xaa\x55\xaa\xff", "qlWq/w==" },
        {4,(const unsigned char*)"\xaa\x55\xff\x00", "qlX/AA==" },
        {4,(const unsigned char*)"\xaa\x55\xff\x55", "qlX/VQ==" },
        {4,(const unsigned char*)"\xaa\x55\xff\xaa", "qlX/qg==" },
        {4,(const unsigned char*)"\xaa\x55\xff\xff", "qlX//w==" },
        {4,(const unsigned char*)"\xaa\xaa\x00\x00", "qqoAAA==" },
        {4,(const unsigned char*)"\xaa\xaa\x00\x55", "qqoAVQ==" },
        {4,(const unsigned char*)"\xaa\xaa\x00\xaa", "qqoAqg==" },
        {4,(const unsigned char*)"\xaa\xaa\x00\xff", "qqoA/w==" },
        {4,(const unsigned char*)"\xaa\xaa\x55\x00", "qqpVAA==" },
        {4,(const unsigned char*)"\xaa\xaa\x55\x55", "qqpVVQ==" },
        {4,(const unsigned char*)"\xaa\xaa\x55\xaa", "qqpVqg==" },
        {4,(const unsigned char*)"\xaa\xaa\x55\xff", "qqpV/w==" },
        {4,(const unsigned char*)"\xaa\xaa\xaa\x00", "qqqqAA==" },
        {4,(const unsigned char*)"\xaa\xaa\xaa\x55", "qqqqVQ==" },
        {4,(const unsigned char*)"\xaa\xaa\xaa\xaa", "qqqqqg==" },
        {4,(const unsigned char*)"\xaa\xaa\xaa\xff", "qqqq/w==" },
        {4,(const unsigned char*)"\xaa\xaa\xff\x00", "qqr/AA==" },
        {4,(const unsigned char*)"\xaa\xaa\xff\x55", "qqr/VQ==" },
        {4,(const unsigned char*)"\xaa\xaa\xff\xaa", "qqr/qg==" },
        {4,(const unsigned char*)"\xaa\xaa\xff\xff", "qqr//w==" },
        {4,(const unsigned char*)"\xaa\xff\x00\x00", "qv8AAA==" },
        {4,(const unsigned char*)"\xaa\xff\x00\x55", "qv8AVQ==" },
        {4,(const unsigned char*)"\xaa\xff\x00\xaa", "qv8Aqg==" },
        {4,(const unsigned char*)"\xaa\xff\x00\xff", "qv8A/w==" },
        {4,(const unsigned char*)"\xaa\xff\x55\x00", "qv9VAA==" },
        {4,(const unsigned char*)"\xaa\xff\x55\x55", "qv9VVQ==" },
        {4,(const unsigned char*)"\xaa\xff\x55\xaa", "qv9Vqg==" },
        {4,(const unsigned char*)"\xaa\xff\x55\xff", "qv9V/w==" },
        {4,(const unsigned char*)"\xaa\xff\xaa\x00", "qv+qAA==" },
        {4,(const unsigned char*)"\xaa\xff\xaa\x55", "qv+qVQ==" },
        {4,(const unsigned char*)"\xaa\xff\xaa\xaa", "qv+qqg==" },
        {4,(const unsigned char*)"\xaa\xff\xaa\xff", "qv+q/w==" },
        {4,(const unsigned char*)"\xaa\xff\xff\x00", "qv//AA==" },
        {4,(const unsigned char*)"\xaa\xff\xff\x55", "qv//VQ==" },
        {4,(const unsigned char*)"\xaa\xff\xff\xaa", "qv//qg==" },
        {4,(const unsigned char*)"\xaa\xff\xff\xff", "qv///w==" },
        {4,(const unsigned char*)"\xff\x00\x00\x00", "/wAAAA==" },
        {4,(const unsigned char*)"\xff\x00\x00\x55", "/wAAVQ==" },
        {4,(const unsigned char*)"\xff\x00\x00\xaa", "/wAAqg==" },
        {4,(const unsigned char*)"\xff\x00\x00\xff", "/wAA/w==" },
        {4,(const unsigned char*)"\xff\x00\x55\x00", "/wBVAA==" },
        {4,(const unsigned char*)"\xff\x00\x55\x55", "/wBVVQ==" },
        {4,(const unsigned char*)"\xff\x00\x55\xaa", "/wBVqg==" },
        {4,(const unsigned char*)"\xff\x00\x55\xff", "/wBV/w==" },
        {4,(const unsigned char*)"\xff\x00\xaa\x00", "/wCqAA==" },
        {4,(const unsigned char*)"\xff\x00\xaa\x55", "/wCqVQ==" },
        {4,(const unsigned char*)"\xff\x00\xaa\xaa", "/wCqqg==" },
        {4,(const unsigned char*)"\xff\x00\xaa\xff", "/wCq/w==" },
        {4,(const unsigned char*)"\xff\x00\xff\x00", "/wD/AA==" },
        {4,(const unsigned char*)"\xff\x00\xff\x55", "/wD/VQ==" },
        {4,(const unsigned char*)"\xff\x00\xff\xaa", "/wD/qg==" },
        {4,(const unsigned char*)"\xff\x00\xff\xff", "/wD//w==" },
        {4,(const unsigned char*)"\xff\x55\x00\x00", "/1UAAA==" },
        {4,(const unsigned char*)"\xff\x55\x00\x55", "/1UAVQ==" },
        {4,(const unsigned char*)"\xff\x55\x00\xaa", "/1UAqg==" },
        {4,(const unsigned char*)"\xff\x55\x00\xff", "/1UA/w==" },
        {4,(const unsigned char*)"\xff\x55\x55\x00", "/1VVAA==" },
        {4,(const unsigned char*)"\xff\x55\x55\x55", "/1VVVQ==" },
        {4,(const unsigned char*)"\xff\x55\x55\xaa", "/1VVqg==" },
        {4,(const unsigned char*)"\xff\x55\x55\xff", "/1VV/w==" },
        {4,(const unsigned char*)"\xff\x55\xaa\x00", "/1WqAA==" },
        {4,(const unsigned char*)"\xff\x55\xaa\x55", "/1WqVQ==" },
        {4,(const unsigned char*)"\xff\x55\xaa\xaa", "/1Wqqg==" },
        {4,(const unsigned char*)"\xff\x55\xaa\xff", "/1Wq/w==" },
        {4,(const unsigned char*)"\xff\x55\xff\x00", "/1X/AA==" },
        {4,(const unsigned char*)"\xff\x55\xff\x55", "/1X/VQ==" },
        {4,(const unsigned char*)"\xff\x55\xff\xaa", "/1X/qg==" },
        {4,(const unsigned char*)"\xff\x55\xff\xff", "/1X//w==" },
        {4,(const unsigned char*)"\xff\xaa\x00\x00", "/6oAAA==" },
        {4,(const unsigned char*)"\xff\xaa\x00\x55", "/6oAVQ==" },
        {4,(const unsigned char*)"\xff\xaa\x00\xaa", "/6oAqg==" },
        {4,(const unsigned char*)"\xff\xaa\x00\xff", "/6oA/w==" },
        {4,(const unsigned char*)"\xff\xaa\x55\x00", "/6pVAA==" },
        {4,(const unsigned char*)"\xff\xaa\x55\x55", "/6pVVQ==" },
        {4,(const unsigned char*)"\xff\xaa\x55\xaa", "/6pVqg==" },
        {4,(const unsigned char*)"\xff\xaa\x55\xff", "/6pV/w==" },
        {4,(const unsigned char*)"\xff\xaa\xaa\x00", "/6qqAA==" },
        {4,(const unsigned char*)"\xff\xaa\xaa\x55", "/6qqVQ==" },
        {4,(const unsigned char*)"\xff\xaa\xaa\xaa", "/6qqqg==" },
        {4,(const unsigned char*)"\xff\xaa\xaa\xff", "/6qq/w==" },
        {4,(const unsigned char*)"\xff\xaa\xff\x00", "/6r/AA==" },
        {4,(const unsigned char*)"\xff\xaa\xff\x55", "/6r/VQ==" },
        {4,(const unsigned char*)"\xff\xaa\xff\xaa", "/6r/qg==" },
        {4,(const unsigned char*)"\xff\xaa\xff\xff", "/6r//w==" },
        {4,(const unsigned char*)"\xff\xff\x00\x00", "//8AAA==" },
        {4,(const unsigned char*)"\xff\xff\x00\x55", "//8AVQ==" },
        {4,(const unsigned char*)"\xff\xff\x00\xaa", "//8Aqg==" },
        {4,(const unsigned char*)"\xff\xff\x00\xff", "//8A/w==" },
        {4,(const unsigned char*)"\xff\xff\x55\x00", "//9VAA==" },
        {4,(const unsigned char*)"\xff\xff\x55\x55", "//9VVQ==" },
        {4,(const unsigned char*)"\xff\xff\x55\xaa", "//9Vqg==" },
        {4,(const unsigned char*)"\xff\xff\x55\xff", "//9V/w==" },
        {4,(const unsigned char*)"\xff\xff\xaa\x00", "//+qAA==" },
        {4,(const unsigned char*)"\xff\xff\xaa\x55", "//+qVQ==" },
        {4,(const unsigned char*)"\xff\xff\xaa\xaa", "//+qqg==" },
        {4,(const unsigned char*)"\xff\xff\xaa\xff", "//+q/w==" },
        {4,(const unsigned char*)"\xff\xff\xff\x00", "////AA==" },
        {4,(const unsigned char*)"\xff\xff\xff\x55", "////VQ==" },
        {4,(const unsigned char*)"\xff\xff\xff\xaa", "////qg==" },
        {4,(const unsigned char*)"\xff\xff\xff\xff", "/////w==" },
        {5,(const unsigned char*)"\x00\x00\x00\x00\x00", "AAAAAAA=" },
        {5,(const unsigned char*)"\x00\x00\x00\x00\x7e", "AAAAAH4=" },
        {5,(const unsigned char*)"\x00\x00\x00\x00\xfc", "AAAAAPw=" },
        {5,(const unsigned char*)"\x00\x00\x00\x7e\x00", "AAAAfgA=" },
        {5,(const unsigned char*)"\x00\x00\x00\x7e\x7e", "AAAAfn4=" },
        {5,(const unsigned char*)"\x00\x00\x00\x7e\xfc", "AAAAfvw=" },
        {5,(const unsigned char*)"\x00\x00\x00\xfc\x00", "AAAA/AA=" },
        {5,(const unsigned char*)"\x00\x00\x00\xfc\x7e", "AAAA/H4=" },
        {5,(const unsigned char*)"\x00\x00\x00\xfc\xfc", "AAAA/Pw=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x00\x00", "AAB+AAA=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x00\x7e", "AAB+AH4=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x00\xfc", "AAB+APw=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x7e\x00", "AAB+fgA=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x7e\x7e", "AAB+fn4=" },
        {5,(const unsigned char*)"\x00\x00\x7e\x7e\xfc", "AAB+fvw=" },
        {5,(const unsigned char*)"\x00\x00\x7e\xfc\x00", "AAB+/AA=" },
        {5,(const unsigned char*)"\x00\x00\x7e\xfc\x7e", "AAB+/H4=" },
        {5,(const unsigned char*)"\x00\x00\x7e\xfc\xfc", "AAB+/Pw=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x00\x00", "AAD8AAA=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x00\x7e", "AAD8AH4=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x00\xfc", "AAD8APw=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x7e\x00", "AAD8fgA=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x7e\x7e", "AAD8fn4=" },
        {5,(const unsigned char*)"\x00\x00\xfc\x7e\xfc", "AAD8fvw=" },
        {5,(const unsigned char*)"\x00\x00\xfc\xfc\x00", "AAD8/AA=" },
        {5,(const unsigned char*)"\x00\x00\xfc\xfc\x7e", "AAD8/H4=" },
        {5,(const unsigned char*)"\x00\x00\xfc\xfc\xfc", "AAD8/Pw=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x00\x00", "AH4AAAA=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x00\x7e", "AH4AAH4=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x00\xfc", "AH4AAPw=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x7e\x00", "AH4AfgA=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x7e\x7e", "AH4Afn4=" },
        {5,(const unsigned char*)"\x00\x7e\x00\x7e\xfc", "AH4Afvw=" },
        {5,(const unsigned char*)"\x00\x7e\x00\xfc\x00", "AH4A/AA=" },
        {5,(const unsigned char*)"\x00\x7e\x00\xfc\x7e", "AH4A/H4=" },
        {5,(const unsigned char*)"\x00\x7e\x00\xfc\xfc", "AH4A/Pw=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x00\x00", "AH5+AAA=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x00\x7e", "AH5+AH4=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x00\xfc", "AH5+APw=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x7e\x00", "AH5+fgA=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x7e\x7e", "AH5+fn4=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\x7e\xfc", "AH5+fvw=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\xfc\x00", "AH5+/AA=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\xfc\x7e", "AH5+/H4=" },
        {5,(const unsigned char*)"\x00\x7e\x7e\xfc\xfc", "AH5+/Pw=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x00\x00", "AH78AAA=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x00\x7e", "AH78AH4=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x00\xfc", "AH78APw=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x7e\x00", "AH78fgA=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x7e\x7e", "AH78fn4=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\x7e\xfc", "AH78fvw=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\xfc\x00", "AH78/AA=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\xfc\x7e", "AH78/H4=" },
        {5,(const unsigned char*)"\x00\x7e\xfc\xfc\xfc", "AH78/Pw=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x00\x00", "APwAAAA=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x00\x7e", "APwAAH4=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x00\xfc", "APwAAPw=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x7e\x00", "APwAfgA=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x7e\x7e", "APwAfn4=" },
        {5,(const unsigned char*)"\x00\xfc\x00\x7e\xfc", "APwAfvw=" },
        {5,(const unsigned char*)"\x00\xfc\x00\xfc\x00", "APwA/AA=" },
        {5,(const unsigned char*)"\x00\xfc\x00\xfc\x7e", "APwA/H4=" },
        {5,(const unsigned char*)"\x00\xfc\x00\xfc\xfc", "APwA/Pw=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x00\x00", "APx+AAA=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x00\x7e", "APx+AH4=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x00\xfc", "APx+APw=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x7e\x00", "APx+fgA=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x7e\x7e", "APx+fn4=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\x7e\xfc", "APx+fvw=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\xfc\x00", "APx+/AA=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\xfc\x7e", "APx+/H4=" },
        {5,(const unsigned char*)"\x00\xfc\x7e\xfc\xfc", "APx+/Pw=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x00\x00", "APz8AAA=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x00\x7e", "APz8AH4=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x00\xfc", "APz8APw=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x7e\x00", "APz8fgA=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x7e\x7e", "APz8fn4=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\x7e\xfc", "APz8fvw=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\xfc\x00", "APz8/AA=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\xfc\x7e", "APz8/H4=" },
        {5,(const unsigned char*)"\x00\xfc\xfc\xfc\xfc", "APz8/Pw=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x00\x00", "fgAAAAA=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x00\x7e", "fgAAAH4=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x00\xfc", "fgAAAPw=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x7e\x00", "fgAAfgA=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x7e\x7e", "fgAAfn4=" },
        {5,(const unsigned char*)"\x7e\x00\x00\x7e\xfc", "fgAAfvw=" },
        {5,(const unsigned char*)"\x7e\x00\x00\xfc\x00", "fgAA/AA=" },
        {5,(const unsigned char*)"\x7e\x00\x00\xfc\x7e", "fgAA/H4=" },
        {5,(const unsigned char*)"\x7e\x00\x00\xfc\xfc", "fgAA/Pw=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x00\x00", "fgB+AAA=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x00\x7e", "fgB+AH4=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x00\xfc", "fgB+APw=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x7e\x00", "fgB+fgA=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x7e\x7e", "fgB+fn4=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\x7e\xfc", "fgB+fvw=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\xfc\x00", "fgB+/AA=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\xfc\x7e", "fgB+/H4=" },
        {5,(const unsigned char*)"\x7e\x00\x7e\xfc\xfc", "fgB+/Pw=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x00\x00", "fgD8AAA=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x00\x7e", "fgD8AH4=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x00\xfc", "fgD8APw=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x7e\x00", "fgD8fgA=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x7e\x7e", "fgD8fn4=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\x7e\xfc", "fgD8fvw=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\xfc\x00", "fgD8/AA=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\xfc\x7e", "fgD8/H4=" },
        {5,(const unsigned char*)"\x7e\x00\xfc\xfc\xfc", "fgD8/Pw=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x00\x00", "fn4AAAA=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x00\x7e", "fn4AAH4=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x00\xfc", "fn4AAPw=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x7e\x00", "fn4AfgA=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x7e\x7e", "fn4Afn4=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\x7e\xfc", "fn4Afvw=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\xfc\x00", "fn4A/AA=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\xfc\x7e", "fn4A/H4=" },
        {5,(const unsigned char*)"\x7e\x7e\x00\xfc\xfc", "fn4A/Pw=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x00\x00", "fn5+AAA=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x00\x7e", "fn5+AH4=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x00\xfc", "fn5+APw=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x7e\x00", "fn5+fgA=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x7e\x7e", "fn5+fn4=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\x7e\xfc", "fn5+fvw=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\xfc\x00", "fn5+/AA=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\xfc\x7e", "fn5+/H4=" },
        {5,(const unsigned char*)"\x7e\x7e\x7e\xfc\xfc", "fn5+/Pw=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x00\x00", "fn78AAA=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x00\x7e", "fn78AH4=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x00\xfc", "fn78APw=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x7e\x00", "fn78fgA=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x7e\x7e", "fn78fn4=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\x7e\xfc", "fn78fvw=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\xfc\x00", "fn78/AA=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\xfc\x7e", "fn78/H4=" },
        {5,(const unsigned char*)"\x7e\x7e\xfc\xfc\xfc", "fn78/Pw=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x00\x00", "fvwAAAA=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x00\x7e", "fvwAAH4=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x00\xfc", "fvwAAPw=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x7e\x00", "fvwAfgA=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x7e\x7e", "fvwAfn4=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\x7e\xfc", "fvwAfvw=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\xfc\x00", "fvwA/AA=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\xfc\x7e", "fvwA/H4=" },
        {5,(const unsigned char*)"\x7e\xfc\x00\xfc\xfc", "fvwA/Pw=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x00\x00", "fvx+AAA=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x00\x7e", "fvx+AH4=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x00\xfc", "fvx+APw=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x7e\x00", "fvx+fgA=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x7e\x7e", "fvx+fn4=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\x7e\xfc", "fvx+fvw=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\xfc\x00", "fvx+/AA=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\xfc\x7e", "fvx+/H4=" },
        {5,(const unsigned char*)"\x7e\xfc\x7e\xfc\xfc", "fvx+/Pw=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x00\x00", "fvz8AAA=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x00\x7e", "fvz8AH4=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x00\xfc", "fvz8APw=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x7e\x00", "fvz8fgA=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x7e\x7e", "fvz8fn4=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\x7e\xfc", "fvz8fvw=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\xfc\x00", "fvz8/AA=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\xfc\x7e", "fvz8/H4=" },
        {5,(const unsigned char*)"\x7e\xfc\xfc\xfc\xfc", "fvz8/Pw=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x00\x00", "/AAAAAA=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x00\x7e", "/AAAAH4=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x00\xfc", "/AAAAPw=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x7e\x00", "/AAAfgA=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x7e\x7e", "/AAAfn4=" },
        {5,(const unsigned char*)"\xfc\x00\x00\x7e\xfc", "/AAAfvw=" },
        {5,(const unsigned char*)"\xfc\x00\x00\xfc\x00", "/AAA/AA=" },
        {5,(const unsigned char*)"\xfc\x00\x00\xfc\x7e", "/AAA/H4=" },
        {5,(const unsigned char*)"\xfc\x00\x00\xfc\xfc", "/AAA/Pw=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x00\x00", "/AB+AAA=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x00\x7e", "/AB+AH4=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x00\xfc", "/AB+APw=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x7e\x00", "/AB+fgA=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x7e\x7e", "/AB+fn4=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\x7e\xfc", "/AB+fvw=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\xfc\x00", "/AB+/AA=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\xfc\x7e", "/AB+/H4=" },
        {5,(const unsigned char*)"\xfc\x00\x7e\xfc\xfc", "/AB+/Pw=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x00\x00", "/AD8AAA=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x00\x7e", "/AD8AH4=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x00\xfc", "/AD8APw=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x7e\x00", "/AD8fgA=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x7e\x7e", "/AD8fn4=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\x7e\xfc", "/AD8fvw=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\xfc\x00", "/AD8/AA=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\xfc\x7e", "/AD8/H4=" },
        {5,(const unsigned char*)"\xfc\x00\xfc\xfc\xfc", "/AD8/Pw=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x00\x00", "/H4AAAA=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x00\x7e", "/H4AAH4=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x00\xfc", "/H4AAPw=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x7e\x00", "/H4AfgA=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x7e\x7e", "/H4Afn4=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\x7e\xfc", "/H4Afvw=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\xfc\x00", "/H4A/AA=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\xfc\x7e", "/H4A/H4=" },
        {5,(const unsigned char*)"\xfc\x7e\x00\xfc\xfc", "/H4A/Pw=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x00\x00", "/H5+AAA=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x00\x7e", "/H5+AH4=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x00\xfc", "/H5+APw=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x7e\x00", "/H5+fgA=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x7e\x7e", "/H5+fn4=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\x7e\xfc", "/H5+fvw=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\xfc\x00", "/H5+/AA=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\xfc\x7e", "/H5+/H4=" },
        {5,(const unsigned char*)"\xfc\x7e\x7e\xfc\xfc", "/H5+/Pw=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x00\x00", "/H78AAA=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x00\x7e", "/H78AH4=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x00\xfc", "/H78APw=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x7e\x00", "/H78fgA=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x7e\x7e", "/H78fn4=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\x7e\xfc", "/H78fvw=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\xfc\x00", "/H78/AA=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\xfc\x7e", "/H78/H4=" },
        {5,(const unsigned char*)"\xfc\x7e\xfc\xfc\xfc", "/H78/Pw=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x00\x00", "/PwAAAA=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x00\x7e", "/PwAAH4=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x00\xfc", "/PwAAPw=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x7e\x00", "/PwAfgA=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x7e\x7e", "/PwAfn4=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\x7e\xfc", "/PwAfvw=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\xfc\x00", "/PwA/AA=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\xfc\x7e", "/PwA/H4=" },
        {5,(const unsigned char*)"\xfc\xfc\x00\xfc\xfc", "/PwA/Pw=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x00\x00", "/Px+AAA=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x00\x7e", "/Px+AH4=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x00\xfc", "/Px+APw=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x7e\x00", "/Px+fgA=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x7e\x7e", "/Px+fn4=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\x7e\xfc", "/Px+fvw=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\xfc\x00", "/Px+/AA=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\xfc\x7e", "/Px+/H4=" },
        {5,(const unsigned char*)"\xfc\xfc\x7e\xfc\xfc", "/Px+/Pw=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x00\x00", "/Pz8AAA=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x00\x7e", "/Pz8AH4=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x00\xfc", "/Pz8APw=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x7e\x00", "/Pz8fgA=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x7e\x7e", "/Pz8fn4=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\x7e\xfc", "/Pz8fvw=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\xfc\x00", "/Pz8/AA=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\xfc\x7e", "/Pz8/H4=" },
        {5,(const unsigned char*)"\xfc\xfc\xfc\xfc\xfc", "/Pz8/Pw=" },
        {6,(const unsigned char*)"\x00\x00\x00\x00\x00\x00", "AAAAAAAA" },
        {6,(const unsigned char*)"\x00\x00\x00\x00\x00\xa8", "AAAAAACo" },
        {6,(const unsigned char*)"\x00\x00\x00\x00\xa8\x00", "AAAAAKgA" },
        {6,(const unsigned char*)"\x00\x00\x00\x00\xa8\xa8", "AAAAAKio" },
        {6,(const unsigned char*)"\x00\x00\x00\xa8\x00\x00", "AAAAqAAA" },
        {6,(const unsigned char*)"\x00\x00\x00\xa8\x00\xa8", "AAAAqACo" },
        {6,(const unsigned char*)"\x00\x00\x00\xa8\xa8\x00", "AAAAqKgA" },
        {6,(const unsigned char*)"\x00\x00\x00\xa8\xa8\xa8", "AAAAqKio" },
        {6,(const unsigned char*)"\x00\x00\xa8\x00\x00\x00", "AACoAAAA" },
        {6,(const unsigned char*)"\x00\x00\xa8\x00\x00\xa8", "AACoAACo" },
        {6,(const unsigned char*)"\x00\x00\xa8\x00\xa8\x00", "AACoAKgA" },
        {6,(const unsigned char*)"\x00\x00\xa8\x00\xa8\xa8", "AACoAKio" },
        {6,(const unsigned char*)"\x00\x00\xa8\xa8\x00\x00", "AACoqAAA" },
        {6,(const unsigned char*)"\x00\x00\xa8\xa8\x00\xa8", "AACoqACo" },
        {6,(const unsigned char*)"\x00\x00\xa8\xa8\xa8\x00", "AACoqKgA" },
        {6,(const unsigned char*)"\x00\x00\xa8\xa8\xa8\xa8", "AACoqKio" },
        {6,(const unsigned char*)"\x00\xa8\x00\x00\x00\x00", "AKgAAAAA" },
        {6,(const unsigned char*)"\x00\xa8\x00\x00\x00\xa8", "AKgAAACo" },
        {6,(const unsigned char*)"\x00\xa8\x00\x00\xa8\x00", "AKgAAKgA" },
        {6,(const unsigned char*)"\x00\xa8\x00\x00\xa8\xa8", "AKgAAKio" },
        {6,(const unsigned char*)"\x00\xa8\x00\xa8\x00\x00", "AKgAqAAA" },
        {6,(const unsigned char*)"\x00\xa8\x00\xa8\x00\xa8", "AKgAqACo" },
        {6,(const unsigned char*)"\x00\xa8\x00\xa8\xa8\x00", "AKgAqKgA" },
        {6,(const unsigned char*)"\x00\xa8\x00\xa8\xa8\xa8", "AKgAqKio" },
        {6,(const unsigned char*)"\x00\xa8\xa8\x00\x00\x00", "AKioAAAA" },
        {6,(const unsigned char*)"\x00\xa8\xa8\x00\x00\xa8", "AKioAACo" },
        {6,(const unsigned char*)"\x00\xa8\xa8\x00\xa8\x00", "AKioAKgA" },
        {6,(const unsigned char*)"\x00\xa8\xa8\x00\xa8\xa8", "AKioAKio" },
        {6,(const unsigned char*)"\x00\xa8\xa8\xa8\x00\x00", "AKioqAAA" },
        {6,(const unsigned char*)"\x00\xa8\xa8\xa8\x00\xa8", "AKioqACo" },
        {6,(const unsigned char*)"\x00\xa8\xa8\xa8\xa8\x00", "AKioqKgA" },
        {6,(const unsigned char*)"\x00\xa8\xa8\xa8\xa8\xa8", "AKioqKio" },
        {6,(const unsigned char*)"\xa8\x00\x00\x00\x00\x00", "qAAAAAAA" },
        {6,(const unsigned char*)"\xa8\x00\x00\x00\x00\xa8", "qAAAAACo" },
        {6,(const unsigned char*)"\xa8\x00\x00\x00\xa8\x00", "qAAAAKgA" },
        {6,(const unsigned char*)"\xa8\x00\x00\x00\xa8\xa8", "qAAAAKio" },
        {6,(const unsigned char*)"\xa8\x00\x00\xa8\x00\x00", "qAAAqAAA" },
        {6,(const unsigned char*)"\xa8\x00\x00\xa8\x00\xa8", "qAAAqACo" },
        {6,(const unsigned char*)"\xa8\x00\x00\xa8\xa8\x00", "qAAAqKgA" },
        {6,(const unsigned char*)"\xa8\x00\x00\xa8\xa8\xa8", "qAAAqKio" },
        {6,(const unsigned char*)"\xa8\x00\xa8\x00\x00\x00", "qACoAAAA" },
        {6,(const unsigned char*)"\xa8\x00\xa8\x00\x00\xa8", "qACoAACo" },
        {6,(const unsigned char*)"\xa8\x00\xa8\x00\xa8\x00", "qACoAKgA" },
        {6,(const unsigned char*)"\xa8\x00\xa8\x00\xa8\xa8", "qACoAKio" },
        {6,(const unsigned char*)"\xa8\x00\xa8\xa8\x00\x00", "qACoqAAA" },
        {6,(const unsigned char*)"\xa8\x00\xa8\xa8\x00\xa8", "qACoqACo" },
        {6,(const unsigned char*)"\xa8\x00\xa8\xa8\xa8\x00", "qACoqKgA" },
        {6,(const unsigned char*)"\xa8\x00\xa8\xa8\xa8\xa8", "qACoqKio" },
        {6,(const unsigned char*)"\xa8\xa8\x00\x00\x00\x00", "qKgAAAAA" },
        {6,(const unsigned char*)"\xa8\xa8\x00\x00\x00\xa8", "qKgAAACo" },
        {6,(const unsigned char*)"\xa8\xa8\x00\x00\xa8\x00", "qKgAAKgA" },
        {6,(const unsigned char*)"\xa8\xa8\x00\x00\xa8\xa8", "qKgAAKio" },
        {6,(const unsigned char*)"\xa8\xa8\x00\xa8\x00\x00", "qKgAqAAA" },
        {6,(const unsigned char*)"\xa8\xa8\x00\xa8\x00\xa8", "qKgAqACo" },
        {6,(const unsigned char*)"\xa8\xa8\x00\xa8\xa8\x00", "qKgAqKgA" },
        {6,(const unsigned char*)"\xa8\xa8\x00\xa8\xa8\xa8", "qKgAqKio" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\x00\x00\x00", "qKioAAAA" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\x00\x00\xa8", "qKioAACo" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\x00\xa8\x00", "qKioAKgA" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\x00\xa8\xa8", "qKioAKio" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\xa8\x00\x00", "qKioqAAA" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\xa8\x00\xa8", "qKioqACo" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\xa8\xa8\x00", "qKioqKgA" },
        {6,(const unsigned char*)"\xa8\xa8\xa8\xa8\xa8\xa8", "qKioqKio" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\x00\x00\x00", "AAAAAAAAAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\x00\x00\xd3", "AAAAAAAA0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\x00\xd3\x00", "AAAAAADTAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\x00\xd3\xd3", "AAAAAADT0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\xd3\x00\x00", "AAAAANMAAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\xd3\x00\xd3", "AAAAANMA0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\xd3\xd3\x00", "AAAAANPTAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\x00\xd3\xd3\xd3", "AAAAANPT0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\x00\x00\x00", "AAAA0wAAAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\x00\x00\xd3", "AAAA0wAA0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\x00\xd3\x00", "AAAA0wDTAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\x00\xd3\xd3", "AAAA0wDT0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\xd3\x00\x00", "AAAA09MAAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\xd3\x00\xd3", "AAAA09MA0w==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\xd3\xd3\x00", "AAAA09PTAA==" },
        {7,(const unsigned char*)"\x00\x00\x00\xd3\xd3\xd3\xd3", "AAAA09PT0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\x00\x00\x00", "AADTAAAAAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\x00\x00\xd3", "AADTAAAA0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\x00\xd3\x00", "AADTAADTAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\x00\xd3\xd3", "AADTAADT0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\xd3\x00\x00", "AADTANMAAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\xd3\x00\xd3", "AADTANMA0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\xd3\xd3\x00", "AADTANPTAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\x00\xd3\xd3\xd3", "AADTANPT0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\x00\x00\x00", "AADT0wAAAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\x00\x00\xd3", "AADT0wAA0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\x00\xd3\x00", "AADT0wDTAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\x00\xd3\xd3", "AADT0wDT0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\xd3\x00\x00", "AADT09MAAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\xd3\x00\xd3", "AADT09MA0w==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\xd3\xd3\x00", "AADT09PTAA==" },
        {7,(const unsigned char*)"\x00\x00\xd3\xd3\xd3\xd3\xd3", "AADT09PT0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\x00\x00\x00", "ANMAAAAAAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\x00\x00\xd3", "ANMAAAAA0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\x00\xd3\x00", "ANMAAADTAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\x00\xd3\xd3", "ANMAAADT0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\xd3\x00\x00", "ANMAANMAAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\xd3\x00\xd3", "ANMAANMA0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\xd3\xd3\x00", "ANMAANPTAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\x00\xd3\xd3\xd3", "ANMAANPT0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\x00\x00\x00", "ANMA0wAAAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\x00\x00\xd3", "ANMA0wAA0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\x00\xd3\x00", "ANMA0wDTAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\x00\xd3\xd3", "ANMA0wDT0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\xd3\x00\x00", "ANMA09MAAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\xd3\x00\xd3", "ANMA09MA0w==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\xd3\xd3\x00", "ANMA09PTAA==" },
        {7,(const unsigned char*)"\x00\xd3\x00\xd3\xd3\xd3\xd3", "ANMA09PT0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\x00\x00\x00", "ANPTAAAAAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\x00\x00\xd3", "ANPTAAAA0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\x00\xd3\x00", "ANPTAADTAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\x00\xd3\xd3", "ANPTAADT0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\xd3\x00\x00", "ANPTANMAAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\xd3\x00\xd3", "ANPTANMA0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\xd3\xd3\x00", "ANPTANPTAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\x00\xd3\xd3\xd3", "ANPTANPT0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\x00\x00\x00", "ANPT0wAAAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\x00\x00\xd3", "ANPT0wAA0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\x00\xd3\x00", "ANPT0wDTAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\x00\xd3\xd3", "ANPT0wDT0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\xd3\x00\x00", "ANPT09MAAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\xd3\x00\xd3", "ANPT09MA0w==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\xd3\xd3\x00", "ANPT09PTAA==" },
        {7,(const unsigned char*)"\x00\xd3\xd3\xd3\xd3\xd3\xd3", "ANPT09PT0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\x00\x00\x00", "0wAAAAAAAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\x00\x00\xd3", "0wAAAAAA0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\x00\xd3\x00", "0wAAAADTAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\x00\xd3\xd3", "0wAAAADT0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\xd3\x00\x00", "0wAAANMAAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\xd3\x00\xd3", "0wAAANMA0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\xd3\xd3\x00", "0wAAANPTAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\x00\xd3\xd3\xd3", "0wAAANPT0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\x00\x00\x00", "0wAA0wAAAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\x00\x00\xd3", "0wAA0wAA0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\x00\xd3\x00", "0wAA0wDTAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\x00\xd3\xd3", "0wAA0wDT0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\xd3\x00\x00", "0wAA09MAAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\xd3\x00\xd3", "0wAA09MA0w==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\xd3\xd3\x00", "0wAA09PTAA==" },
        {7,(const unsigned char*)"\xd3\x00\x00\xd3\xd3\xd3\xd3", "0wAA09PT0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\x00\x00\x00", "0wDTAAAAAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\x00\x00\xd3", "0wDTAAAA0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\x00\xd3\x00", "0wDTAADTAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\x00\xd3\xd3", "0wDTAADT0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\xd3\x00\x00", "0wDTANMAAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\xd3\x00\xd3", "0wDTANMA0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\xd3\xd3\x00", "0wDTANPTAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\x00\xd3\xd3\xd3", "0wDTANPT0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\x00\x00\x00", "0wDT0wAAAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\x00\x00\xd3", "0wDT0wAA0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\x00\xd3\x00", "0wDT0wDTAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\x00\xd3\xd3", "0wDT0wDT0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\xd3\x00\x00", "0wDT09MAAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\xd3\x00\xd3", "0wDT09MA0w==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\xd3\xd3\x00", "0wDT09PTAA==" },
        {7,(const unsigned char*)"\xd3\x00\xd3\xd3\xd3\xd3\xd3", "0wDT09PT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\x00\x00\x00", "09MAAAAAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\x00\x00\xd3", "09MAAAAA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\x00\xd3\x00", "09MAAADTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\x00\xd3\xd3", "09MAAADT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\xd3\x00\x00", "09MAANMAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\xd3\x00\xd3", "09MAANMA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\xd3\xd3\x00", "09MAANPTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\x00\xd3\xd3\xd3", "09MAANPT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\x00\x00\x00", "09MA0wAAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\x00\x00\xd3", "09MA0wAA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\x00\xd3\x00", "09MA0wDTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\x00\xd3\xd3", "09MA0wDT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\xd3\x00\x00", "09MA09MAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\xd3\x00\xd3", "09MA09MA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\xd3\xd3\x00", "09MA09PTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\x00\xd3\xd3\xd3\xd3", "09MA09PT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\x00\x00\x00", "09PTAAAAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\x00\x00\xd3", "09PTAAAA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\x00\xd3\x00", "09PTAADTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\x00\xd3\xd3", "09PTAADT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\xd3\x00\x00", "09PTANMAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\xd3\x00\xd3", "09PTANMA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\xd3\xd3\x00", "09PTANPTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\x00\xd3\xd3\xd3", "09PTANPT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\x00\x00\x00", "09PT0wAAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\x00\x00\xd3", "09PT0wAA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\x00\xd3\x00", "09PT0wDTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\x00\xd3\xd3", "09PT0wDT0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\xd3\x00\x00", "09PT09MAAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\xd3\x00\xd3", "09PT09MA0w==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\xd3\xd3\x00", "09PT09PTAA==" },
        {7,(const unsigned char*)"\xd3\xd3\xd3\xd3\xd3\xd3\xd3", "09PT09PT0w==" },
        {8,(const unsigned char*)"\x00\x00\x00\x00\x00\x00\x00\x00", "AAAAAAAAAAA=" },
        {9,(const unsigned char*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00", "AAAAAAAAAAAA" },
        {10,(const unsigned char*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "AAAAAAAAAAAAAA==" },

    };

    static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

BEGIN_TEST_SUITE(base64_unittests)

TEST_SUITE_INITIALIZE(TestSuiteInitialize)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(f)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(cleans)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/*Tests_SRS_BASE64_06_001: [If input is NULL then Azure_Base64_Encode shall return NULL.]*/
TEST_FUNCTION(Base64_Encode_bad_input)
{
    //arrange
    STRING_HANDLE result;

    //act
    result = Azure_Base64_Encode(NULL);
    //assert
    ASSERT_IS_NULL( result);

}

/*Tests_SRS_BASE64_06_007: [Otherwise Azure_Base64_Encode shall return a pointer to STRING, that string contains the base 64 encoding of inpuit.]*/
TEST_FUNCTION(Base64_Encode_simple_good)
{
    //arrange
    BUFFER_HANDLE input = BUFFER_new();
    STRING_HANDLE result;

    //act
    result = Azure_Base64_Encode(input);

    //assert
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(size_t, (size_t)0, strlen(STRING_c_str(result)));

    // Cleanup
    STRING_delete(result);
    BUFFER_delete(input);
}

TEST_FUNCTION(Base64_Encode_one_char_encode)
{
    //arrange
    BUFFER_HANDLE input = BUFFER_new();
    STRING_HANDLE result;
    const char* oneCharacter = "a";

    BUFFER_build(input, (unsigned char*)oneCharacter,strlen(oneCharacter));
    //act
    result = Azure_Base64_Encode(input);

    //assert
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, "YQ==", STRING_c_str(result));

    // Cleanup
    STRING_delete(result);
    BUFFER_delete(input);

}

TEST_FUNCTION(Base64_Encode_leviathan_succeeds)
{
    //arrange
    BUFFER_HANDLE input = BUFFER_new();
    STRING_HANDLE result;
    const char* leviathan = "any carnal pleasure.";

    BUFFER_build(input, (unsigned char*)leviathan, strlen(leviathan));
    //act
    result = Azure_Base64_Encode(input);

    //assert
    ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
    ASSERT_ARE_EQUAL(char_ptr, "YW55IGNhcm5hbCBwbGVhc3VyZS4=", STRING_c_str(result));

    // Cleanup
    STRING_delete(result);
    BUFFER_delete(input);

}

TEST_FUNCTION(Base64_Encode_exhaustive_succeeds)
{
    ///arrange
    size_t i;

    for (i = 0; i < sizeof(testVector_BINARY_with_equal_signs) / sizeof(testVector_BINARY_with_equal_signs[0]); i++)
    {
        ///arrange
        BUFFER_HANDLE input = BUFFER_new();
        STRING_HANDLE result;

        ASSERT_ARE_EQUAL(int, 0, BUFFER_build(input, testVector_BINARY_with_equal_signs[i].inputData, testVector_BINARY_with_equal_signs[i].inputLength));

        ///act
        result = Azure_Base64_Encode(input);

        ///assert
        ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
        ASSERT_ARE_EQUAL(char_ptr, testVector_BINARY_with_equal_signs[i].expectedOutput, STRING_c_str(result));


        ///cleanup
        BUFFER_delete(input);
        STRING_delete(result);
    }
}

/*Tests_SRS_BASE64_02_001: [If source is NULL then Azure_Base64_Encode_Bytes shall return NULL.] */
TEST_FUNCTION(Base64_Encode_Bytes_with_NULL_source_returns_NULL)
{
    ///arrange

    ///act
    STRING_HANDLE result = Azure_Base64_Encode_Bytes(NULL, 3);

    ///assert
    ASSERT_IS_NULL(result);

    ///cleanup
}

/*Tests_SRS_BASE64_02_002: [If source is not NULL and size is zero, then Azure_Base64_Encode_Bytes shall produce an empty STRING_HANDLE.]*/
TEST_FUNCTION(Base64_Encode_Bytes_with_zero_size_returns_empty_string)
{
    ///arrange

    ///act
    STRING_HANDLE result = Azure_Base64_Encode_Bytes((const unsigned char*)"a", 0);

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 0, STRING_length(result));

    ///cleanup
    STRING_delete(result);
}


/*Tests_SRS_BASE64_02_003: [Otherwise, Azure_Base64_Encode_Bytes shall produce a STRING_HANDLE containing the Base64 representation of the buffer.] */
TEST_FUNCTION(Base64_Encode_Bytes_exhaustive_succeeds)
{
    ///arrange
    size_t i;

    for (i = 0; i < sizeof(testVector_BINARY_with_equal_signs) / sizeof(testVector_BINARY_with_equal_signs[0]); i++)
    {
        ///arrange
        STRING_HANDLE result;

        ///act
        result = Azure_Base64_Encode_Bytes(testVector_BINARY_with_equal_signs[i].inputData, testVector_BINARY_with_equal_signs[i].inputLength);

        ///assert
        ASSERT_ARE_NOT_EQUAL(void_ptr, NULL, result);
        ASSERT_ARE_EQUAL(char_ptr, testVector_BINARY_with_equal_signs[i].expectedOutput, STRING_c_str(result));

        ///cleanup
        STRING_delete(result);
    }
}

TEST_FUNCTION(Azure_Base64_Decode_exhaustive_succeeds)
{
    size_t i;
    for (i = 0; i < sizeof(testVector_BINARY_with_equal_signs) / sizeof(testVector_BINARY_with_equal_signs[0]); i++)
    {
        ///Arrange
        BUFFER_HANDLE result;

        ///act
        result = Azure_Base64_Decode(testVector_BINARY_with_equal_signs[i].expectedOutput);

        ///assert
        ASSERT_ARE_EQUAL(size_t, testVector_BINARY_with_equal_signs[i].inputLength, BUFFER_length(result));
        ASSERT_ARE_EQUAL(int, (int)0, memcmp(BUFFER_u_char(result), testVector_BINARY_with_equal_signs[i].inputData, BUFFER_length(result)));

        ///Cleanup
        BUFFER_delete(result);
    }
}

/*Tests_SRS_BASE64_06_008: [If source is NULL then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_null_return_null)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode(NULL);

    ///assert
    ASSERT_IS_NULL(result);
}

/*Tests_SRS_BASE64_06_009: [If the string pointed to by source is zero length then the handle returned shall refer to a zero length buffer.]*/
TEST_FUNCTION(Azure_Base64_Decode_zero_length_returns_zero_length)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("");

    ///assert
    ASSERT_IS_NOT_NULL(result);
    ASSERT_ARE_EQUAL(size_t, 0, BUFFER_length(result));

    ///Cleanup
    BUFFER_delete(result);
}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_1)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("1");

    ///assert
    ASSERT_IS_NULL(result);

}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_2)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("12");

    ///assert
    ASSERT_IS_NULL(result);

}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_3)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("123");

    ///assert
    ASSERT_IS_NULL(result);

}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_4)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("12345");

    ///assert
    ASSERT_IS_NULL(result);

}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_5)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("123456");
    ASSERT_IS_NULL(result);

}

/*Tests_SRS_BASE64_06_011: [If the source string has an invalid length for a base 64 encoded string then Azure_Base64_Decode shall return NULL.]*/
TEST_FUNCTION(Azure_Base64_Decode_invalid_length_fails_6)
{
    ///Arrange
    BUFFER_HANDLE result;

    ///act
    result = Azure_Base64_Decode("1234567");

    ///assert
    ASSERT_IS_NULL(result);

}


END_TEST_SUITE(base64_unittests);
