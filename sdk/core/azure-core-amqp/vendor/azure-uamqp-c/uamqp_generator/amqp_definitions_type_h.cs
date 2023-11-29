﻿// ------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version: 15.0.0.0
//  
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
// ------------------------------------------------------------------------------
namespace amqplib_generator
{
    using System.Linq;
    using System.Text;
    using System.Collections.Generic;
    using amqplib_generator;
    using System;
    
    /// <summary>
    /// Class to produce the template output
    /// </summary>
    
    #line 1 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.VisualStudio.TextTemplating", "15.0.0.0")]
    public partial class amqp_definitions_type_h : amqp_definitions_type_hBase
    {
#line hidden
        /// <summary>
        /// Create the template output
        /// </summary>
        public virtual string TransformText()
        {
            this.Write("\r\n");
            
            #line 8 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
 amqp amqp = Program.LoadAMQPTypes(); 
            
            #line default
            #line hidden
            this.Write(@"
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is generated. DO NOT EDIT it manually.
// The generator that produces it is located at /uamqp_generator/uamqp_generator.sln

");
            
            #line 16 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
 type type = Program.CurrentTypeObject; 
            
            #line default
            #line hidden
            
            #line 17 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
 string type_name = type.name.ToLower().Replace('-', '_'); 
            
            #line default
            #line hidden
            
            #line 18 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
 string amqpDefinitionName = $"AMQP_DEFINITIONS_{type_name.ToUpper()}_H"; 
            
            #line default
            #line hidden
            this.Write("#ifndef ");
            
            #line 19 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(amqpDefinitionName));
            
            #line default
            #line hidden
            this.Write("\r\n#define ");
            
            #line 20 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(amqpDefinitionName));
            
            #line default
            #line hidden
            this.Write("\r\n\r\n\r\n#ifdef __cplusplus\r\n#include <cstdint>\r\nextern \"C\" {\r\n#else\r\n#include <stdi" +
                    "nt.h>\r\n#include <stdbool.h>\r\n#endif\r\n\r\n#include \"azure_uamqp_c/amqpvalue.h\"\r\n#in" +
                    "clude \"azure_c_shared_utility/umock_c_prod.h\"\r\n\r\n");
            
            #line 34 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          if (type.@class == typeClass.composite) 
            
            #line default
            #line hidden
            
            #line 35 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          { 
            
            #line default
            #line hidden
            this.Write("    typedef struct ");
            
            #line 36 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_INSTANCE_TAG* ");
            
            #line 36 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE;\r\n\r\n");
            
            #line 38 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              string arg_list = Program.GetMandatoryArgListMock(type); 
            
            #line default
            #line hidden
            this.Write("    MOCKABLE_FUNCTION(, ");
            
            #line 39 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 39 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_create ");
            
            #line 39 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(arg_list));
            
            #line default
            #line hidden
            this.Write(");\r\n    MOCKABLE_FUNCTION(, ");
            
            #line 40 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 40 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_clone, ");
            
            #line 40 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, value);\r\n    MOCKABLE_FUNCTION(, void, ");
            
            #line 41 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_destroy, ");
            
            #line 41 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 41 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(");\r\n    MOCKABLE_FUNCTION(, bool, is_");
            
            #line 42 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_type_by_descriptor, AMQP_VALUE, descriptor);\r\n    MOCKABLE_FUNCTION(, int, amqpv" +
                    "alue_get_");
            
            #line 43 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(", AMQP_VALUE, value, ");
            
            #line 43 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE*, ");
            
            #line 43 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_handle);\r\n    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_");
            
            #line 44 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 44 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 44 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(");\r\n\r\n");
            
            #line 46 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              foreach (field field in type.Items.Where(item => item is field)) 
            
            #line default
            #line hidden
            
            #line 47 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              { 
            
            #line default
            #line hidden
            
            #line 48 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  string field_name = field.name.ToLower().Replace('-', '_'); 
            
            #line default
            #line hidden
            
            #line 49 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  string c_type = Program.GetCType(field.type, field.multiple == "true").Replace('-', '_').Replace(':', '_'); 
            
            #line default
            #line hidden
            
            #line 50 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  type field_type = Program.GetTypeByName(field.type); 
            
            #line default
            #line hidden
            
            #line 51 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  if ((field_type != null) && (field_type.@class == typeClass.composite)) c_type = field_type.name.ToUpper().Replace('-', '_').Replace(':', '_') + "_HANDLE"; 
            
            #line default
            #line hidden
            this.Write("    MOCKABLE_FUNCTION(, int, ");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_get_");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(field_name));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(c_type));
            
            #line default
            #line hidden
            this.Write("*, ");
            
            #line 52 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(field_name));
            
            #line default
            #line hidden
            this.Write("_value);\r\n    MOCKABLE_FUNCTION(, int, ");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_set_");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(field_name));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToUpper()));
            
            #line default
            #line hidden
            this.Write("_HANDLE, ");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(c_type));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 53 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(field_name));
            
