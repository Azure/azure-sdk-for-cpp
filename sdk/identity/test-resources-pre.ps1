# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# IMPORTANT: Do not invoke this file directly. Please instead run eng/New-TestResources.ps1 from the repository root.

#Requires -Version 6.0
#Requires -PSEdition Core

using namespace System.Security.Cryptography
using namespace System.Security.Cryptography.X509Certificates

# Use same parameter names as declared in eng/New-TestResources.ps1 (assume validation therein).
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'Medium')]
param (
    # Captures any arguments from eng/New-TestResources.ps1 not declared here (no parameter errors).
    [Parameter(ValueFromRemainingArguments = $true)]
    $RemainingArguments
)

# By default stop for any error.
if (!$PSBoundParameters.ContainsKey('ErrorAction')) {
    $ErrorActionPreference = 'Stop'
}

$certPath = Join-Path $(Get-Location) "azure-identity-test2.pem"

@"
Bag Attributes
    Microsoft Local Key set: <No Values>
    localKeyID: 01 00 00 00 
    friendlyName: te-66f5c973-4fc8-4cd3-8acc-64964d79b693
    Microsoft CSP Name: Microsoft Software Key Storage Provider
Key Attributes
    X509v3 Key Usage: 90 
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDPdm4pukO7ugEx
8wXrmo4VIEoicp7w3QsEJGA2bMx9nHMvwugG54t14QpfqBQYQWLeL1HmpcDeivVD
+15ZXeGLCPVZBHhoY8ZWGibfhAAzqQ0P9Ca1kydjvB4uJcEnF/RYtQv6n6OwmdO1
wJ22JNcRlMtZqmnb/Q0In2fjXEbdl85/GZlYzMQRdyfI0yriSRBcYV2kg0zeXCxf
mCvB3rb6I1KpoUFHlkeHtkeDwm0VHUEt4Hz8ghcB00tI5eS2fH2rPkINQKc6+0QU
C2KICQC+GzJsYDbwQOao5Vhk80H5LRuM9Ndzv+fU3lLnktYCgXgL9AX4L/R9Z4Pz
tuao/qbRAgMBAAECggEBAMQZIrooiTuZ7uVC3Ja96Y1IjyqOg3QSzAXnSFZJcuVM
i4hayC02khkjVUXjvtLKg2SW/+hvRqZUXM8cfCsm1Tkxh4/T7OhnXyMl5xahU/uA
0IsC8c/xv2rDdxeRskh8mQd8Yk1MtlIIpRgIcEqp+exxY+FmdldtkvNSkcVUBNwQ
nXi+oWPhE2guo2g1BPk2gbF0+3FvSrQ8QwGHg+uQJwrQpJ+SB9TyuQFauGR5/wSq
H93cFH5YC/+v5I7qW6ZQe0f7rEKQDybGVzkBlKJyGCVYmPn7Xa/wJriws+FZIfHz
f3m0kJigxJd/HwTrnKSg+H8oBgng7lZLdBYWHMGJhA0CgYEA48moW7szegvfLuUF
a0sHfyKuNyvOv7Wud4sa0lwdKPHS+atwL6TNUWCAGkomYADEe3qiYgMXDX9U3hlW
6zktYFj03tnRg4iBjp8nchLBVLf3Wd5TPRw1VKu4ZW43y8BRhYWV+3Z4s1nyMEDA
NFbKRmL7LDB05oWHdJMjFK/L6YcCgYEA6ShV4v2RQiXzkW6GHSBZDIVHCeWwvIld
OlEfG7wzZW4e8wNDhfSMtXyJrzfbEyXBtVKoESdP6Nnm9W7ftcynW965S94THuy7
+ofvHo6JAm8g/0uX70wZ26LU8qhkJMTWmsONBNKLwUzkFT7VGsdaBliam1RLvjeT
URdQgnftIucCgYEA4FYamT0k1W4bv/OOAr1CBNQDABME64ni6Zj2MXbGwSxou7s8
IbANBbgkcb/VS3d2CqYchqrEaWaeDp6mG8OUDO+POmsLDJ/D+NKF5rLR9L25vahY
EjdVzq3QTRTfnqspnnaR37Yt6XUMMLmUkfdn/yo8dKjEeMPJQ+YlBpqcGMECgYBZ
rmIaxV2yC9b8AX8khOS7pCgG7opkepGZdMp6aJF8WjcdUgwO4lmdFSIAe4OQgd1Y
WUq8Dlr2PZpQnSz/SJC3DZxISksggf5sBw06u6iHfyc6C2GNccAgcylljM+4NN42
+TCswi9vUpwIb/qYKkW+WyZcyLe5mrbXYhhdlrNn0QKBgDe8aRG+MOSUTTXjAVss
bDY0Us943FN91qBmagNqDyozKAAqDoKvdRxM0IlIDnOptj4AfbpJ1JThNOJDYBpU
+Azo8UoedANgndtZ2n11RSjmlQ6TE/WGlsirHExqr6y/l71znoQm1y3E2cArbsmy
hp0P5v42PKxmAx4pR0EjNKsd
-----END PRIVATE KEY-----
Bag Attributes
    localKeyID: 01 00 00 00 
    1.3.6.1.4.1.311.17.3.71: 61 00 6E 00 74 00 6B 00 2D 00 6C 00 61 00 70 00 74 00 6F 00 70 00 00 00 
subject=CN = azure-identity-test

issuer=CN = azure-identity-test

-----BEGIN CERTIFICATE-----
MIIDODCCAiCgAwIBAgIQNqa9U3MBxqBF7ksWk+XRkzANBgkqhkiG9w0BAQsFADAe
MRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MCAXDTIyMDQyMjE1MDYwNloY
DzIyMjIwMTAxMDcwMDAwWjAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz3ZuKbpDu7oBMfMF65qO
FSBKInKe8N0LBCRgNmzMfZxzL8LoBueLdeEKX6gUGEFi3i9R5qXA3or1Q/teWV3h
iwj1WQR4aGPGVhom34QAM6kND/QmtZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTX
EZTLWapp2/0NCJ9n41xG3ZfOfxmZWMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62
+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8/IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkA
vhsybGA28EDmqOVYZPNB+S0bjPTXc7/n1N5S55LWAoF4C/QF+C/0fWeD87bmqP6m
0QIDAQABo3AwbjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwIG
CCsGAQUFBwMBMB4GA1UdEQQXMBWCE2F6dXJlLWlkZW50aXR5LXRlc3QwHQYDVR0O
BBYEFCoJ5tInmafyNuR0tGxZOz522jlWMA0GCSqGSIb3DQEBCwUAA4IBAQBzLXpw
Xmrg1sQTmzMnS24mREKxj9B3YILmgsdBMrHkH07QUROee7IbQ8gfBKeln0dEcfYi
Jyh42jn+fmg9AR17RP80wPthD2eKOt4WYNkNM3H8U4JEo+0ML0jZyswynpR48h/E
m96sm/NUeKUViD5iVTb1uHL4j8mQAN1IbXcunXvrrek1CzFVn5Rpah0Tn+6cYVKd
Jg531i53udzusgZtV1NPZ82tzYkPQG1vxB//D9vd0LzmcfCvT50MKhz0r/c5yJYk
i9q94DBuzMhe+O9j+Ob2pVQt5akVFJVtIVSfBZzRBAd66u9JeADlT4sxwS4QAUHi
RrCsEpJsnJXkx/6O
-----END CERTIFICATE-----
"@ > $certPath

$EnvironmentVariables["AZURE_CLIENT_CERTIFICATE_PATH"] = $certPath
