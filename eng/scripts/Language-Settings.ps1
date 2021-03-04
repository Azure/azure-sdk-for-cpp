$Language = "cpp"
$LanguageDisplayName = "C++"
$PackageRepository = "CPP"
$packagePattern = "*.json"
$MetadataUri = "https://raw.githubusercontent.com/Azure/azure-sdk/master/_data/releases/latest/cpp-packages.csv"
$BlobStorageUrl = "https://azuresdkdocs.blob.core.windows.net/%24web?restype=container&comp=list&prefix=cpp%2F&delimiter=%2F"



function Get-cpp-PackageInfoFromRepo($pkgPath, $serviceDirectory, $pkgName) 
{
  # Test if the package path ends with the package name (e.g. sdk/storage/azure-storage-common)
  # This function runs in a loop where $pkgPath might be the path to the package and must return 
  # $null in cases where $pkgPath is not the path to the package specified by $pkgName
  if ($pkgName -and ($pkgName -ne (Split-Path -Leaf $pkgPath))) { 
    return $null
  }

  if (!$pkgName)
  {
    $pkgName = Split-Path -Leaf $pkgPath
  }

  $packageVersion = & $PSScriptRoot/Get-PkgVersion.ps1 -ServiceDirectory $serviceDirectory -PackageName $pkgName
  if ($null -ne $packageVersion)
  {
    $packageProps = [PackageProps]::new($pkgName, $packageVersion, $pkgPath, $serviceDirectory)
    $packageProps.ArtifactName = $pkgName
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
    # Artifact info is always considered deployable for now becasue it is not
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

function Get-cpp-GithubIoDocIndex() {
  # Update the main.js and docfx.json language content
  UpdateDocIndexFiles -appTitleLang "C++"
  # Fetch out all package metadata from csv file.
  $metadata = Get-CSVMetadata -MetadataUri $MetadataUri
  # Get the artifacts name from blob storage
  $artifacts =  Get-BlobStorage-Artifacts -blobStorageUrl $BlobStorageUrl -blobDirectoryRegex "^cpp/(.*)/$" -blobArtifactsReplacement '$1'
  # Build up the artifact to service name mapping for GithubIo toc.
  $tocContent = Get-TocMapping -metadata $metadata -artifacts $artifacts
  # Generate yml/md toc files and build site.
  GenerateDocfxTocContent -tocContent $tocContent -lang "C++"
}
