﻿// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace macro_utils_h_generator
{
    class Program
    {
        static void Main(string[] args)
        {
            var macro_utilsTransform = new macro_utils_generated();
            var macro_utilsTransform_text = macro_utilsTransform.TransformText();
            System.IO.File.WriteAllText(@"..\inc\azure_macro_utils\macro_utils_generated.h", macro_utilsTransform_text);

        }
    }
}
