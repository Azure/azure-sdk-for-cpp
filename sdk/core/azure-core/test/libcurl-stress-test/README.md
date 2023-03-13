# Stress test prototype 
This is work in progress. It's a prototype of how a stress test would look. This PR is to park the work in progress while being dealing with higher priority issues.
## Components 
### Code (https://en.wikipedia.org/wiki/C%2B%2B)
The cpp file represents the code for the test, it will generate a number of invalid URLs and then issue CURL send commands. The requests are expected to fail. The point was that it exposes memory leaks in handling the error cases, which we fixed since. 

### Dockerfile (https://www.docker.com/)
Represents the build file for the container in which the test runs, it is based on Ubuntu 22.04 , from MCR. 
The main change from default Ubuntu is making sure we have the Valgrind tool installed. Valgrind is a heap monitoring tool that helps identify potential stack traces that might leak memory. While not 100% effective is great at reducing the surface are for investigations. 

### Helm chart (https://helm.sh/)
Chart.yaml together with the bicep file(https://docs.microsoft.com/azure/azure-resource-manager/bicep/overview?tabs=bicep) and the deploy job file , represent the helm chart needed to deploy to the docker image built from the dockerfile to the stress cluster and execute the stress test. 

The helm chart creates a pod with a container based on the docker image, and executes the test under Valgrind. 

To deploy the chart you will need to run "azure-sdk-for-cpp\eng\common\scripts\stress-testing> .\deploy-stress-tests.ps1 -Namespace azuresdkforcpp -SearchDirectory E:\src\azure-sdk-for-cpp\sdk\core\azure-core\test -PushImage"

Where namespace will be created if missing , search directory can be any folder where it will search for charts in it and all it's sub dirs, push image will call it to build the docker image. 

ATM the docker image is build by hand and hard-coded in the chart to simplify matters.  

To build the image run "docker build -t <acr>/azuresdkforcpp/curlstress:v8  --build-arg targetTest=azure-core-libcurl-stress-test --build-arg build=on  ."

To push to mcr : "docker push <acr>/azuresdkforcpp/curlstress:v8"
Obviously after logging in to the acr "az acr login -n <acr>"

To use another image you will need to go to line 12 in deploy job and update with your new file. 

Once the deploy succeeds run " kubectl logs -n azuresdkforcpp -f libcurl-stress-test" to grab the logs in real time .