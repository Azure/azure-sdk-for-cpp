@description('The base resource name.')
param baseName string = resourceGroup().name

@description('The location for all resources.')
param location string = resourceGroup().location


@description('Storage endpoint suffix. The default value uses Azure Public Cloud (core.windows.net)')
param storageEndpointSuffix string = 'core.windows.net'

var apiVersion = '2017-04-01'
var storageApiVersion = '2019-04-01'
//var iotApiVersion = '2018-04-01'
var namespaceName = baseName
var storageAccountName = 'storage${baseName}'
var containerName = 'container'
//var iotName = 'iot${baseName}'
var authorizationName = '${baseName}/RootManageSharedAccessKey'
var eventHubName = 'eventhub'
var eventHubFullName = '${baseName}/eventhub'
//var location = resourceGroup().location

resource namespace 'Microsoft.EventHub/namespaces@2017-04-01' = {
  name: namespaceName
  location: location
  sku: {
    name: 'Standard'
    tier: 'Standard'
    capacity: 5
  }
  properties: {
//    zoneRedundant: false
    isAutoInflateEnabled: false
    maximumThroughputUnits: 0
  }
}

resource authorization 'Microsoft.EventHub/namespaces/AuthorizationRules@2017-04-01' = {
  name: authorizationName
//  location: location
  properties: {
    rights: [
      'Listen'
      'Manage'
      'Send'
    ]
  }
  dependsOn: [
    namespace
  ]
}

resource eventHubNameFull 'Microsoft.EventHub/namespaces/eventhubs@2017-04-01' = {
  name: eventHubFullName
//  location: location
  properties: {
    messageRetentionInDays: 7
    partitionCount: 32
  }
  dependsOn: [
    namespace
  ]
}

resource namespaceName_default 'Microsoft.EventHub/namespaces/networkRuleSets@2017-04-01' = {
  parent:namespace
  name: 'default'
//  location: location
  properties: {
    defaultAction: 'Deny'
    virtualNetworkRules: []
    ipRules: []
  }
  dependsOn: [
  ]
}

resource eventHubNameFull_Default 'Microsoft.EventHub/namespaces/eventhubs/consumergroups@2017-04-01' = {
  parent: eventHubNameFull
  name: '$Default'
//  location: location
  properties: {}
  dependsOn: [
    namespace
  ]
}

resource storageAccount 'Microsoft.Storage/storageAccounts@2019-04-01' = {
  name: storageAccountName
  location: location
  sku: {
    name: 'Standard_RAGRS'
//    tier: 'Standard'
  }
  kind: 'StorageV2'
  properties: {
    networkAcls: {
      bypass: 'AzureServices'
      virtualNetworkRules: []
      ipRules: []
      defaultAction: 'Allow'
    }
    supportsHttpsTrafficOnly: true
    encryption: {
      services: {
        file: {
          enabled: true
        }
        blob: {
          enabled: true
        }
      }
      keySource: 'Microsoft.Storage'
    }
    accessTier: 'Hot'
  }
}

resource storageAccountName_default_container 'Microsoft.Storage/storageAccounts/blobServices/containers@2019-04-01' = {
  name: '${storageAccountName}/default/${containerName}'
  dependsOn: [
    storageAccount
  ]
}

output EVENTHUB_NAME string = eventHubName
output EVENTHUB_CONNECTION_STRING string = '"${listKeys(resourceId('Microsoft.EventHub/namespaces/authorizationRules', namespaceName, 'RootManageSharedAccessKey'), apiVersion).primaryConnectionString}"'
output CHECKPOINTSTORE_STORAGE_CONNECTION_STRING string = '"DefaultEndpointsProtocol=https;AccountName=${storageAccountName};AccountKey=${listKeys(storageAccount.id, storageApiVersion).keys[0].value};EndpointSuffix=${storageEndpointSuffix}"'
