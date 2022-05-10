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

function Log($Message) {
    Write-Host ('{0} - {1}' -f [DateTime]::Now.ToLongTimeString(), $Message)
}

function New-X509Certificate2([RSA] $rsa, [string] $SubjectName) {

    try {
        $req = [CertificateRequest]::new(
            [string] $SubjectName,
            $rsa,
            [HashAlgorithmName]::SHA256,
            [RSASignaturePadding]::Pkcs1
        )

        # TODO: Add any KUs necessary to $req.CertificateExtensions

        $req.CertificateExtensions.Add([X509BasicConstraintsExtension]::new($true, $false, 0, $false))

        $NotBefore = [DateTimeOffset]::Now.AddDays(-1)
        $NotAfter = $NotBefore.AddDays(365)

        $req.CreateSelfSigned($NotBefore, $NotAfter)
    }
    finally {
    }
}

function Export-X509Certificate2PEMWithPrivateKey([string] $Path, [X509Certificate2] $Certificate) {
    $rsa = [RSACertificateExtensions]::GetRSAPrivateKey($Certificate)
    $privKey = $rsa.ExportRSAPrivateKey()
@"
-----BEGIN PRIVATE KEY-----
$([Convert]::ToBase64String($privKey, 'InsertLineBreaks').Replace("=", "").Replace("/", "_").Replace("+", "-"))
-----END PRIVATE KEY-----

-----BEGIN CERTIFICATE-----
$([Convert]::ToBase64String($Certificate.RawData, 'InsertLineBreaks').Replace("=", "").Replace("/", "_").Replace("+", "-"))
-----END CERTIFICATE-----
"@ > $Path

}

Log "Running PreConfig script".

try {
   $subj = "client_certificate_credential_sample"
   $rsa = [RSA]::Create(2048)
   $x509 = New-X509Certificate2 $rsa "CN=$subj"
   $certPath = Join-Path $(Get-Location) "$subj.pem"
   Export-X509Certificate2PEMWithPrivateKey $certPath $x509

   $EnvironmentVariables["AZURE_CLIENT_CERTIFICATE_PATH"] = $certPath
}
finally {
   $rsa.Dispose()
}
