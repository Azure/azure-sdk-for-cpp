// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"
#include "samples_common.hpp"

SAMPLE(DataLakeGettingStarted, DataLakeGettingStarted)
void DataLakeGettingStarted()
{
  using namespace Azure::Storage::Files::DataLake;
  auto client = ServiceClient::CreateFromConnectionString(GetConnectionString());
  auto response = client.ListFileSystems();
}
