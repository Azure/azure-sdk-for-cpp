# Get Environment Variable for Samples

This is an tool that helps cleanup key vault resources. 
It lists and then starts the delete process on keys secrets and certificates.
Once that is done it will call purge on the deleted resources. 
While on large number of resources present in the key vault it might take a while , and be slower than deleting and recreating a key vault resource, it helps not having to reconfigure and update the connection settings. 
