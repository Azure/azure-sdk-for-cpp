# Helper functions for retrieving useful information from azure-sdk-for-* repo
. "${PSScriptRoot}\logging.ps1"
. "${PSScriptRoot}\Helpers\Package-Helpers.ps1"
class PackageProps
{
    [string]$Name
    [string]$Version
    [string]$DevVersion
    [string]$DirectoryPath
    [string]$ServiceDirectory
    [string]$ReadMePath
    [string]$ChangeLogPath
    [string]$Group
    [string]$SdkType
    [boolean]$IsNewSdk
    [string]$ArtifactName
    [string]$ReleaseStatus
    # was this package purely included because other packages included it as an AdditionalValidationPackage?
    [boolean]$IncludedForValidation
    # does this package include other packages that we should trigger validation for?
    [string[]]$AdditionalValidationPackages
    [HashTable]$ArtifactDetails

    PackageProps([string]$name, [string]$version, [string]$directoryPath, [string]$serviceDirectory)
    {
        $this.Initialize($name, $version, $directoryPath, $serviceDirectory)
    }

    PackageProps([string]$name, [string]$version, [string]$directoryPath, [string]$serviceDirectory, [string]$group = "")
    {
        $this.Initialize($name, $version, $directoryPath, $serviceDirectory, $group)
    }

    hidden [void]Initialize(
        [string]$name,
        [string]$version,
        [string]$directoryPath,
        [string]$serviceDirectory
    )
    {
        $this.Name = $name
        $this.Version = $version
        $this.DirectoryPath = $directoryPath
        $this.ServiceDirectory = $serviceDirectory
        $this.IncludedForValidation = $false

        if (Test-Path (Join-Path $directoryPath "README.md"))
        {
            $this.ReadMePath = Join-Path $directoryPath "README.md"
        }
        else
        {
            $this.ReadMePath = $null
        }

        if (Test-Path (Join-Path $directoryPath "CHANGELOG.md"))
        {
            $this.ChangeLogPath = Join-Path $directoryPath "CHANGELOG.md"
            # Get release date for current version and set in package property
            $changeLogEntry = Get-ChangeLogEntry -ChangeLogLocation $this.ChangeLogPath -VersionString $this.Version
            if ($changeLogEntry -and $changeLogEntry.ReleaseStatus)
            {
              $this.ReleaseStatus = $changeLogEntry.ReleaseStatus.Trim().Trim("()")
            }
        }
        else
        {
            $this.ChangeLogPath = $null
        }

        $this.InitializeCIArtifacts()
    }

    hidden [void]Initialize(
        [string]$name,
        [string]$version,
        [string]$directoryPath,
        [string]$serviceDirectory,
        [string]$group
    )
    {
        $this.Initialize($name, $version, $directoryPath, $serviceDirectory)
        $this.Group = $group
    }

    hidden [object]GetValueSafely($hashtable, [string[]]$keys) {
        $current = $hashtable
        foreach ($key in $keys) {
            if ($current.ContainsKey($key) -or $current[$key]) {
                $current = $current[$key]
            }
            else {
                return $null
            }
        }

        return $current
    }

    hidden [HashTable]ParseYmlForArtifact([string]$ymlPath) {
        if (Test-Path -Path $ymlPath) {
            try {
                $content = Get-Content -Raw -Path $ymlPath | CompatibleConvertFrom-Yaml
                if ($content) {
                    $artifacts = $this.GetValueSafely($content, @("extends", "parameters", "Artifacts"))

                    $artifactForCurrentPackage = $artifacts | Where-Object { $_["name"] -eq $this.ArtifactName -or $_["name"] -eq $this.Name }

                    if ($artifactForCurrentPackage.Count -gt 1)
                    {
                        $data = ($artifactForCurrentPackage | % { $_.name }) -join ", "
                        Write-Host "Found more than one project with the name [$($this.Name)]: [ $data ]. Choosing the first one under $($artifactForCurrentPackage[0].DirectoryPath)"
                        return $artifactForCurrentPackage[0]
                    }
                    if ($artifactForCurrentPackage) {
                        Write-Host "Got $artifactForCurrentPackage with type $($artifactForCurrentPackage.GetType())"
                        return [HashTable]$artifactForCurrentPackage
                    }
                    else {
                        "We don't have a matching artifact for $($this.Name) in the yml file $($ymlPath)"
                    }
                }
            }
            catch {
              Write-Host "Exception while parsing yml file $($ymlPath): $_"
            }
        }

        return $null
    }

