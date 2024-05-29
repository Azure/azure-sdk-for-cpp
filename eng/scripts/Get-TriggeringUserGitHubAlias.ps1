param(
    [string] $EmailAddress = $env:BUILD_REQUESTEDFOREMAIL,
    [string] $OpenApiToken,
    [string] $Fallback
)

. "$PSScriptRoot/../common/scripts/Helpers/Metadata-Helpers.ps1"

$allUsers = GetAllGitHubUsers -Token $OpenApiToken

if (!$allUsers) {
    Write-Host "Failed to get all GitHub users"
    return $Fallback
}

$targetUser = $allUsers.Where({ $_.aad.userPrincipalName -eq $EmailAddress -and $_.github.login }, 'First')

if (!$targetUser) {
    Write-Host "Failed to find GitHub user for triggering user"
    return $Fallback
}

return "@$($targetUser.github.login)"
