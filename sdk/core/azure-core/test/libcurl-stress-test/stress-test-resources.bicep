// Dummy parameter to handle defaults the script passes in
param testApplicationOid string = ''

output RESOURCE_GROUP string = resourceGroup().name
output AZURE_CLIENT_OID string = testApplicationOid
