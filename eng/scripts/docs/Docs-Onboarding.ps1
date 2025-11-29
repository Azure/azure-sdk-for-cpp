. "$PSScriptRoot/Docs-ToC.ps1"

# $SetDocsPackageOnboarding = "Set-${Language}-DocsPackageOnboarding"
function Set-cpp-DocsPackageOnboarding($moniker, $metadata, $docRepoLocation, $packageSourceOverride) {
  $onboardingFile = GetOnboardingFile `
    -docRepoLocation $docRepoLocation `
    -moniker $moniker

  $onboardingSpec = Get-Content $onboardingFile -Raw | ConvertFrom-Json -AsHashtable

  $packagesToOnboard = @()
  foreach ($package in $metadata) { 
    $packageSpec = [ordered]@{
      name = $package.Name
      tool = 'cpp-doxygen-1.9.1'
      url = "https://azuresdkdocs.z19.web.core.windows.net/cpp/$($package.Name)/$($package.Version)/annotated.html"
    }

    $packagesToOnboard += $packageSpec
  }

  $onboardingSpec['packages'] = $packagesToOnboard

  Set-Content `
    -Path $onboardingFile `
    -Value ($onboardingSpec | ConvertTo-Json -Depth 100)
}

# $GetDocsPackagesAlreadyOnboarded = "Get-${Language}-DocsPackagesAlreadyOnboarded"
function Get-cpp-DocsPackagesAlreadyOnboarded($docRepoLocation, $moniker) {
  $packageOnboardingFile = GetOnboardingFile `
    -docRepoLocation $DocRepoLocation `
    -moniker $moniker
  
  $onboardedPackages = @{}
  $onboardingSpec = ConvertFrom-Json (Get-Content $packageOnboardingFile -Raw)
  foreach ($spec in $onboardingSpec.packages) {
    $packageInfo = GetPackageInfoFromDocsMsConfig $spec
    $onboardedPackages[$packageInfo.Name] = $packageInfo
  }

  return $onboardedPackages
}

function GetPackageInfoFromDocsMsConfig($spec) {
  if (!$spec) { 
    throw "Spec must not be empty"
  }

  # "https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-security-attestation/1.1.0/annotated.html" => 1.1.0
  $version = $spec.url.Split('/')[-2]
  if (!$version) {
    throw "Could not determine version from URL: $($spec.url)"
  }

  return @{
    Name    = $spec.name
    Version = $version
  }
}
