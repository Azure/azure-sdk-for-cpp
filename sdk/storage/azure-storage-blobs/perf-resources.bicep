param baseName string = resourceGroup().name
param testApplicationOid string
param location string = resourceGroup().location


var blobDataContributorRoleId = 'ba92f5b4-2d11-453d-a403-e96b0029c9fe'
var blobDataOwnerRoleId = 'b7e6dc6d-f1e8-4753-8033-0f276bb0955b'

resource blobDataContributor 'Microsoft.Authorization/roleAssignments@2022-04-01' = {
  name: guid(blobDataContributorRoleId, resourceGroup().id)
  properties: {
    roleDefinitionId: resourceId('Microsoft.Authorization/roleDefinitions', blobDataContributorRoleId)
    principalId: testApplicationOid
  }
}

resource blobDataOwner 'Microsoft.Authorization/roleAssignments@2022-04-01' = {
  name: guid(blobDataOwnerRoleId, resourceGroup().id)
  properties: {
    roleDefinitionId: resourceId('Microsoft.Authorization/roleDefinitions', blobDataOwnerRoleId)
    principalId: testApplicationOid
  }
}

resource storage 'Microsoft.Storage/storageAccounts@2024-01-01' = {
  name: '${baseName}blob'
  location: location
  kind: 'BlockBlobStorage'
  sku: {
    name: 'Premium_LRS'
  }
  properties: {
    allowSharedKeyAccess: false
    publicNetworkAccess: null
    supportsHttpsTrafficOnly: true
     
  }
}

var name = storageAccount.name
var key = storageAccount.listKeys().keys[0].value
var connectionString = 'DefaultEndpointsProtocol=https;AccountName=${name};AccountKey=${key}'

output AZURE_STORAGE_ACCOUNT_NAME string = name
output AZURE_STORAGE_ACCOUNT_KEY string = key
output AZURE_STORAGE_CONNECTION_STRING string = connectionString
output STANDARD_STORAGE_CONNECTION_STRING string = connectionString
output STORAGE_CONNECTION_STRING string = connectionString