    [void]InitializeCIArtifacts(){
        $RepoRoot = Resolve-Path (Join-Path $PSScriptRoot ".." ".." "..")
        $ciFilePath = Join-Path -Path $RepoRoot -ChildPath (Join-Path "sdk" $this.ServiceDirectory "ci.yml")
        $ciMgmtYmlFilePath = Join-Path -Path $RepoRoot -ChildPath (Join-Path "sdk" $this.ServiceDirectory "ci.mgmt.yml")

        Write-Host "Calling InitializeCIArtifacts against $($this.Name)"

        if (-not $this.ArtifactDetails) {
            Write-Host "Artifact details for $($this.Name) is not set. Trying to get it from ci.yml."
            $ciArtifactResult = $this.ParseYmlForArtifact($ciFilePath)
            if ($ciArtifactResult) {
                Write-Host "We have a ciArtifactResult: $ciArtifactResult"
                $this.ArtifactDetails = [Hashtable]$ciArtifactResult
            }
            else {
                Write-Host "We don't have an artifact result to assign to $($this.Name)"
            }
        }

        if (-not $this.ArtifactDetails) {
            Write-Host "Artifact details for $($this.Name) is not set. Trying to get it from ci.mgmt.yml."
            $ciMgmtResult = $this.ParseYmlForArtifact($ciMgmtYmlFilePath)

            if ($ciMgmtResult) {
                Write-Host "We have a ciMgmtResult: $ciMgmtResult"
                $this.ArtifactDetails = [Hashtable]$ciMgmtResult
            }
            else {
                Write-Host "We don't have a mgmt artifact result to assign to $($this.ArtifactName)"
            }
        }
    }
}

# Takes package name and service Name
# Returns important properties of the package relative to the language repo
# Returns a PS Object with properties @ { pkgName, pkgVersion, pkgDirectoryPath, pkgReadMePath, pkgChangeLogPath }
# Note: python is required for parsing python package properties.
function Get-PkgProperties
{
    Param
    (
        [Parameter(Mandatory = $true)]
        [string]$PackageName,
        [string]$ServiceDirectory
    )

    $allPkgProps = Get-AllPkgProperties -ServiceDirectory $ServiceDirectory
    $pkgProps = $allPkgProps.Where({ $_.Name -eq $PackageName -or $_.ArtifactName -eq $PackageName });

    if ($pkgProps.Count -ge 1)
    {
        if ($pkgProps.Count -gt 1)
        {
            Write-Host "Found more than one project with the name [$PackageName], choosing the first one under $($pkgProps[0].DirectoryPath)"
        }
        return $pkgProps[0]
    }

    LogError "Failed to retrieve Properties for [$PackageName]"
    return $null
}

