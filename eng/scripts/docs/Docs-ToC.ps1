function GetOnboardingFile($docRepoLocation, $moniker) { 
  $packageOnboardingFile = "$docRepoLocation/ci-configs/packages-latest.json"
  if ("preview" -eq $moniker) {
    $packageOnboardingFile = "$docRepoLocation/ci-configs/packages-preview.json"
  }

  return $packageOnboardingFile
}

# $GetOnboardedDocsMsPackagesFn = "Get-${Language}-OnboardedDocsMsPackages"
function Get-cpp-OnboardedDocsMsPackages($DocRepoLocation) {
  $packageOnboardingFiles = @(
    "$DocRepoLocation/ci-configs/packages-latest.json",
    "$DocRepoLocation/ci-configs/packages-preview.json")

  $onboardedPackages = @{}
  foreach ($file in $packageOnboardingFiles) {
    $onboardingSpec = ConvertFrom-Json (Get-Content $file -Raw)
    foreach ($spec in $onboardingSpec.packages) {
      $packageName = $spec.name
      $onboardedPackages[$packageName] = $null
    }
  }

  return $onboardedPackages
}

# $GetOnboardedDocsMsPackagesForMonikerFn = "Get-${Language}-OnboardedDocsMsPackagesForMoniker"
function Get-cpp-OnboardedDocsMsPackagesForMoniker($DocRepoLocation, $moniker) {
  $packageOnboardingFile = GetOnboardingFile `
    -docRepoLocation $DocRepoLocation `
    -moniker $moniker
  
  $onboardedPackages = @{}
  $onboardingSpec = ConvertFrom-Json (Get-Content $packageOnboardingFile -Raw)
  foreach ($spec in $onboardingSpec.packages) {
    $jsonFile = "$DocRepoLocation/metadata/$($moniker)/$($spec.name).json"
    if (Test-Path $jsonFile) {
      $onboardedPackages[$spec.name] = ConvertFrom-Json (Get-Content $jsonFile -Raw)
    }
    else {
      $onboardedPackages[$spec.name] = $null
    }
  }

  return $onboardedPackages
}

# $GetPackageLevelReadmeFn = "Get-${Language}-PackageLevelReadme"
function Get-cpp-PackageLevelReadme($packageMetadata) {
  return GetPackageReadmeName -packageMetadata $packageMetadata
}

function GetPackageReadmeName($packageMetadata) {
    # Fallback to get package-level readme name if metadata file info does not exist
    $packageLevelReadmeName = $packageMetadata.Package.ToLower().Replace('azure-', '')
  
    # If there is a metadata json for the package use the DocsMsReadmeName from
    # the metadata function
    if ($packageMetadata.PSObject.Members.Name -contains "FileMetadata") {
      $readmeMetadata = &$GetDocsMsMetadataForPackageFn -PackageInfo $packageMetadata.FileMetadata
      $packageLevelReadmeName = $readmeMetadata.DocsMsReadMeName
    }
    return $packageLevelReadmeName
}

# $GetDocsMsTocDataFn = "Get-${Language}-DocsMsTocData"
function Get-cpp-DocsMsTocData($packageMetadata, $docRepoLocation) {
  $packageLevelReadmeName = GetPackageReadmeName -packageMetadata $packageMetadata
  $packageTocHeader = GetDocsTocDisplayName $packageMetadata
  $output = [PSCustomObject]@{
    PackageLevelReadmeHref = "~/docs-ref-services/{moniker}/$packageLevelReadmeName-readme.md"
    PackageTocHeader       = $packageTocHeader
    TocChildren            = @($packageMetadata.Package)
  }

  return $output
}

# TODO
# $GetRepositoryLinkFn = "Get-${Language}-RepositoryLink"
function Get-cpp-RepositoryLink($packageInfo) {
  return "$PackageRepositoryUri/$($packageInfo.Package)-cpp"
}


