@description('The base resource name.')
param baseName string = resourceGroup().name

@description('The client OID to grant access to test resources.')
param testApplicationOid string

var apiVersion = '2017-04-01'
// Uncomment this if you want to test against the southeastasia nodes.
//var location = 'southeastasia'
var location = resourceGroup().location
var authorizationRuleName_var = '${baseName}/RootManageSharedAccessKey'
var authorizationRuleNameNoManage_var = '${baseName}/NoManage'
var serviceBusDataOwnerRoleId = '/subscriptions/${subscription().subscriptionId}/providers/Microsoft.Authorization/roleDefinitions/090c5cfd-751d-490a-894a-3ce6f1109419'

resource servicebus 'Microsoft.ServiceBus/namespaces@2018-01-01-preview' = {
  name: baseName
  location: location
  sku: {
    name: 'Standard'
    tier: 'Standard'
  }
  properties: {
    zoneRedundant: false
  }
}

output QUEUE_NAME_WITH_SESSIONS string = 'testQueueWithSessions'
