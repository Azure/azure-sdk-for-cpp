# Release History

## 1.0.0-beta.3 (2020-10-13)

* BlockBlob::Upload, AppendBlob::Create and PageBlob::Create are now by default non-overwriting
* Variable name change: BreakContainerLeaseOptions::breakPeriod -> BreakContainerLeaseOptions::BreakPeriod
* Variable name change: BreakBlobLeaseOptions::breakPeriod -> BreakBlobLeaseOptions::BreakPeriod
* CreateFromConnectionString now accepts unencoded blob name
* TagConditions is changed to nullable