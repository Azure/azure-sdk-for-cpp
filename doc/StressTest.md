# _Stress Test

## Dependencies
Adding a stress test requires the following tools installed:
* Docker : https://www.docker.com/
* Helm : https://helm.sh/


Structure : 

A stress tests is composes of the following components : 
* Source code : represents the test that is run, there are no prerequisites as to what the code does, the outputs of the tests can be at the console , as file share or Application Insights. 
* DockerFile : represents the docker file that will build the container image that  will be executed by the stress automation, on the stress test clusters. 
* Helm Chart : represents the deployment script for the docker image built by the Dockerfile. 
* stress-test-resources.bicep/json ARM template for deploying azure resources required by the 
tests 
* scenarios-matrix.yaml : definition of tests and which are offered in one docker image. e.g. We can have multiple tests coexisting in a single docker image, the definition contains a list of test names and executables (with params) that will be executed in order to run those tests.

To deploy a stress test to the playground (test) clusters, you will need to execute the following script : "eng/common/scripts/stress-testing/deploy-stress-tests.ps1 -Login -PushImages" in the folder of where the dockerfile exists. 

Example : \sdk\core\azure-core\test\libcurl-stress-test

The deployment script will run the following steps : 
* deploy the arm resources
* build the docker image 
* read the scenarios and generate the final helm chart 
* deploy the helm chart - caveat - make sure to have a .helmignore file present in which you exclude files that are not required for the deployment (e.g. test source code) . When initiating a deployment helm zips the whole folder , in order to upload it the chart needs to be less than 1MB. Thus excluding any unneeded files is advisable. 

The Dockerfile is very flexible, it can be used to include the binaries for the tests or include the code and run the config/build of the tests as part of the build , or anything in between. The only requirement is that the command lines to run the test executables are specified in the scenarios.txt