            #line default
            #line hidden
            this.Write("_value);\r\n");
            
            #line 54 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              } 
            
            #line default
            #line hidden
            this.Write("\r\n");
            
            #line 56 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          } 
            
            #line default
            #line hidden
            
            #line 57 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          else 
            
            #line default
            #line hidden
            
            #line 58 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          if (type.@class == typeClass.restricted) 
            
            #line default
            #line hidden
            
            #line 59 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          { 
            
            #line default
            #line hidden
            
            #line 60 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              string c_type = Program.GetCType(type.source, false).Replace('-', '_').Replace(':', '_'); 
            
            #line default
            #line hidden
            this.Write("\r\n    typedef ");
            
            #line 62 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(c_type));
            
            #line default
            #line hidden
            this.Write(" ");
            
            #line 62 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write(";\r\n\r\n");
            
            #line 64 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              if (c_type != "AMQP_VALUE") 
            
            #line default
            #line hidden
            
            #line 65 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              { 
            
            #line default
            #line hidden
            this.Write("    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_");
            
            #line 66 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write(", ");
            
            #line 66 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write(", value);\r\n");
            
            #line 67 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              } 
            
            #line default
            #line hidden
            
            #line 68 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              else 
            
            #line default
            #line hidden
            
            #line 69 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              { 
            
            #line default
            #line hidden
            this.Write("    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_");
            
            #line 70 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write(", AMQP_VALUE, value);\r\n    #define ");
            
            #line 71 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write("_clone amqpvalue_clone\r\n    #define ");
            
            #line 72 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write("_destroy amqpvalue_destroy\r\n");
            
            #line 73 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              } 
            
            #line default
            #line hidden
            this.Write("\r\n");
            
            #line 75 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              if ((type.Items != null) && (type.Items.Where(item => item is descriptor).Count() > 0)) 
            
            #line default
            #line hidden
            
            #line 76 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              { 
            
            #line default
            #line hidden
            this.Write("    MOCKABLE_FUNCTION(, bool, is_");
            
            #line 77 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_type_by_descriptor, AMQP_VALUE, descriptor);\r\n");
            
            #line 78 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              } 
            
            #line default
            #line hidden
            this.Write("\r\n    #define amqpvalue_get_");
            
            #line 80 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name.ToLower()));
            
            #line default
            #line hidden
            this.Write(" amqpvalue_get_");
            
            #line 80 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type.source.Replace('-', '_').Replace(':', '_')));
            
            #line default
            #line hidden
            this.Write("\r\n\r\n");
            
            #line 82 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              if (type.Items != null) 
            
            #line default
            #line hidden
            
            #line 83 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              { 
            
            #line default
            #line hidden
            
            #line 84 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  foreach (choice choice in type.Items.Where(item => item is choice)) 
            
            #line default
            #line hidden
            
            #line 85 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  { 
            
            #line default
            #line hidden
            
            #line 86 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      if (type.@source == "symbol") 
            
            #line default
            #line hidden
            
            #line 87 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      { 
            
            #line default
            #line hidden
            this.Write("    #define ");
            
            #line 88 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_");
            
            #line 88 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(choice.name.Replace('-', '_').Replace(':', '_')));
            
            #line default
            #line hidden
            this.Write(" \"");
            
            #line 88 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(choice.value));
            
            #line default
            #line hidden
            this.Write("\"\r\n");
            
            #line 89 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      } 
            
            #line default
            #line hidden
            
            #line 90 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      else 
            
            #line default
            #line hidden
            
            #line 91 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      { 
            
            #line default
            #line hidden
            this.Write("    #define ");
            
            #line 92 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(type_name));
            
            #line default
            #line hidden
            this.Write("_");
            
            #line 92 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(choice.name.Replace('-', '_').Replace(':', '_')));
            
            #line default
            #line hidden
            this.Write(" ");
            
