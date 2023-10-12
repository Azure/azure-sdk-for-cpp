param(
    [string] $EmailAddress = $env:BUILD_REQUESTEDFOREMAIL,
    [string] $ClientId,
    [string] $ClientSecret,
    [string] $TenantId,
    [string] $Fallback
)

. "$PSScriptRoot/../common/scripts/Helpers/Metadata-Helpers.ps1"

$allUsers = GetAllGitHubUsers `
    -TenantId $TenantId `
    -ClientId $ClientId `
    -ClientSecret $ClientSecret

if (!$allUsers) {
    Write-Host "Failed to get all GitHub users"
    return $Fallback
}

$targetUser = $allUsers.Where({ $_.aad.userPrincipalName -eq $EmailAddress -and $_.github.login }, 'First')

if (!$targetUser) {
    Write-Host "Failed to find GitHub user for $EmailAddress"
    return $Fallback
}

return "@$($targetUser.github.login)"
