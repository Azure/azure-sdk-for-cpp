// cspell: words azuresdkforcppproxy azuresdkforcppauthproxy
@description('Name for the container group')
param anonymousName string = 'azuresdkforcppproxy'

@description('Name for the container group')
param authenticatedName string = 'azuresdkforcppauthproxy'

@description('Location for all resources.')
param location string = resourceGroup().location

@description('Container image to deploy. Should be of the form repoName/imagename:tag for images stored in public Docker Hub, or a fully qualified URI for other registries. Images from private registries require additional registry credentials.')
param anonymousImage string = 'azsdkengsys.azurecr.io/cpp/squid:latest'

@description('Container image to deploy. Should be of the form repoName/imagename:tag for images stored in public Docker Hub, or a fully qualified URI for other registries. Images from private registries require additional registry credentials.')
param authenticatedImage string = 'azsdkengsys.azurecr.io/cpp/squid.passwd:latest'

@description('Port to open on the container and the public IP address.')
param anonymousPort int = 3128
@description('Port to open on the container and the public IP address.')
param authenticatedPort int = 3129

@description('The number of CPU cores to allocate to the container.')
param cpuCores int = 1

@description('The amount of memory to allocate to the container in gigabytes.')
param memoryInGb int = 2

@description('The behavior of Azure runtime if container has stopped.')
@allowed([
  'Always'
  'Never'
  'OnFailure'
])
param restartPolicy string = 'Always'

resource anonymousContainerGroup 'Microsoft.ContainerInstance/containerGroups@2021-09-01' = {
  name: anonymousName
  location: location
  properties: {
    containers: [
      {
        name: anonymousName
        properties: {
          image: anonymousImage
          ports: [
            {
              port: anonymousPort
              protocol: 'TCP'
            }
          ]
          resources: {
            requests: {
              cpu: cpuCores
              memoryInGB: memoryInGb
            }
          }
        }
      }
    ]
    osType: 'Linux'
    restartPolicy: restartPolicy
    ipAddress: {
      type: 'Public'
      ports: [
        {
          port: anonymousPort
          protocol: 'TCP'
        }
      ]
    }
  }
}

resource authenticatedContainerGroup 'Microsoft.ContainerInstance/containerGroups@2021-09-01' = {
  name: authenticatedName
  location: location
  properties: {
    containers: [
      {
        name: authenticatedName
        properties: {
          image: authenticatedImage
          ports: [
            {
              port: authenticatedPort
              protocol: 'TCP'
            }
          ]
          resources: {
            requests: {
              cpu: cpuCores
              memoryInGB: memoryInGb
            }
          }
        }
      }
    ]
    osType: 'Linux'
    restartPolicy: restartPolicy
    ipAddress: {
      type: 'Public'
      ports: [
        {
          port: authenticatedPort
          protocol: 'TCP'
        }
      ]
    }
  }
}

output anonymousContainerIPv4Address string = anonymousContainerGroup.properties.ipAddress.ip
output authenticatedContainerIPv4Address string = authenticatedContainerGroup.properties.ipAddress.ip