            #line 92 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(choice.value.Replace('-', '_').Replace(':', '_')));
            
            #line default
            #line hidden
            this.Write("\r\n");
            
            #line 93 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                      } 
            
            #line default
            #line hidden
            
            #line 94 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
                  } 
            
            #line default
            #line hidden
            
            #line 95 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
              } 
            
            #line default
            #line hidden
            this.Write("\r\n");
            
            #line 97 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
          } 
            
            #line default
            #line hidden
            this.Write("\r\n#ifdef __cplusplus\r\n}\r\n#endif\r\n\r\n#endif /* ");
            
            #line 103 "G:\repos\azure-uamqp-c\uamqp_generator\amqp_definitions_type_h.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture(amqpDefinitionName));
            
            #line default
            #line hidden
            this.Write(" */\r\n");
            return this.GenerationEnvironment.ToString();
        }
    }
    
    #line default
    #line hidden
    #region Base class
    /// <summary>
    /// Base class for this transformation
    /// </summary>
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.VisualStudio.TextTemplating", "15.0.0.0")]
    public class amqp_definitions_type_hBase
    {
        #region Fields
        private global::System.Text.StringBuilder generationEnvironmentField;
        private global::System.CodeDom.Compiler.CompilerErrorCollection errorsField;
        private global::System.Collections.Generic.List<int> indentLengthsField;
        private string currentIndentField = "";
        private bool endsWithNewline;
        private global::System.Collections.Generic.IDictionary<string, object> sessionField;
        #endregion
        #region Properties
        /// <summary>
        /// The string builder that generation-time code is using to assemble generated output
        /// </summary>
        protected System.Text.StringBuilder GenerationEnvironment
        {
            get
            {
                if ((this.generationEnvironmentField == null))
                {
                    this.generationEnvironmentField = new global::System.Text.StringBuilder();
                }
                return this.generationEnvironmentField;
            }
            set
            {
                this.generationEnvironmentField = value;
            }
        }
        /// <summary>
        /// The error collection for the generation process
        /// </summary>
        public System.CodeDom.Compiler.CompilerErrorCollection Errors
        {
            get
            {
                if ((this.errorsField == null))
                {
                    this.errorsField = new global::System.CodeDom.Compiler.CompilerErrorCollection();
                }
                return this.errorsField;
            }
        }
        /// <summary>
        /// A list of the lengths of each indent that was added with PushIndent
        /// </summary>
        private System.Collections.Generic.List<int> indentLengths
        {
            get
            {
                if ((this.indentLengthsField == null))
                {
                    this.indentLengthsField = new global::System.Collections.Generic.List<int>();
                }
                return this.indentLengthsField;
            }
        }
        /// <summary>
        /// Gets the current indent we use when adding lines to the output
        /// </summary>
        public string CurrentIndent
        {
            get
            {
                return this.currentIndentField;
            }
        }
        /// <summary>
        /// Current transformation session
        /// </summary>
        public virtual global::System.Collections.Generic.IDictionary<string, object> Session
        {
            get
            {
                return this.sessionField;
            }
            set
            {
                this.sessionField = value;
            }
        }
        #endregion
        #region Transform-time helpers
        /// <summary>
        /// Write text directly into the generated output
        /// </summary>
        public void Write(string textToAppend)
        {
            if (string.IsNullOrEmpty(textToAppend))
            {
                return;
            }
            // If we're starting off, or if the previous text ended with a newline,
            // we have to append the current indent first.
            if (((this.GenerationEnvironment.Length == 0) 
                        || this.endsWithNewline))
            {
                this.GenerationEnvironment.Append(this.currentIndentField);
                this.endsWithNewline = false;
            }
            // Check if the current text ends with a newline
            if (textToAppend.EndsWith(global::System.Environment.NewLine, global::System.StringComparison.CurrentCulture))
            {
                this.endsWithNewline = true;
            }
            // This is an optimization. If the current indent is "", then we don't have to do any
            // of the more complex stuff further down.
            if ((this.currentIndentField.Length == 0))
            {
                this.GenerationEnvironment.Append(textToAppend);
                return;
            }
            // Everywhere there is a newline in the text, add an indent after it
            textToAppend = textToAppend.Replace(global::System.Environment.NewLine, (global::System.Environment.NewLine + this.currentIndentField));
            // If the text ends with a newline, then we should strip off the indent added at the very end
            // because the appropriate indent will be added when the next time Write() is called
            if (this.endsWithNewline)
            {
                this.GenerationEnvironment.Append(textToAppend, 0, (textToAppend.Length - this.currentIndentField.Length));
            }
            else
            {
                this.GenerationEnvironment.Append(textToAppend);
            }
        }
        /// <summary>
        /// Write text directly into the generated output
        /// </summary>
        public void WriteLine(string textToAppend)
        {
            this.Write(textToAppend);
            this.GenerationEnvironment.AppendLine();
            this.endsWithNewline = true;
        }
        /// <summary>
        /// Write formatted text directly into the generated output
        /// </summary>
        public void Write(string format, params object[] args)
        {
            this.Write(string.Format(global::System.Globalization.CultureInfo.CurrentCulture, format, args));
        }
        /// <summary>
        /// Write formatted text directly into the generated output
        /// </summary>
        public void WriteLine(string format, params object[] args)
        {
            this.WriteLine(string.Format(global::System.Globalization.CultureInfo.CurrentCulture, format, args));
        }
        /// <summary>
        /// Raise an error
        /// </summary>
        public void Error(string message)
        {
            System.CodeDom.Compiler.CompilerError error = new global::System.CodeDom.Compiler.CompilerError();
            error.ErrorText = message;
            this.Errors.Add(error);
        }
        /// <summary>
        /// Raise a warning
        /// </summary>
        public void Warning(string message)
        {
            System.CodeDom.Compiler.CompilerError error = new global::System.CodeDom.Compiler.CompilerError();
            error.ErrorText = message;
            error.IsWarning = true;
            this.Errors.Add(error);
        }
        /// <summary>
        /// Increase the indent
        /// </summary>
        public void PushIndent(string indent)
        {
            if ((indent == null))
            {
                throw new global::System.ArgumentNullException("indent");
            }
            this.currentIndentField = (this.currentIndentField + indent);
            this.indentLengths.Add(indent.Length);
        }
        /// <summary>
        /// Remove the last indent that was added with PushIndent
        /// </summary>
        public string PopIndent()
        {
            string returnValue = "";
            if ((this.indentLengths.Count > 0))
            {
                int indentLength = this.indentLengths[(this.indentLengths.Count - 1)];
                this.indentLengths.RemoveAt((this.indentLengths.Count - 1));
                if ((indentLength > 0))
                {
                    returnValue = this.currentIndentField.Substring((this.currentIndentField.Length - indentLength));
                    this.currentIndentField = this.currentIndentField.Remove((this.currentIndentField.Length - indentLength));
                }
            }
            return returnValue;
        }
        /// <summary>
        /// Remove any indentation
        /// </summary>
        public void ClearIndent()
        {
            this.indentLengths.Clear();
            this.currentIndentField = "";
        }
        #endregion
        #region ToString Helpers
        /// <summary>
        /// Utility class to produce culture-oriented representation of an object as a string.
        /// </summary>
        public class ToStringInstanceHelper
        {
            private System.IFormatProvider formatProviderField  = global::System.Globalization.CultureInfo.InvariantCulture;
            /// <summary>
            /// Gets or sets format provider to be used by ToStringWithCulture method.
            /// </summary>
            public System.IFormatProvider FormatProvider
            {
                get
                {
                    return this.formatProviderField ;
                }
                set
                {
                    if ((value != null))
                    {
                        this.formatProviderField  = value;
                    }
                }
            }
            /// <summary>
            /// This is called from the compile/run appdomain to convert objects within an expression block to a string
            /// </summary>
            public string ToStringWithCulture(object objectToConvert)
            {
                if ((objectToConvert == null))
                {
                    throw new global::System.ArgumentNullException("objectToConvert");
                }
                System.Type t = objectToConvert.GetType();
                System.Reflection.MethodInfo method = t.GetMethod("ToString", new System.Type[] {
                            typeof(System.IFormatProvider)});
                if ((method == null))
                {
                    return objectToConvert.ToString();
                }
                else
                {
                    return ((string)(method.Invoke(objectToConvert, new object[] {
                                this.formatProviderField })));
                }
            }
        }
        private ToStringInstanceHelper toStringHelperField = new ToStringInstanceHelper();
        /// <summary>
        /// Helper to produce culture-oriented representation of an object as a string
        /// </summary>
        public ToStringInstanceHelper ToStringHelper
        {
            get
            {
                return this.toStringHelperField;
            }
        }
        #endregion
    }
    #endregion
}


