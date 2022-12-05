$Language = "cpp"
$LanguageDisplayName = "C++"
$PackageRepository = "CPP"
$packagePattern = "package-info.json"
$MetadataUri = "https://raw.githubusercontent.com/Azure/azure-sdk/main/_data/releases/latest/cpp-packages.csv"
$BlobStorageUrl = "https://azuresdkdocs.blob.core.windows.net/%24web?restype=container&comp=list&prefix=cpp%2F&delimiter=%2F"

$VersionRegex = '(#define AZURE_\w+_VERSION_MAJOR )(?<major>[0-9]+)(\s+#define AZURE_\w+_VERSION_MINOR )(?<minor>[0-9]+)(\s+#define AZURE_\w+_VERSION_PATCH )(?<patch>[0-9]+)(\s+#define AZURE_\w+_VERSION_PRERELEASE )"(?<prerelease>[a-zA-Z0-9.]*)"';
function Get-VersionHppLocation ($ServiceDirectory, $PackageName) {
    $versionHppLocation = Get-ChildItem package_version.hpp -Path "$RepoRoot/sdk/$ServiceDirectory/$PackageName" -Recurse
    Write-Verbose "package_version.hpp location: $versionHppLocation"
    return $versionHppLocation
}

function Get-cpp-PackageInfoFromRepo($pkgPath, $serviceDirectory)
{
  $pkgName = Split-Path -Leaf $pkgPath
  $packageVersion = & $PSScriptRoot/Get-PkgVersion.ps1 -ServiceDirectory $serviceDirectory -PackageName $pkgName
  if ($null -ne $packageVersion)
  {
    $packageProps = [PackageProps]::new($pkgName, $packageVersion, $pkgPath, $serviceDirectory)
    $packageProps.ArtifactName = $pkgName
    $packageProps.IsNewSDK = "true"
    $packageProps.SdkType = "client"
    return $packageProps
  }
  return $null
}

# Parse out package publishing information from a package-info.json file.
function Get-cpp-PackageInfoFromPackageFile($pkg, $workingDirectory)
{
  $packageInfo = Get-Content -Raw -Path $pkg | ConvertFrom-Json
  $packageArtifactLocation = (Get-ItemProperty $pkg).Directory.FullName
  $releaseNotes = ""
  $readmeContent = ""

  $pkgVersion = $packageInfo.version
  $pkgName = $packageInfo.name

  $changeLogLoc = @(Get-ChildItem -Path $packageArtifactLocation -Recurse -Include "CHANGELOG.md")[0]
  if ($changeLogLoc)
  {
    $releaseNotes = Get-ChangeLogEntryAsString -ChangeLogLocation $changeLogLoc -VersionString $pkgVersion
  }

  $readmeContentLoc = @(Get-ChildItem -Path $packageArtifactLocation -Recurse -Include "README.md")[0]
  if ($readmeContentLoc)
  {
    $readmeContent = Get-Content -Raw $readmeContentLoc
  }

  return New-Object PSObject -Property @{
    PackageId      = $pkgName
    PackageVersion = $pkgVersion
    ReleaseTag     = "${pkgName}_${pkgVersion}"
    # Artifact info is always considered deployable for now because it is not
    # deployed anywhere. Dealing with duplicate tags happens downstream in
    # CheckArtifactShaAgainstTagsList
    Deployable     = $true
    ReleaseNotes   = $releaseNotes
  }
}

# Stage and Upload Docs to blob Storage
function Publish-cpp-GithubIODocs ($DocLocation, $PublicArtifactLocation)
{
  $packageInfo = (Get-Content (Join-Path $DocLocation 'package-info.json') | ConvertFrom-Json)
  $releaseTag = RetrieveReleaseTag $PublicArtifactLocation
  Upload-Blobs -DocDir $DocLocation -PkgName $packageInfo.name -DocVersion $packageInfo.version -ReleaseTag $releaseTag
}

function Get-cpp-GithubIoDocIndex()
{
  # Update the main.js and docfx.json language content
  UpdateDocIndexFiles -appTitleLang "C++"
  # Fetch out all package metadata from csv file.
  $metadata = Get-CSVMetadata -MetadataUri $MetadataUri
  # Get the artifacts name from blob storage
  $artifacts =  Get-BlobStorage-Artifacts -blobStorageUrl $BlobStorageUrl -blobDirectoryRegex "^cpp/(.*)/$" -blobArtifactsReplacement '$1'
  # Build up the artifact to service name mapping for GithubIo toc.
  $tocContent = Get-TocMapping -metadata $metadata -artifacts $artifacts
  # Generate yml/md toc files and build site.
  GenerateDocfxTocContent -tocContent $tocContent -lang "C++" -campaignId "UA-62780441-44"
}

function SetPackageVersion ($PackageName, $Version, $ServiceDirectory, $ReleaseDate, $ReplaceLatestEntryTitle=$true)
{
  if($null -eq $ReleaseDate)
  {
    $ReleaseDate = Get-Date -Format "yyyy-MM-dd"
  }

  & "$EngDir/scripts/Update-PkgVersion.ps1" `
    -ServiceDirectory $ServiceDirectory `
    -PackageName $PackageName `
    -NewVersionString $Version `
    -ReleaseDate $ReleaseDate `
    -ReplaceLatestEntryTitle $ReplaceLatestEntryTitle
}

function Find-cpp-Artifacts-For-Apireview($ArtifactPath, $PackageName)
{
  $artifact = Get-ChildItem -Path (Join-Path $ArtifactPath $PackageName) -Filter "*.cppast"
  if ($artifact)
  {
    $packages = @{
      $artifact.FullName = $artifact.FullName
    }
    return $packages
  }
  return $null
}