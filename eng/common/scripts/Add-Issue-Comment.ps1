[CmdletBinding(SupportsShouldProcess = $true)]
param(
  [Parameter(Mandatory = $true)]
  [string]$RepoOwner,

  [Parameter(Mandatory = $true)]
  [string]$RepoName,

  [Parameter(Mandatory = $true)]
  [string]$IssueNumber,

  [Parameter(Mandatory = $true)]
  [string]$Comment,

  [Parameter(Mandatory = $true)]
  [string]$AuthToken
)

. "${PSScriptRoot}\common.ps1"

try {
  AddIssueComment -RepoOwner $RepoOwner -RepoName $RepoName `
  -IssueNumber $IssueNumber -Comment $Comment -AuthToken $AuthToken
}
catch {
  LogError "AddIssueComment failed with exception:`n$_"
  exit 1
}