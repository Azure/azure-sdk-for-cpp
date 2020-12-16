# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

#############  Collect Changelog script ###################
#
# This script helps to create the Language release notes by collectiong each service's release notes
# and generating one only release notes with the required format and links to be used to publish.
#
# Usage:
#  - Run from powershell terminal: `pwsh ./Collect-Changelogs.ps1`
#  - Enter the Month as a number (i.e. use 11 for November)
#
# That's it, the script will use the current year and print out the Release Notes in the standard output.
#
###########################################################

[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)]
  [ValidateRange(1, 12)]
  [int] $Month
)

$repoRoot = Resolve-Path "$PSScriptRoot/../..";
. ${repoRoot}\eng\common\scripts\SemVer.ps1
. ${repoRoot}\eng\common\scripts\ChangeLog-Operations.ps1
$InstallNotes = "";
$ReleaseNotes = "";

$date = Get-Date -Month $month -Format "yyyy-MM"
$date += "-\d\d"

Get-ChildItem "$repoRoot/sdk" -Filter CHANGELOG.md -Recurse | Sort-Object -Property Name | % {
    
    $changeLogEntries = Get-ChangeLogEntries -ChangeLogLocation $_ 
    $package = $_.Directory.Name
    $serviceDirectory = $_.Directory.Parent.Name

    foreach ($changeLogEntry in $changeLogEntries.Values)
    {
        if ($changeLogEntry.ReleaseStatus -notmatch $date)
        {
            
            continue;
        }

        $version = $changeLogEntry.ReleaseVersion
        $githubAnchor = $changeLogEntry.ReleaseTitle.Replace("## ", "").Replace(".", "").Replace("(", "").Replace(")", "").Replace(" ", "-")

        $ReleaseNotes += "### $package [Changelog](https://github.com/Azure/azure-sdk-for-cpp/blob/master/sdk/$serviceDirectory/$package/CHANGELOG.md#$githubAnchor)`n"
        $changeLogEntry.ReleaseContent | %{ 

            $ReleaseNotes += $_.Replace("###", "####")
            $ReleaseNotes += "`n"            
        }
        $ReleaseNotes += "`n"
    }
}

return $ReleaseNotes
