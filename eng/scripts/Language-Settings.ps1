$Language = "cpp"
$PackageRepository = "CPP"
$packagePattern = "*.json"
$MetadataUri = ""


# Parse out package publishing information given a nupkg ZIP format.
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
    PackageId $pkgName
    PackageVersion $pkgVersion
    # Artifact info is always considered deployable for now becasue it is not
    # deployed anywhere. Dealing with duplicate tags happens downstream in
    # CheckArtifactShaAgainstTagsList
    Deployable = $true
    ReleaseNotes = $releaseNotes
  }
}

# Stage and Upload Docs to blob Storage
function Publish-cpp-GithubIODocs ()
{
  $packageInfo = (Get-Content (Join-Path $DocLocation 'package-info.json') | ConvertFrom-Json)
  $releaseTag = RetrieveReleaseTag "CPP" $PublicArtifactLocation
  Upload-Blobs -DocDir $DocLocation -PkgName $packageInfo.name -DocVersion $packageInfo.version -ReleaseTag $releaseTag
}