// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"

#include <fstream>
#include <iostream>

int main()
{
  using namespace Azure::Storage::DataLake;
  auto client = ServiceClient::CreateFromConnectionString("");
  auto response = client.ListFileSystems();
  return 0;
}