function Get-PrPkgProperties([string]$InputDiffJson) {
    $packagesWithChanges = @()

    $allPackageProperties = Get-AllPkgProperties
    $diff = Get-Content $InputDiffJson | ConvertFrom-Json
    $targetedFiles = $diff.ChangedFiles

    $additionalValidationPackages = @()
    $lookup = @{}

    foreach ($pkg in $allPackageProperties)
    {
        $pkgDirectory = Resolve-Path "$($pkg.DirectoryPath)"
        $lookupKey = ($pkg.DirectoryPath).Replace($RepoRoot, "").TrimStart('\/')
        $lookup[$lookupKey] = $pkg

        foreach ($file in $targetedFiles)
        {
            $filePath = Resolve-Path (Join-Path $RepoRoot $file)
            $shouldInclude = $filePath -like "$pkgDirectory*"
            if ($shouldInclude) {
                $packagesWithChanges += $pkg

                if ($pkg.AdditionalValidationPackages) {
                    $additionalValidationPackages += $pkg.AdditionalValidationPackages
                }

                # avoid adding the same package multiple times
                break
            }
        }
    }

    foreach ($addition in $additionalValidationPackages) {
        $key = $addition.Replace($RepoRoot, "").TrimStart('\/')

        if ($lookup[$key]) {
            $lookup[$key].IncludedForValidation = $true
            $packagesWithChanges += $lookup[$key]
        }
    }

    if ($AdditionalValidationPackagesFromPackageSetFn -and (Test-Path "Function:$AdditionalValidationPackagesFromPackageSetFn"))
    {
        $packagesWithChanges += &$AdditionalValidationPackagesFromPackageSetFn $packagesWithChanges $diff $allPackageProperties
    }

    return $packagesWithChanges
}

# Takes ServiceName and Repo Root Directory
# Returns important properties for each package in the specified service, or entire repo if the serviceName is not specified
# Returns a Table of service key to array values of PS Object with properties @ { pkgName, pkgVersion, pkgDirectoryPath, pkgReadMePath, pkgChangeLogPath }
function Get-AllPkgProperties ([string]$ServiceDirectory = $null)
{
    $pkgPropsResult = @()

    if (Test-Path "Function:Get-AllPackageInfoFromRepo")
    {
        $pkgPropsResult = Get-AllPackageInfoFromRepo -ServiceDirectory $serviceDirectory
    }
    else
    {
        if ([string]::IsNullOrEmpty($ServiceDirectory))
        {
            foreach ($dir in (Get-ChildItem (Join-Path $RepoRoot "sdk") -Directory))
            {
                $pkgPropsResult += Get-PkgPropsForEntireService -serviceDirectoryPath $dir.FullName
            }
        }
        else
        {
            $pkgPropsResult = Get-PkgPropsForEntireService -serviceDirectoryPath (Join-Path $RepoRoot "sdk" $ServiceDirectory)
        }
    }

    return $pkgPropsResult
}

# Given the metadata url under https://github.com/Azure/azure-sdk/tree/main/_data/releases/latest,
# the function will return the csv metadata back as part of the response.
function Get-CSVMetadata ([string]$MetadataUri=$MetadataUri)
{
    $metadataResponse = Invoke-RestMethod -Uri $MetadataUri -method "GET" -MaximumRetryCount 3 -RetryIntervalSec 10 | ConvertFrom-Csv
    return $metadataResponse
}

function Get-PkgPropsForEntireService ($serviceDirectoryPath)
{
    $projectProps = @() # Properties from every project in the service
    $serviceDirectory = $serviceDirectoryPath -replace '^.*[\\/]+sdk[\\/]+([^\\/]+).*$', '$1'

    if (!$GetPackageInfoFromRepoFn -or !(Test-Path "Function:$GetPackageInfoFromRepoFn"))
    {
        LogError "The function for '$GetPackageInfoFromRepoFn' was not found.`
        Make sure it is present in eng/scripts/Language-Settings.ps1 and referenced in eng/common/scripts/common.ps1.`
        See https://github.com/Azure/azure-sdk-tools/blob/main/doc/common/common_engsys.md#code-structure"
    }

    foreach ($directory in (Get-ChildItem $serviceDirectoryPath -Directory))
    {
        $pkgProps = &$GetPackageInfoFromRepoFn $directory.FullName $serviceDirectory
        if ($null -ne $pkgProps)
        {
            $projectProps += $pkgProps
        }
    }

    return $projectProps
}
